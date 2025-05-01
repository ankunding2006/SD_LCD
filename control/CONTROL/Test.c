#include "Test.h"
#include "control.h"
#include <stdio.h>
#include <math.h>

/**
 * @brief 转向测试函数 - 单方向循环旋转测试
 * @note 控制小车顺时针转60度，等待3秒，再次顺时针转60度，等待3秒，如此循环
 * @return None
 */
void SteeringTest_CyclicRotation(void)
{
    static uint8_t test_state = 0;  // 测试状态：0-初始化 1-顺时针旋转 2-等待3秒
    static uint32_t wait_time = 0;  // 等待时间计数器
    
    // 根据当前状态执行不同操作
    switch(test_state)
    {
        case 0:  // 初始化状态
            printf("开始转向测试：顺时针60度 -> 等待3秒 -> 循环\r\n");
            test_state = 1;  // 进入顺时针旋转状态
            led1_on();       // 点亮LED1作为顺时针旋转指示
            led2_off();
            led3_off();
            break;
            
        case 1:  // 顺时针旋转60度
            if(localSteeringControl_Handler(-60.0f))  // 如果转向完成
            {
                printf("顺时针旋转60度完成，等待3秒...\r\n");
                test_state = 2;   // 进入等待状态
                wait_time = 0;    // 清零等待时间计数器
                led1_off();       // 关闭LED1
                led3_on();        // 点亮LED3作为等待指示
            }
            break;
            
        case 2:  // 等待3秒
            wait_time++;
            if(wait_time >= 600)  // 5ms中断，600次大约3秒
            {
                test_state = 1;   // 返回顺时针旋转状态，形成循环
                printf("等待结束，再次开始顺时针旋转60度...\r\n");
                led3_off();       // 关闭LED3
                led1_on();        // 点亮LED1作为顺时针旋转指示
            }
            break;
            
        default:
            test_state = 0;  // 异常情况，重置状态
            break;
    }
}

/**
 * @brief 测试处理函数 - 根据配置选择执行不同的测试
 * @note 在TIM6中断中被调用
 * @return None
 */
void Test_Handler(void)
{
    #if TEST_STEERING_ROTATION==1 // 测试转向旋转 
        SteeringTest_CyclicRotation(); // 调用转向测试函数
    #elif TEST_TRACKING==1 // 测试循迹
        lineTracking_Handler(); // 调用循迹函数
    #elif TEST_MOVE_FORWARD==1 // 测试前进
        moveForward_Handler(); // 调用前进函数
    #elif TEST_TURNTO_ABSLUTE_ANGLE==1 // 测试转向到绝对角度
        turnToAbsoluteAngle_TEST_Handler(); // 调用转向到绝对角度函数
    #elif MOVE_FORWARD_WITH_ANGLE_HANDLE==1 // 测试指定角度前进
        moveForwardWithAngle_Handler(115.0f); // 调用指定角度前进函数
    #endif
}

/**
 * @brief turnToAbsoluteAngle测试函数
 * @note 通过绝对角度控制小车先顺时针转90度，再逆时针转90度
 * @return None
 */
void turnToAbsoluteAngle_TEST_Handler(void)
{
    static u8 test_state = 0; // 测试状态：0-顺时针转90度 1-等待 2-逆时针转90度 3-等待
    static uint32_t wait_time = 0; // 等待时间计数器
    
    // 根据当前状态执行不同操作
    switch(test_state)
    {
        case 0:  // 顺时针转90度
            if(turnToAbsoluteAngle(90.0f)) { // 如果转向完成
                printf("顺时针转90度完成，等待3秒...\r\n");
                test_state = 1;   // 进入等待状态
                wait_time = 0;    // 清零等待时间计数器
            }
            break;
            
        case 1:  // 等待3秒
            wait_time++;
            if(wait_time >= 600) { // 5ms中断，600次大约3秒
                test_state = 2;   // 返回逆时针转90度状态
                printf("等待结束，再次开始逆时针转90度...\r\n");
            }
            break;
            
        case 2:  // 逆时针转90度
            if(turnToAbsoluteAngle(30.0f)) { // 如果转向完成
                printf("逆时针转30度完成，等待3秒...\r\n");
                test_state = 3;   // 返回顺时针转一定角度状态，形成循环
                wait_time = 0;    // 清零等待时间计数器
            }
            break;
        
        case 3:  // 等待3秒
            wait_time++;
            if(wait_time >= 600) { // 5ms中断，600次大约3秒
                test_state = 0;   // 返回顺时针转一定角度状态，形成循环
                printf("等待结束，再次开始顺时针转30度...\r\n");
            }
            break;
            
        default:
            test_state = 0;  // 异常情况，重置状态
            break;
    }
}


/**
 * @brief 打印角度处理函数 - 打印当前角度
 * @param None
 * @return None
 */
void print_angle_Handle(void)
{
    static uint32_t debug_print_counter = 0; // 调试信息发送计数器
    bool can_print_debug = false;
    
    if (++debug_print_counter >= ANGLE_PRINT_COUNT) {
        debug_print_counter = 0;
        can_print_debug = true;
    }
    
    if(can_print_debug) {
        printf("当前角度: %.2f\r\n", getHeadingAngle()); // 打印当前角度
    }
}
