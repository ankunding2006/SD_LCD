#include "Task.h"
#include "control.h"
#include "LED.h"
#include <stdio.h>

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
        INIT_WAIT,         // 等待陀螺仪稳定
        INIT,              // 初始化状态
        MOVE_A_TO_B,       // A到B直线行驶
        TRACK_B_TO_C,      // B到C沿黑线循迹
        TURN_AT_C,         // 在C点旋转
        MOVE_C_TO_D,       // C到D直线行驶
        TRACK_D_TO_A,      // D到A沿黑线循迹
        COMPLETED          // 任务完成
    } TestState;
    
    // 使用静态变量保存状态和起始角度
    static TestState currentState = INIT_WAIT;
    static float startAngle = 0.0f;
    static float targetAngle = 0.0f;
    static uint32_t init_start_time = 0;   // 初始化开始时间
    static float prev_angle = 0.0f;        // 上一次读取的角度
    static uint8_t retry_count = 0;        // 重试次数计数
    static uint16_t debug_print_counter = 0; // 调试信息发送计数器
    
    // 调试信息发送频率控制
    bool can_print_debug = false;
    if (++debug_print_counter >= DEBUG_PRINT_COUNT) {
        debug_print_counter = 0;
        can_print_debug = true;
    }
    
    // 状态机实现
    switch(currentState)
    {
        case INIT_WAIT:
            // 等待陀螺仪稳定
            if(init_start_time == 0) {
                // 记录初始化开始时间
                init_start_time = HAL_GetTick();
                prev_angle = getHeadingAngle();
                printf("等待陀螺仪稳定...\r\n");
                return 0;
            }
            
            // 检查是否已经等待足够时间
            if(HAL_GetTick() - init_start_time >= GYRO_CHECK_INTERVAL) {
                // 读取当前角度
                float current_angle = getHeadingAngle();
                
                // 检查数据是否有效
                if(current_angle == prev_angle) {
                    // 读取的值完全一样，可能是传感器未正确更新
                    printf("陀螺仪数据未更新，重试中 (%d/%d)...\r\n", 
                           retry_count + 1, GYRO_INIT_RETRIES);
                    
                    retry_count++;
                    if(retry_count >= GYRO_INIT_RETRIES) {
                        // 达到最大重试次数，尝试重新初始化传感器
                        printf("陀螺仪初始化失败，请检查连接或重启设备\r\n");
                        
                        // 这里可以添加传感器重新初始化的代码
                        // JY901_init();
                        
                        // 重置重试计数和初始化时间，重新开始初始化
                        retry_count = 0;
                        init_start_time = 0;
                    } else {
                        // 重置初始化时间，继续等待
                        init_start_time = HAL_GetTick();
                    }
                    return 0;
                }
                
                // 检查数据变化是否在合理范围内
                float angle_diff = fabs(current_angle - prev_angle);
                if(angle_diff > GYRO_CHECK_THRESHOLD) {
                    printf("陀螺仪数据波动较大 (%.2f°)，继续等待稳定...\r\n", angle_diff);
                    
                    // 更新上一次角度值并继续等待
                    prev_angle = current_angle;
                    init_start_time = HAL_GetTick();
                    return 0;
                }
                
                // 陀螺仪数据稳定，进入初始化状态
                printf("陀螺仪数据稳定，开始任务初始化\r\n");
                currentState = INIT;
                init_start_time = 0;
                retry_count = 0;
            }
            return 0;
            
        case INIT:
            // 初始化：记录起始角度，设置初始速度
            startAngle = getHeadingAngle();
            printf("测试任务2开始: 初始角度 = %.2f\r\n", startAngle);
            
            // 计算目标反向角度（起始角度+180度，确保在±180度范围内）
            targetAngle = startAngle + 180.0f;
            // 规范化角度到±180度范围
            while(targetAngle > 180.0f) {
                targetAngle -= 360.0f;
            }
            while(targetAngle < -180.0f) {
                targetAngle += 360.0f;
            }
            
            printf("目标反向角度: %.2f\r\n", targetAngle);
            Set_Target_Velocity(10); // 设置适当的速度
            led1_on(); // 点亮LED1表示任务开始
            
            // 重置调试打印计数器
            debug_print_counter = 0;
            
            // 切换到下一状态
            currentState = MOVE_A_TO_B;
            return 0;
            
        case MOVE_A_TO_B:
            // A点到B点的直线行驶
            if(can_print_debug) {
                printf("A→B直线行驶中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(moveForward_Handler()) {
                // moveForward_Handler返回1表示检测到黑线，即到达B点
                printf("到达B点，当前角度: %.2f\r\n", getHeadingAngle());
                led1_off();
                led2_on(); // 切换LED指示当前状态
                currentState = TRACK_B_TO_C;
            }
            return 0;
            
        case TRACK_B_TO_C:
            // B点到C点沿黑线循迹
            if(can_print_debug) {
                printf("B→C循迹中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(lineTracking_Handler()) {
                // lineTracking_Handler返回1表示完成循迹（所有传感器都检测到白色）
                printf("到达C点，当前角度: %.2f\r\n", getHeadingAngle());
                led2_off();
                led3_on(); // 切换LED指示当前状态
                currentState = TURN_AT_C;
            }
            return 0;
            
        case TURN_AT_C:
            // 在C点旋转至目标角度
            if(can_print_debug) {
                printf("C点旋转：当前角度 = %.2f, 目标角度 = %.2f, 差值 = %.2f\r\n", 
                      getHeadingAngle(), targetAngle, targetAngle - getHeadingAngle());
            }
                   
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
            if(can_print_debug) {
                printf("C→D直线行驶中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(moveForward_Handler()) {
                // moveForward_Handler返回1表示检测到黑线，即到达D点
                printf("到达D点，当前角度: %.2f\r\n", getHeadingAngle());
                led1_off();
                led2_on(); // 切换LED指示当前状态
                currentState = TRACK_D_TO_A;
            }
            return 0;
            
        case TRACK_D_TO_A:
            // D点到A点沿黑线循迹
            if(can_print_debug) {
                printf("D→A循迹中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(lineTracking_Handler()) {
                // lineTracking_Handler返回1表示完成循迹
                printf("到达A点, 任务完成，当前角度: %.2f\r\n", getHeadingAngle());
                led2_off();
                all_leds_on(); // 所有LED亮起表示任务完成
                Set_Pwm(0, 0); // 停止电机
                currentState = COMPLETED;
            }
            return 0;
            
        case COMPLETED:
            // 任务完成状态
            // 重置状态，为下次任务做准备
            currentState = INIT_WAIT;
            all_leds_off();
            return 1; // 返回1表示任务完成
            
        default:
            // 异常情况，重置状态
            currentState = INIT_WAIT;
            return 0;
    }
}
