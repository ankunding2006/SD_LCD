/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdlib.h>
#include "my_menu.h"
#include "Emm_V5.h"
#include "test.h"
#include "gray_detection.h"
#include "control.h"
#include "task.h"
#include "car_config.h"

void main_test(void)
{
    line_following_task();
}

void before_main_test(void)
{
    while (1)
    {
        /* code */
    }
}

void test_in_interrupt(void)
{
#if TEST_IN_INTERRUPT == 1
#if TEST_TASK == 1
    // 定义测试状态
    typedef enum
    {
        TEST_STATE_RUNNING, // 任务运行中
        TEST_STATE_DELAY,   // 任务完成，延迟等待
    } TestState_t;

    static TestState_t test_state = TEST_STATE_RUNNING;
#if TASK_RUN_ONLY_ONCE == 0
    static uint32_t delay_end_time = 0;
#endif

    switch (test_state)
    {
    case TEST_STATE_RUNNING:
    {
        uint8_t task_finished = 0;
        // 执行选定的测试任务
#if TEST_CAR_TO_CROSSING == 1
        task_finished = Car_To_Crossing(TEST_CROSSING_NUM);
#elif TEST_CAR_TO_ROOM == 1
        room_target = TEST_ROOM_NUM; // 设置全局目标
        task_finished = Car_To_Room();
#elif TEST_OPEN_LOOP_STEERING == 1
        task_finished = open_loop_steering_control(TEST_OPEN_LOOP_STEERING_TIME, TEST_OPEN_LOOP_STEERING_SPEED, TEST_OPEN_LOOP_STEERING_SPEED_BASE);
#endif

        // 如果任务完成，则进入延迟状态
        if (task_finished)
        {
            test_state = TEST_STATE_DELAY;
#if TASK_RUN_ONLY_ONCE == 0
            delay_end_time = HAL_GetTick() + (TEST_REPEAT_DELAY_S * 1000);
#endif
        }
        break;
    }
    case TEST_STATE_DELAY:
        // 等待延迟时间结束
#if TASK_RUN_ONLY_ONCE == 0
        if (HAL_GetTick() >= delay_end_time)
        {
            test_state = TEST_STATE_RUNNING; // 延迟结束，准备重新开始任务
        }
#endif
        break;
    }
#elif TEST_DRIVER == 1
#if TEST_DRIVER_EMM_V5 == 1
    // 测试Emm_V5驱动,正转1s,反转1s,如此循环,不要用阻塞的方式实现
    typedef enum
    {
        MOTOR_TEST_FORWARD, // 正转状态
        MOTOR_TEST_REVERSE, // 反转状态
        MOTOR_TEST_WAIT     // 延时等待状态
    } MotorTestState_t;

    static MotorTestState_t motor_test_state = MOTOR_TEST_FORWARD;
    static MotorTestState_t next_motor_state = MOTOR_TEST_REVERSE; // 记录下一个要进入的运行状态
    static uint32_t wait_end_time = 0;

    switch (motor_test_state)
    {
    case MOTOR_TEST_FORWARD:
        // 发送正转指令
        if (Emm_V5_Vel_Control(TEST_MOTOR_ADDR, 1, TEST_MOTOR_SPEED, DEFAULT_ACCELERATION, 0) == HAL_OK)
        {
            motor_test_state = MOTOR_TEST_WAIT;
            next_motor_state = MOTOR_TEST_REVERSE; // 下一步反转
            wait_end_time = HAL_GetTick() + 1000;
        }
        break;

    case MOTOR_TEST_REVERSE:
        // 发送反转指令
        if (Emm_V5_Vel_Control(TEST_MOTOR_ADDR, 0, TEST_MOTOR_SPEED, DEFAULT_ACCELERATION, 0) == HAL_OK)
        {
            motor_test_state = MOTOR_TEST_WAIT;
            next_motor_state = MOTOR_TEST_FORWARD; // 下一步正转
            wait_end_time = HAL_GetTick() + 1000;
        }
        break;

    case MOTOR_TEST_WAIT:
        // 等待1秒结束
        if (HAL_GetTick() >= wait_end_time)
        {
            motor_test_state = next_motor_state; // 进入下一个运行状态
        }
        break;
    }
#elif TEST_CONTROL_SET_MOTOR_SPEED == 1
    // 测试set_motor_speed函数, 控制两个电机正转1s, 反转1s, 如此循环
    typedef enum
    {
        SMS_TEST_FORWARD, // 正转状态
        SMS_TEST_REVERSE, // 反转状态
        SMS_TEST_WAIT     // 延时等待状态
    } SMSTestState_t;

    static SMSTestState_t sms_test_state = SMS_TEST_FORWARD;
    static SMSTestState_t next_sms_state = SMS_TEST_REVERSE;
    static uint32_t wait_end_time = 0;

    switch (sms_test_state)
    {
    case SMS_TEST_FORWARD:
        // 通过set_motor_speed请求两个电机都正转
        if (set_motor_speed(TEST_MOTOR_SPEED, TEST_MOTOR_SPEED, DEFAULT_ACCELERATION) == 0)
        {
            // 任务成功添加到队列，进入等待状态
            sms_test_state = SMS_TEST_WAIT;
            next_sms_state = SMS_TEST_REVERSE; // 下一步反转
            wait_end_time = HAL_GetTick() + 1000;
        }
        break;

    case SMS_TEST_REVERSE:
        // 通过set_motor_speed请求两个电机都反转
        if (set_motor_speed(-TEST_MOTOR_SPEED, -TEST_MOTOR_SPEED, DEFAULT_ACCELERATION) == 0)
        {
            // 任务成功添加到队列，进入等待状态
            sms_test_state = SMS_TEST_WAIT;
            next_sms_state = SMS_TEST_FORWARD; // 下一步正转
            wait_end_time = HAL_GetTick() + 1000;
        }
        break;

    case SMS_TEST_WAIT:
        // 等待1秒结束
        if (HAL_GetTick() >= wait_end_time)
        {
            sms_test_state = next_sms_state; // 进入下一个运行状态
        }
        break;
    }
#endif
#endif
#endif
}
