#include "Task.h"


/**
 * @brief 任务1处理函数
 * @note 小车从A点出发，沿直线行驶至B点后停车，全程用时不超过15秒
 * 触发条件：到达B点时需有声（如蜂鸣器）光（如LED）提示
 * @return u8 1:完成任务 0:小车正在执行任务中
 */
u8 Task1_Handler(void)
{
    // 定义状态机状态
    typedef enum {
        INIT_WAIT,         // 等待陀螺仪稳定
        INIT,              // 初始化状态
        MOVE_A_TO_B,       // A到B直线行驶
        COMPLETED          // 任务完成
    } TaskState;
    
    // 使用静态变量保存状态
    static TaskState currentState = INIT_WAIT;
    static uint32_t init_start_time = 0;   // 初始化开始时间
    static float prev_angle = 0.0f;        // 上一次读取的角度
    static uint8_t retry_count = 0;        // 重试次数计数
    static uint16_t debug_print_counter = 0; // 调试信息发送计数器
    static uint32_t task_start_time = 0;    // 任务开始时间
    
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
            // 初始化：设置初始速度和状态
            printf("任务1开始: 从A点沿直线行驶至B点并停车\r\n");
            printf("初始角度 = %.2f\r\n", getHeadingAngle());
            
            Set_Target_Velocity(16); // 设置适当的速度
            led1_on(); // 点亮LED1表示任务开始
            beep_on(); // 蜂鸣器提示任务开始
            
            // 重置调试打印计数器
            debug_print_counter = 0;
            
            // 记录任务开始时间
            task_start_time = HAL_GetTick();
            
            // 切换到下一状态
            currentState = MOVE_A_TO_B;
            return 0;
            
        case MOVE_A_TO_B:
            // A点到B点的直线行驶
            if(can_print_debug) {
                printf("A→B直线行驶中: 当前角度 = %.2f, 已用时间: %dms\r\n", 
                       getHeadingAngle(), (int)(HAL_GetTick() - task_start_time));
            }
            
            // 检查任务是否超时
            if(HAL_GetTick() - task_start_time > 15000) { // 15秒超时
                printf("任务1超时！已停止\r\n");
                Set_Pwm(0, 0); // 停止电机
                all_leds_on(); // 所有LED亮起表示出错
                currentState = COMPLETED;
                return 1;
            }
            
            if(moveForward_Handler()) {
                // moveForward_Handler返回1表示检测到黑线，即到达B点
                printf("到达B点，任务完成！用时: %dms\r\n", (int)(HAL_GetTick() - task_start_time));
                // 声光提示已到达B点
                    all_leds_on();
                    beep_on(); // 蜂鸣器提示
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

    if(resetTask_flag==1)
    {
        // 一定时间后重置任务状态和变量
        Set_Pwm(0,0);
        if(HAL_GetTick()-resetMode_start_time>RESET_WAIT_TIME)
        {
            currentState = INIT;
            init_start_time = 0;
            prev_angle = 0.0f;
            retry_count = 0; 
            resetTask_flag=0;
            led1_off(); led2_off(); led3_off(); // 关闭所有LED
            printf("任务重置完成\r\n");
        }
        else
        {
            return 0;
        }
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
            #if IS_SET_INIT_ANGLE_MANUALY==1
            initialAngle = MANUAL_INIT_ANGLE; // 手动设置初始角度
            #else
            if(manual_mode == 0) {
                initialAngle = getHeadingAngle(); // 获取当前角度作为初始角度
            }
            #endif
            manual_mode = 0; // 重置手动模式标志
            printf("测试任务2开始: 初始角度 = %.2f\r\n", initialAngle);
            
            // 计算目标反向角度（起始角度+180度，确保在±180度范围内）
            targetAngle = initialAngle + 180.0f + 8.0f;
            // 规范化角度到±180度范围
            while(targetAngle > 180.0f) {
                targetAngle -= 360.0f;
            }
            while(targetAngle < -180.0f) {
                targetAngle += 360.0f;
            }
            
            printf("目标反向角度: %.2f\r\n", targetAngle);
            Set_Target_Velocity(14); // 设置适当的速度
            led1_on(); // 点亮LED1表示任务开始
            beep_on(); // 蜂鸣器提示任务开始
            
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
                // 返回1表示检测到黑线，即到达B点
                printf("到达B点，当前角度: %.2f\r\n", getHeadingAngle());
                led1_off();
                Clear_Encoder(); // 清除编码器计数器
                led2_on(); // 切换LED指示当前状态
                beep_on(); // 蜂鸣器提示到达B点
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
                beep_on(); // 蜂鸣器提示到达C点
                currentState = TURN_AT_C;
            }
            return 0;
            
        case TURN_AT_C:
            // 在C点旋转至目标角度
            if(can_print_debug) {
                printf("C点旋转：当前角度 = %.2f, 目标角度 = %.2f, 差值 = %.2f\r\n", 
                      getHeadingAngle(), targetAngle, targetAngle - getHeadingAngle());
            }
                   
            if(openLoopSteeringWithBase_PWM_Handler(-900, 300, 0)) {
                // openLoopSteeringWithBase_PWM_Handler返回1表示完成转向
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
            
            if(moveForwardWithAngle_Handler(targetAngle)) {
                // 返回1表示检测到黑线，即到达D点
                printf("到达D点，当前角度: %.2f\r\n", getHeadingAngle());
                Clear_Encoder(); // 清除编码器计数器
                led1_off();
                led2_on(); // 切换LED指示当前状态
                beep_on(); // 蜂鸣器提示到达D点
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
                beep_on();     // 蜂鸣器提示任务完成
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






/**
 * @brief 任务3处理函数
 * @note - 功能：  
 * 小车从 A 点 出发，按以下路径行驶：  
 * 1. 顺时针旋转50度对准C点
 * 2. 直线行驶至 C 点。  
 * 3. 沿 半圆弧黑线 从 C 行驶至 B 点。  
 * 4. 直线行驶至 D 点。  
 * 5. 在D点顺时针旋转
 * 6. 沿另一 半圆弧黑线 从 D 返回 A 点 停车。  
 * - 触发条件：每经过 A/B/C/D 点时需有声光提示。  
 * - @return u8 1:完成任务 0:小车正在执行任务中
 */
u8 Task3_Handler(void)
{
    // 定义状态机状态
    typedef enum {
        INIT_WAIT,         // 等待陀螺仪稳定
        INIT,              // 初始化状态
        INIT_DELAY,        // A点初始延时状态(LED提示)
        TURN_A_TO_C,       // A点顺时针旋转一定角度对准C点
        MOVE_A_TO_C,       // A到C直线行驶
        C_POINT_DELAY,     // C点延时状态(LED提示)
        TURN_AT_C,         // 在C点旋转
        TRACK_C_TO_B,      // C到B沿黑线循迹
        B_POINT_DELAY,     // B点延时状态(LED提示)
        TURN_AT_B,         // 在B点旋转到D点方向
        MOVE_B_TO_D,       // B到D直线行驶
        D_POINT_DELAY,     // D点延时状态(LED提示)
        TURN_AT_D,         // 在D点旋转(新增状态)
        TRACK_D_TO_A,      // D到A沿黑线循迹
        A_POINT_DELAY,     // A点延时状态(LED提示)
        COMPLETED          // 任务完成
    } TaskState;
    
    // 使用静态变量保存状态和角度
    static TaskState currentState = INIT_WAIT;
    static float targetAngle = 0.0f;     // 目标角度
    static uint32_t init_start_time = 0; // 初始化开始时间
    static float prev_angle = 0.0f;      // 上一次读取的角度
    static uint8_t retry_count = 0;      // 重试次数计数
    static uint16_t debug_print_counter = 0; // 调试信息发送计数器
    static uint16_t delay_counter = 0;   // 非阻塞延时计数器
    
    // 调试信息发送频率控制
    bool can_print_debug = false;
    if (++debug_print_counter >= DEBUG_PRINT_COUNT) {
        debug_print_counter = 0;
        can_print_debug = true;
    }
    if(resetTask_flag==1)
    {
        // 一定时间后重置任务状态和变量
        Set_Pwm(0,0);
        if(HAL_GetTick()-resetMode_start_time>RESET_WAIT_TIME)
        {
            currentState = INIT;
            init_start_time = 0;
            prev_angle = 0.0f;
            retry_count = 0; 
            delay_counter = 0;
            resetTask_flag=0;
            led1_off(); led2_off(); led3_off(); // 关闭所有LED
            printf("任务重置完成\r\n");
        }
        else
        {
            return 0;
        }
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
            
            // 初始化：记录初始航向角，设置初始速度
            #if IS_SET_INIT_ANGLE_MANUALY==1
            initialAngle = MANUAL_INIT_ANGLE; // 手动设置初始角度
            #else
            if(manual_mode == 0) {
                initialAngle = getHeadingAngle(); // 获取当前角度作为初始角度
            }
            #endif
            manual_mode = 0; // 重置手动模式标志
            printf("任务3开始: 初始角度 = %.2f\r\n", initialAngle);
            
            Set_Target_Velocity(Task3_Move_Forward_Speed); // 设置适当的速度
            all_leds_off();
            led1_on(); // A点提示
            beep_on(); // 蜂鸣器提示任务开始
            printf("从A点出发\r\n");
            
            // 计算顺时针旋转TASK3_ROTATION_ANGLE_1度的目标角度
            targetAngle = initialAngle - Task3_Rotation_Angle_1; // 顺时针旋转要减去角度值
            // 规范化角度到±180度范围
            while(targetAngle > 180.0f) {
                targetAngle -= 360.0f;
            }
            while(targetAngle < -180.0f) {
                targetAngle += 360.0f;
            }
            
            printf("A点初始旋转至C点方向，目标角度: %.2f\r\n", targetAngle);
            
            // 重置延时计数器，进入A点延时状态
            delay_counter = 0;
            currentState = INIT_DELAY;
            return 0;
            
        case INIT_DELAY:
            // A点LED提示延时状态（非阻塞）
            delay_counter++;
            if(delay_counter >= 100) { // 5ms中断，100次约等于500ms
                led1_off();
                // 重置调试打印计数器
                debug_print_counter = 0;
                // 切换到下一状态 - 先旋转对准C点
                currentState = TURN_A_TO_C;
            }
            return 0;
            
        case TURN_A_TO_C:
            // A点旋转对准C点（顺时针旋转TASK3_ROTATION_ANGLE_1度）
            if(can_print_debug) {
                printf("A点旋转对准C点: 当前角度 = %.2f, 目标角度 = %.2f\r\n", 
                       getHeadingAngle(), targetAngle);
            }
            
            if(turnToAbsoluteAngle(targetAngle)) {
                // 旋转完成，进入直线行驶状态
                printf("A点旋转完成，开始向C点直线行驶\r\n");
                Set_Target_Velocity(Task3_Move_Forward_Speed); // 设置适当的速度
                currentState = MOVE_A_TO_C;
            }
            return 0;
            
        case MOVE_A_TO_C:
            // A点到C点的直线行驶
            if(can_print_debug) {
                printf("A→C直线行驶中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(moveForwardWithAngle_UntileSomeGraySencorActived_Handler(targetAngle,FORWARD_GraySENSOR_INDEX_1)) {
                printf("到达C点，当前角度: %.2f\r\n", getHeadingAngle());
                led3_on(); // C点提示
                beep_on(); // 蜂鸣器提示到达C点
                // 重置延时计数器，进入C点延时状态
                delay_counter = 0;
                currentState = C_POINT_DELAY;
            }
            return 0;
            
        case C_POINT_DELAY: 
            led3_off();
            printf("在C点旋转至目标角度: %.2f\r\n", targetAngle);
            currentState = TURN_AT_C;
            return 0;
            
        case TURN_AT_C:
            // 使用openLoopSteering_Handler在C点进行旋转
            if(openLoopSteering_Handler(Task3_Steer_Time_C, Task3_Steer_PWM_C)) {
                // 转向完成
                printf("C点转向完成，开始C→B循迹\r\n");
                Set_Target_Velocity(Task3_line_tracking_Speed);
                Clear_Encoder(); // 清除编码器值
                currentState = TRACK_C_TO_B;
            }
            return 0;
            
        case TRACK_C_TO_B:
            // C点到B点沿黑线循迹
            if(can_print_debug) {
                printf("C→B循迹中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(lineTracking_Handler()) {
                // lineTracking_Handler返回1表示完成循迹（所有传感器都检测到白色）
                printf("到达B点，当前角度: %.2f\r\n", getHeadingAngle());
                led2_on(); // B点提示
                beep_on(); // 蜂鸣器提示到达B点

                // 计算目标角度：初始角度的反方向再逆时针旋转TASK3_ROTATION_ANGLE_2度
                // 初始方向的反方向 = 初始角度 + 180度
                // 再逆时针偏转Task3_Rotation_Angle_2度 = 再加上Task3_Rotation_Angle_2度
                targetAngle = initialAngle + 180.0f + Task3_Rotation_Angle_2;
                
                // 规范化角度到±180度范围
                while(targetAngle > 180.0f) {
                    targetAngle -= 360.0f;
                }
                while(targetAngle < -180.0f) {
                    targetAngle += 360.0f;
                }
                
                // 重置延时计数器，进入B点延时状态
                delay_counter = 0;
                currentState = B_POINT_DELAY;
            }
            return 0;
            
        case B_POINT_DELAY:
            // B点LED提示延时状态（非阻塞）
            delay_counter++;
            if(delay_counter >= (TASK3_B_PONIT_DELAY_TIME/5)) { // 5ms中断，100次约等于500ms
                led2_off();
                printf("在B点旋转至目标角度: %.2f\r\n", targetAngle);
                currentState = TURN_AT_B;
            }
            return 0;
            
        case TURN_AT_B:
            // 在B点旋转至目标角度（使用绝对角度旋转函数）
            if(can_print_debug) {
                printf("B点旋转：当前角度 = %.2f, 目标角度 = %.2f, 差值 = %.2f\r\n", 
                      getHeadingAngle(), targetAngle, targetAngle - getHeadingAngle());
            }
            
            if(turnToAbsoluteAngle(targetAngle)) {
                // turnToAbsoluteAngle返回1表示完成转向
                printf("B点转向完成，目标角度: %.2f，当前角度: %.2f\r\n", 
                       targetAngle, getHeadingAngle());
                Set_Target_Velocity(Task3_Move_Forward_Speed); // 设置适当的速度
                currentState = MOVE_B_TO_D;
            }
            return 0;
            
        case MOVE_B_TO_D:
            // B点到D点的直线行驶
            if(can_print_debug) {
                printf("B→D直线行驶中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(moveForwardWithAngle_UntileSomeGraySencorActived_Handler(targetAngle,FORWARD_GraySENSOR_INDEX_2)) { 
                // moveForwardWithAngle_Handler返回1表示检测到黑线，即到达D点
                printf("到达D点，当前角度: %.2f\r\n", getHeadingAngle());
                all_leds_off();
                led2_on(); // D点提示
                beep_on(); // 蜂鸣器提示到达D点
                // 重置延时计数器，进入D点延时状态
                delay_counter = 0;
                currentState = D_POINT_DELAY;
            }
            return 0;
            
        case D_POINT_DELAY:
                led2_off();
                printf("在D点进行顺时针旋转\r\n");
                currentState = TURN_AT_D; // 进入D点旋转状态
            return 0;
            
        case TURN_AT_D:
            // 使用openLoopSteering_Handler在D点进行顺时针旋转
            if(can_print_debug) {
                printf("D点顺时针旋转中：当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(openLoopSteering_Handler(Task3_Steer_Time_D, Task3_Steer_PWM_D)) {
                // 转向完成
                printf("D点转向完成，开始D→A循迹\r\n");
                Set_Target_Velocity(Task3_line_tracking_Speed); // 设置适当的速度
                Clear_Encoder(); // 清除编码器值
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
                all_leds_on(); // 所有LED亮起表示任务完成
                beep_on();     // 蜂鸣器提示任务完成
                Set_Pwm(0, 0); // 停止电机
                
                // 重置延时计数器，进入A点延时状态
                delay_counter = 0;
                currentState = A_POINT_DELAY;
            }
            return 0;
            
        case A_POINT_DELAY:
            // A点任务完成LED提示延时状态（非阻塞）
            delay_counter++;
            if(delay_counter >= (TASK3_INTERVAL/5)) { // 5ms中断，TASK3_INTERVAL次约等于TASK3_INTERVALms
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








/**
 * @brief 任务4处理函数 - 重复执行类似任务3的循环四次
 * @note - 功能：  
 * 小车重复执行以下路径行驶四次：  
 * 1. 顺时针旋转对准C点
 * 2. 直线行驶至 C 点。  
 * 3. 沿 半圆弧黑线 从 C 行驶至 B 点。  
 * 4. 直线行驶至 D 点。  
 * 5. 在D点顺时针旋转
 * 6. 沿另一 半圆弧黑线 从 D 返回 A 点 停车。  
 * - 触发条件：每经过 A/B/C/D 点时需有声光提示。  
 * - @return u8 1:完成任务 0:小车正在执行任务中
 */
u8 Task4_Handler(void)
{
    // 定义状态机状态
    typedef enum {
        INIT_WAIT,         // 等待陀螺仪稳定
        INIT,              // 初始化状态
        INIT_DELAY,        // A点初始延时状态(LED提示)
        TURN_A_TO_C,       // A点顺时针旋转一定角度对准C点
        MOVE_A_TO_C,       // A到C直线行驶
        C_POINT_DELAY,     // C点延时状态(LED提示)
        TURN_AT_C,         // 在C点旋转
        TRACK_C_TO_B,      // C到B沿黑线循迹
        B_POINT_DELAY,     // B点延时状态(LED提示)
        TURN_AT_B,         // 在B点旋转到D点方向
        MOVE_B_TO_D,       // B到D直线行驶
        D_POINT_DELAY,     // D点延时状态(LED提示)
        TURN_AT_D,         // 在D点旋转(新增状态)
        TRACK_D_TO_A,      // D到A沿黑线循迹
        A_POINT_DELAY,     // A点延时状态(LED提示)
        CYCLE_COMPLETE,    // 一次循环完成
        COMPLETED          // 任务完成
    } TaskState;
    
    // 使用静态变量保存状态和角度
    static TaskState currentState = INIT_WAIT;
    static float targetAngle = 0.0f;     // 目标角度
    static uint32_t init_start_time = 0; // 初始化开始时间
    static float prev_angle = 0.0f;      // 上一次读取的角度
    static uint8_t retry_count = 0;      // 重试次数计数
    static uint16_t debug_print_counter = 0; // 调试信息发送计数器
    static uint16_t delay_counter = 0;   // 非阻塞延时计数器
    static uint8_t cycle_count = 0;      // 循环计数器，记录完成的循环次数
    
    // 调试信息发送频率控制
    bool can_print_debug = false;
    if (++debug_print_counter >= DEBUG_PRINT_COUNT) {
        debug_print_counter = 0;
        can_print_debug = true;
    }
    
    if(resetTask_flag==1)
    {
        // 一定时间后重置任务状态和变量
        Set_Pwm(0,0);
        if(HAL_GetTick()-resetMode_start_time>RESET_WAIT_TIME)
        {
            currentState = INIT;
            init_start_time = 0;
            prev_angle = 0.0f;
            retry_count = 0; 
            delay_counter = 0;
            cycle_count = 0;  // 重置循环计数
            resetTask_flag=0;
            led1_off(); led2_off(); led3_off(); // 关闭所有LED
            printf("任务重置完成\r\n");
        }
        else
        {
            return 0;
        }
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
            // 初始化：记录初始航向角，设置初始速度
            // 只在第一次循环记录初始角度
            if (cycle_count == 0) {
                #if IS_SET_INIT_ANGLE_MANUALY==1
                initialAngle = MANUAL_INIT_ANGLE; // 手动设置初始角度
                #else
                if(manual_mode == 0) {
                    initialAngle = getHeadingAngle(); // 获取当前角度作为初始角度
                }
                #endif
                manual_mode = 0; // 重置手动模式标志
            }
            
            printf("任务4 第%d圈开始: 初始角度 = %.2f\r\n", cycle_count + 1, initialAngle);
            
            Set_Target_Velocity(Task4_Move_Forward_Speed); // 使用全局变量
            all_leds_off();
            led1_on(); // A点提示
            beep_on(); // 蜂鸣器提示任务开始
            printf("从A点出发\r\n");
            
            // 计算顺时针旋转的目标角度
            if(cycle_count == 0) {
                targetAngle = initialAngle - Task4_Rotation_Angle_1; // 使用全局变量
            } else {
                targetAngle = initialAngle - Task4_Rotation_Angle_3; // 使用全局变量
            }
            // 规范化角度到±180度范围
            while(targetAngle > 180.0f) {
                targetAngle -= 360.0f;
            }
            while(targetAngle < -180.0f) {
                targetAngle += 360.0f;
            }
            
            printf("A点初始旋转至C点方向，目标角度: %.2f\r\n", targetAngle);
            
            // 重置延时计数器，进入A点延时状态
            delay_counter = 0;
            currentState = INIT_DELAY;
            return 0;
            
        case INIT_DELAY:
            // A点LED提示延时状态（非阻塞）
                led1_off();
                // 重置调试打印计数器
                debug_print_counter = 0;
                // 切换到下一状态 - 先旋转对准C点
                currentState = TURN_A_TO_C;
            return 0;
            
        case TURN_A_TO_C:
            // A点旋转对准C点
            if(can_print_debug) {
                printf("A点旋转对准C点: 当前角度 = %.2f, 目标角度 = %.2f\r\n", 
                       getHeadingAngle(), targetAngle);
            }
            
            if(turnToAbsoluteAngle(targetAngle)) {
                // 旋转完成，进入直线行驶状态
                printf("A点旋转完成，开始向C点直线行驶\r\n");
                Set_Target_Velocity(Task4_Move_Forward_Speed); // 使用全局变量
                currentState = MOVE_A_TO_C;
            }
            return 0;
            
        case MOVE_A_TO_C:
            // A点到C点的直线行驶
            if(can_print_debug) {
                printf("A→C直线行驶中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            if(cycle_count==0) 
            {
                if(moveForwardWithAngle_UntileSomeGraySencorActived_Handler(targetAngle,FORWARD_GraySENSOR_INDEX_1)) {
                    // 返回1表示检测到黑线，即到达C点
                    printf("到达C点，当前角度: %.2f\r\n", getHeadingAngle());
                    led3_on(); // C点提示
                    beep_on(); // 蜂鸣器提示到达C点
                    // 重置延时计数器，进入C点延时状态
                    delay_counter = 0;
                    currentState = C_POINT_DELAY;
                }
            }
            else 
            {
                if(moveForwardWithAngle_UntileSomeGraySencorActived_Handler(targetAngle,FORWARD_GraySENSOR_INDEX_1)) {
                    // 返回1表示检测到黑线，即到达C点
                    printf("到达C点，当前角度: %.2f\r\n", getHeadingAngle());
                    led3_on(); // C点提示
                    beep_on(); // 蜂鸣器提示到达C点
                    // 重置延时计数器，进入C点延时状态
                    delay_counter = 0;
                    currentState = C_POINT_DELAY;
                }
            }
            return 0;
            
        case C_POINT_DELAY: 
            led3_off();
            printf("在C点旋转至目标角度: %.2f\r\n", targetAngle);
            currentState = TURN_AT_C;
            return 0;
            
        case TURN_AT_C:
            // 使用openLoopSteering_Handler在C点进行旋转
            if(openLoopSteering_Handler(Task4_Steer_Time_C, Task4_Steer_PWM_C)) { // 使用全局变量
                // 转向完成
                printf("C点转向完成，开始C→B循迹\r\n");
                Set_Target_Velocity(Task4_Line_Tracking_Speed); // 使用全局变量
                Clear_Encoder(); // 清除编码器值
                currentState = TRACK_C_TO_B;
            }
            return 0;
            
        case TRACK_C_TO_B:
            // C点到B点沿黑线循迹
            if(can_print_debug) {
                printf("C→B循迹中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(lineTracking_Handler()) {
                // lineTracking_Handler返回1表示完成循迹（所有传感器都检测到白色）
                printf("到达B点，当前角度: %.2f\r\n", getHeadingAngle());
                led2_on(); // B点提示
                beep_on(); // 蜂鸣器提示到达B点

                // 计算目标角度：初始角度的反方向再逆时针旋转Task4_Rotation_Angle_2度
                // 初始方向的反方向 = 初始角度 + 180度
                // 再逆时针偏转Task4_Rotation_Angle_2度 = 再加上Task4_Rotation_Angle_2度
                targetAngle = initialAngle + 180.0f + Task4_Rotation_Angle_2; // 使用全局变量
                
                // 规范化角度到±180度范围
                while(targetAngle > 180.0f) {
                    targetAngle -= 360.0f;
                }
                while(targetAngle < -180.0f) {
                    targetAngle += 360.0f;
                }
                
                // 重置延时计数器，进入B点延时状态
                delay_counter = 0;
                currentState = B_POINT_DELAY;
            }
            return 0;
            
        case B_POINT_DELAY:
            // B点LED提示延时状态（非阻塞）
            delay_counter++;
            if(delay_counter >= (Task4_B_Point_Delay_Time/5)) { // 使用全局变量
                led2_off();
                printf("在B点旋转至目标角度: %.2f\r\n", targetAngle);
                currentState = TURN_AT_B;
            }
            return 0;
            
        case TURN_AT_B:
            // 在B点旋转至目标角度（使用绝对角度旋转函数）
            if(can_print_debug) {
                printf("B点旋转：当前角度 = %.2f, 目标角度 = %.2f, 差值 = %.2f\r\n", 
                      getHeadingAngle(), targetAngle, targetAngle - getHeadingAngle());
            }
            
            if(turnToAbsoluteAngle(targetAngle)) {
                // turnToAbsoluteAngle返回1表示完成转向
                printf("B点转向完成，目标角度: %.2f，当前角度: %.2f\r\n", 
                       targetAngle, getHeadingAngle());
                Set_Target_Velocity(Task4_Move_Forward_Speed); // 使用全局变量
                currentState = MOVE_B_TO_D;
            }
            return 0;
            
        case MOVE_B_TO_D:
            // B点到D点的直线行驶
            if(can_print_debug) {
                printf("B→D直线行驶中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(moveForwardWithAngle_UntileSomeGraySencorActived_Handler(targetAngle,FORWARD_GraySENSOR_INDEX_2)) { 
                // 返回1表示检测到黑线，即到达D点
                printf("到达D点，当前角度: %.2f\r\n", getHeadingAngle());
                all_leds_off();
                led2_on(); // D点提示
                beep_on(); // 蜂鸣器提示到达D点
                // 重置延时计数器，进入D点延时状态
                delay_counter = 0;
                currentState = D_POINT_DELAY;
            }
            return 0;
            
        case D_POINT_DELAY:
                led2_off();
                printf("在D点进行顺时针旋转\r\n");
                currentState = TURN_AT_D; // 进入D点旋转状态
            return 0;
            
        case TURN_AT_D:
            // 使用openLoopSteering_Handler在D点进行顺时针旋转
            if(can_print_debug) {
                printf("D点顺时针旋转中：当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(openLoopSteering_Handler(Task4_Steer_Time_D, Task4_Steer_PWM_D)) { // 使用全局变量
                // 转向完成
                printf("D点转向完成，开始D→A循迹\r\n");
                Set_Target_Velocity(Task4_Line_Tracking_Speed); // 使用全局变量
                Clear_Encoder(); // 清除编码器值
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
                printf("到达A点, 第%d圈完成，当前角度: %.2f\r\n", cycle_count + 1, getHeadingAngle());
                all_leds_on(); // 所有LED亮起表示该圈完成
                beep_on(); // 蜂鸣器提示任务完成
                Set_Pwm(0, 0); // 停止电机
                
                // 重置延时计数器，进入A点延时状态
                delay_counter = 0;
                currentState = A_POINT_DELAY;
            }
            return 0;
            
        case A_POINT_DELAY:
            // A点任务完成LED提示延时状态（非阻塞）
            delay_counter++;
            if(delay_counter >= (Task4_Interval/5)) { // 使用全局变量
                currentState = CYCLE_COMPLETE;
            }
            return 0;
            
        case CYCLE_COMPLETE:
            // 一个循环完成，检查是否需要继续下一圈
            cycle_count++;
            all_leds_off();
            
            if(cycle_count >= Task4_Cycle_Count) { // 使用全局变量
                // 所有圈数完成
                printf("任务4所有%d圈已完成!\r\n", Task4_Cycle_Count);
                currentState = COMPLETED;
            } else {
                // 继续下一圈
                printf("开始第%d/%d圈...\r\n", cycle_count + 1, Task4_Cycle_Count);
                delay_counter = 0;
                currentState = INIT; // 返回到初始化状态开始下一圈
            }
            return 0;
            
        case COMPLETED:
            // 任务完成状态
            // 重置状态，为下次任务做准备
            currentState = INIT_WAIT;
            cycle_count = 0; // 重置循环计数
            all_leds_off();
            return 1; // 返回1表示任务完成
            
        default:
            // 异常情况，重置状态
            currentState = INIT_WAIT;
            cycle_count = 0; // 重置循环计数
            return 0;
    }
}
