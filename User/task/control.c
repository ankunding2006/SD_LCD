/**
 * @file control.c
 * @brief 控制任务
 * @note:介绍一下硬件系统,我的车轮使用了两个伺服电机作为后轮驱动,电机的地址分别为0x01和0x02
 * @version 0.1
 * @date 2025-07-15
 * @copyright Copyright (c) 2025
 */
#include "control.h"
#include "main.h"
#include <stdlib.h>
#include "Emm_V5.h"
#include "gray_detection.h"

uint8_t usart2_rx_buffer = 9;    // 单字节接收缓冲区
volatile uint8_t crossroads = 0; // 到十字路口的次数

#define CAR_BASE_SPEED 200 // 小车基础速度
#define CAR_MAX_SPEED 400  // 小车最大速度
/**
 * @brief 设置两个电机的速度
 * @param left_speed 左电机速度(单位:转/分钟)
 * @param right_speed 右电机速度(单位:转/分钟)
 * @note 速度都为正数的时候,小车向前走,速度都为负数的时候,小车向后走,速度一个为正一个为负的时候,小车向左转,速度一个为负一个为正的时候,小车向右转
 * @param acc 加速度(0-255)
 */
void set_motor_speed(int16_t left_speed, int16_t right_speed, uint8_t acc)
{
    if (left_speed > 0)
        Emm_V5_Vel_Control(0x01, 1, left_speed, acc, 0);
    else
        Emm_V5_Vel_Control(0x01, 0, -left_speed, acc, 0);
    if (right_speed > 0)
        Emm_V5_Vel_Control(0x02, 1, right_speed, acc, 0);
    else
        Emm_V5_Vel_Control(0x02, 0, -right_speed, acc, 0);
}

/**
 * @brief 这个函数用来控制小车进行循迹 有直角弯就crossroads++
 * @return 如果完成了循迹(即出现至少3个传感器都返回黑色)就返回1,否则返回0
 */
uint8_t line_following_task(void)
{
    // 1. 更新传感器数据
    grey_sensor_Read();

    // 2. 计算转向值
    int turn_value = Calculate_Turn_Value();

    // 3. 根据转向值调整电机速度
    if (turn_value == INT16_MAX)
    {
        // 检测到6个或更多传感器，判定为到达了直角弯区域，让直角弯计数加一，根据写死的逻辑左右转向，并且记录
        static uint32_t pre_time = 0;      // (ms)
        uint64_t now_time = HAL_GetTick(); // 两个时间进行处理
        if (now_time - pre_time > 500)     // 间隔较长视为进一次直角弯 0-1也是间隔较长 无bug
        {
            crossroads++;
        }
        pre_time = now_time;

        set_motor_speed(0, 0, 252); // 左转 or 右转
        return 1;                   // 完成循迹 ？
    }
    else if (turn_value == INT16_MIN)
    {
        // 没有检测到线，可能脱轨了
        // 策略：原地停车
        set_motor_speed(0, 0, 252);
    }
    else
    {
        // 根据转向值调整左右轮速度
        // turn_value > 0 表示线在左边，需要左转，左轮慢，右轮快
        // turn_value < 0 表示线在右边，需要右转，左轮快，右轮慢
        int16_t left_speed = CAR_BASE_SPEED - turn_value;
        int16_t right_speed = CAR_BASE_SPEED + turn_value;

        // 对速度进行限幅
        if (left_speed > CAR_MAX_SPEED)
            left_speed = CAR_MAX_SPEED;
        if (left_speed < 0)
            left_speed = 0;
        if (right_speed > CAR_MAX_SPEED)
            right_speed = CAR_MAX_SPEED;
        if (right_speed < 0)
            right_speed = 0;

        set_motor_speed(left_speed, right_speed, 252); // 使用一个默认加速度
    }

    return 0; // 循迹未完成
}

/**
 * @brief Open loop steering control function
 *
 * @param angle 小车旋转的时间(ms),逆时针旋转为正,顺时针旋转为负. 0表示查询状态
 * @param Rotate_Speed 转向转速值(单位:转/分钟)
 * @param Rotate_Speed_Base 基础转速值(单位:转/分钟)
 * @return uint8_t 1:完成 0:小车正在转向中
 */
uint8_t open_loop_steering_control(int16_t angle, int16_t Rotate_Speed, int16_t Rotate_Speed_Base)
{
    static uint32_t turn_end_time = 0;

    // A non-zero angle indicates a new turn command.
    if (angle != 0)
    {
        int16_t left_speed, right_speed;

        if (angle > 0)
        { // Counter-clockwise turn (left)
            left_speed = Rotate_Speed_Base - Rotate_Speed;
            right_speed = Rotate_Speed_Base + Rotate_Speed;
        }
        else
        { // Clockwise turn (right)
            left_speed = Rotate_Speed_Base + Rotate_Speed;
            right_speed = Rotate_Speed_Base - Rotate_Speed;
        }

        set_motor_speed(left_speed, right_speed, 252); // Use a default acceleration
        turn_end_time = HAL_GetTick() + abs(angle);
        return 0; // Turn initiated
    }
    else
    {
        // angle is 0, check status of the current turn.
        if (turn_end_time == 0)
        {
            return 1; // No turn in progress, so it's "completed"
        }

        if (HAL_GetTick() >= turn_end_time)
        {
            set_motor_speed(0, 0, 252); // Stop motors
            turn_end_time = 0;          // Reset state
            return 1;                   // Turn completed
        }

        return 0; // Still turning
    }
}

/**
 * @brief 串口初始化
 * @param 无：懒得改，写死了
 *
 */
void visual_reception_init()
{
    HAL_UART_Receive_IT(&huart2, &usart2_rx_buffer, 1);
}

/**
 * @brief 接收视觉传来的数据，接收到正确数据后会执行“一次”处理，然后重新启动中断接收
 * @param 无：懒得改，写死了
 */
void visual_process_command(void) // 处理相应消息
{
    switch (usart2_rx_buffer)
    {
    case 1:
        printf("num: %d\n", usart2_rx_buffer);
        usart2_rx_buffer = 9; // 清除接收缓冲区

        break;
    case 2:
        printf("num: %d\n", usart2_rx_buffer);
        usart2_rx_buffer = 9; // 清除接收缓冲区

        break;
    case 3:
        printf("num: %d\n", usart2_rx_buffer);
        usart2_rx_buffer = 9; // 清除接收缓冲区

        break;
    case 4:
        printf("num: %d\n", usart2_rx_buffer);
        usart2_rx_buffer = 9; // 清除接收缓冲区

        break;
    case 5:
        printf("num: %d\n", usart2_rx_buffer);
        usart2_rx_buffer = 9; // 清除接收缓冲区

        break;
    case 6:
        printf("num: %d\n", usart2_rx_buffer);
        usart2_rx_buffer = 9; // 清除接收缓冲区

        break;
    case 7:
        printf("num: %d\n", usart2_rx_buffer);
        usart2_rx_buffer = 9; // 清除接收缓冲区

        break;
    case 8:
        printf("num: %d\n", usart2_rx_buffer);
        usart2_rx_buffer = 9; // 清除接收缓冲区

        break;
    default:
        if (usart2_rx_buffer != 9)
        {
            printf("bug \n");
            usart2_rx_buffer = 9; // 清除接收缓冲区
        }
        HAL_UART_Receive_IT(&huart2, &usart2_rx_buffer, 1);
        break;
    }
}