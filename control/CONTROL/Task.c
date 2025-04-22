#include "Task.h"
#include "control.h"
#include "LED.h"
#include <stdio.h>
#include "main.h"

/**
 * @brief 测试任务2的处理函数
 * @note 小车从A点出发，按以下路径行驶：
 * 1. 直线行驶至B点
 * 2. 沿半圆弧黑线从B行驶至C点
 * 3. 直线行驶至D点
 * 4. 沿另一半圆弧黑线从D返回A点停车
 * @return u8 1:完成全程 0:小车正在执行任务中
 */
u8 Task2_Handler(void)
{
    // 定义状态机状态
    typedef enum {
        INIT,              // 初始化状态
        MOVE_A_TO_B,       // A到B直线行驶
        TRACK_B_TO_C,      // B到C沿黑线循迹
        TURN_AT_C,         // 在C点旋转
        MOVE_C_TO_D,       // C到D直线行驶
        TRACK_D_TO_A,      // D到A沿黑线循迹
        COMPLETED          // 任务完成
    } TestState;
    
    // 使用静态变量保存状态和起始角度
    static TestState currentState = INIT;
    static float startAngle = 0.0f;
    static float targetAngle = 0.0f;
    
    // 读取编码器数据和处理运动计算（这部分在各状态处理中使用）
    Encoder_Left = Read_Encoder(3);
    Encoder_Right = Read_Encoder(5);
    Get_Velocity_Form_Encoder(Encoder_Left, Encoder_Right);
    
    // 状态机实现
    switch(currentState)
    {
        case INIT:
            // 初始化：记录起始角度，设置初始速度
            startAngle = getHeadingAngle();
            printf("测试任务2开始: 初始角度 = %.2f\r\n", startAngle);
            
            // 计算目标反向角度（起始角度+180度，处理边界情况）
            targetAngle = startAngle + 180.0f;
            if(targetAngle > 180.0f) {
                targetAngle -= 360.0f;
            }
            
            printf("目标反向角度: %.2f\r\n", targetAngle);
            Set_Target_Velocity(10); // 设置适当的速度
            led1_on(); // 点亮LED1表示任务开始
            
            // 切换到下一状态
            currentState = MOVE_A_TO_B;
            return 0;
            
        case MOVE_A_TO_B:
            // A点到B点的直线行驶
            printf("状态: A->B 直线行驶\r\n");
            if(moveForward_Handler()) {
                // moveForward_Handler返回1表示检测到黑线，即到达B点
                printf("到达B点\r\n");
                led1_off();
                led2_on(); // 切换LED指示当前状态
                currentState = TRACK_B_TO_C;
            }
            return 0;
            
        case TRACK_B_TO_C:
            // B点到C点沿黑线循迹
            printf("状态: B->C 沿黑线循迹\r\n");
            if(lineTracking_Handler()) {
                // lineTracking_Handler返回1表示完成循迹（所有传感器都检测到白色）
                printf("到达C点\r\n");
                led2_off();
                led3_on(); // 切换LED指示当前状态
                currentState = TURN_AT_C;
            }
            return 0;
            
        case TURN_AT_C:
            // 在C点旋转至目标角度（起始角度的反向）
            printf("状态: C点旋转\r\n");
            if(turnToAbsoluteAngle(targetAngle)) {
                // turnToAbsoluteAngle返回1表示完成转向
                printf("C点转向完成，目标角度: %.2f，当前角度: %.2f\r\n", 
                       targetAngle, getHeadingAngle());
                led3_off();
                led1_on(); // 切换LED指示当前状态
                currentState = MOVE_C_TO_D;
            }
            return 0;
            
        case MOVE_C_TO_D:
            // C点到D点的直线行驶
            printf("状态: C->D 直线行驶\r\n");
            if(moveForward_Handler()) {
                // moveForward_Handler返回1表示检测到黑线，即到达D点
                printf("到达D点\r\n");
                led1_off();
                led2_on(); // 切换LED指示当前状态
                currentState = TRACK_D_TO_A;
            }
            return 0;
            
        case TRACK_D_TO_A:
            // D点到A点沿黑线循迹
            printf("状态: D->A 沿黑线循迹\r\n");
            if(lineTracking_Handler()) {
                // lineTracking_Handler返回1表示完成循迹
                printf("到达A点, 任务完成\r\n");
                led2_off();
                all_leds_on(); // 所有LED亮起表示任务完成
                Set_Pwm(0, 0); // 停止电机
                currentState = COMPLETED;
            }
            return 0;
            
        case COMPLETED:
            // 任务完成状态
            // 重置状态，为下次任务做准备
            currentState = INIT;
            all_leds_off();
            return 1; // 返回1表示任务完成
            
        default:
            // 异常情况，重置状态
            currentState = INIT;
            return 0;
    }
}
