#include "Task.h"
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
        TURN_A_TO_C,       // A点顺时针旋转50度对准C点
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
    static float initialAngle = 0.0f;    // 初始航向角
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
            initialAngle = getHeadingAngle();
            printf("任务3开始: 初始角度 = %.2f\r\n", initialAngle);
            
            Set_Target_Velocity(16); // 设置适当的速度
            all_leds_off();
            led1_on(); // A点提示
            printf("从A点出发\r\n");
            
            // 计算顺时针旋转TASK3_ROTATION_ANGLE_1度的目标角度
            targetAngle = initialAngle - TASK3_ROTATION_ANGLE_1; // 顺时针旋转要减去角度值
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
                currentState = MOVE_A_TO_C;
            }
            return 0;
            
        case MOVE_A_TO_C:
            // A点到C点的直线行驶
            if(can_print_debug) {
                printf("A→C直线行驶中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(moveForward_Handler()) {
                // moveForward_Handler返回1表示检测到黑线，即到达C点
                printf("到达C点，当前角度: %.2f\r\n", getHeadingAngle());
                led3_on(); // C点提示
                
                // 计算顺时针旋转TASK3_ROTATION_ANGLE_2度的目标角度
                targetAngle = getHeadingAngle() - TASK3_ROTATION_ANGLE_2; // 顺时针旋转要减去角度值
                // 规范化角度到±180度范围
                while(targetAngle > 180.0f) {
                    targetAngle -= 360.0f;
                }
                while(targetAngle < -180.0f) {
                    targetAngle += 360.0f;
                }
                
                // 重置延时计数器，进入C点延时状态
                delay_counter = 0;
                currentState = C_POINT_DELAY;
            }
            return 0;
            
        case C_POINT_DELAY:
            // C点LED提示延时状态（非阻塞）
            delay_counter++;
            if(delay_counter >= 100) { // 5ms中断，100次约等于500ms
                led3_off();
                printf("在C点旋转至目标角度: %.2f\r\n", targetAngle);
                currentState = TURN_AT_C;
            }
            return 0;
            
        case TURN_AT_C:
            // 使用openLoopSteering_Handler在C点进行旋转
            if(openLoopSteering_Handler(TASK3_STEER_TIME_C, TASK3_STEER_PWM_C)) {
                // 转向完成
                printf("C点转向完成，开始C→B循迹\r\n");
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
                
                // 计算目标角度：初始角度的反方向再逆时针旋转TASK3_ROTATION_ANGLE_3度
                // 初始方向的反方向 = 初始角度 + 180度
                // 再逆时针偏转TASK3_ROTATION_ANGLE_3度 = 再加上TASK3_ROTATION_ANGLE_3度
                targetAngle = initialAngle + 180.0f + TASK3_ROTATION_ANGLE_3;
                
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
            if(delay_counter >= 100) { // 5ms中断，100次约等于500ms
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
                currentState = MOVE_B_TO_D;
            }
            return 0;
            
        case MOVE_B_TO_D:
            // B点到D点的直线行驶
            if(can_print_debug) {
                printf("B→D直线行驶中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(moveForward_Handler()) {
                // moveForward_Handler返回1表示检测到黑线，即到达D点
                printf("到达D点，当前角度: %.2f\r\n", getHeadingAngle());
                all_leds_off();
                led2_on(); // D点提示
                
                // 重置延时计数器，进入D点延时状态
                delay_counter = 0;
                currentState = D_POINT_DELAY;
            }
            return 0;
            
        case D_POINT_DELAY:
            // D点LED提示延时状态（非阻塞）
            delay_counter++;
            if(delay_counter >= 100) { // 5ms中断，100次约等于500ms
                led2_off();
                printf("在D点进行顺时针旋转\r\n");
                currentState = TURN_AT_D; // 进入D点旋转状态
            }
            return 0;
            
        case TURN_AT_D:
            // 使用openLoopSteering_Handler在D点进行顺时针旋转
            if(can_print_debug) {
                printf("D点顺时针旋转中：当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(openLoopSteering_Handler(TASK3_STEER_TIME_D, TASK3_STEER_PWM_D)) {
                // 转向完成
                printf("D点转向完成，开始D→A循迹\r\n");
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
                Set_Pwm(0, 0); // 停止电机
                
                // 重置延时计数器，进入A点延时状态
                delay_counter = 0;
                currentState = A_POINT_DELAY;
            }
            return 0;
            
        case A_POINT_DELAY:
            // A点任务完成LED提示延时状态（非阻塞）
            delay_counter++;
            if(delay_counter >= 200) { // 5ms中断，200次约等于1000ms
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
        TURN_A_TO_C,       // A点顺时针旋转对准C点
        MOVE_A_TO_C,       // A到C直线行驶
        C_POINT_DELAY,     // C点延时状态(LED提示)
        TURN_AT_C,         // 在C点旋转
        TRACK_C_TO_B,      // C到B沿黑线循迹
        B_POINT_DELAY,     // B点延时状态(LED提示)
        TURN_AT_B,         // 在B点旋转到D点方向
        MOVE_B_TO_D,       // B到D直线行驶
        D_POINT_DELAY,     // D点延时状态(LED提示)
        TURN_AT_D,         // 在D点旋转
        TRACK_D_TO_A,      // D到A沿黑线循迹
        A_POINT_DELAY,     // A点延时状态(LED提示)
        CYCLE_DELAY,       // 循环之间的延时状态
        COMPLETED          // 任务完成
    } TaskState;
    
    // 使用静态变量保存状态和角度
    static TaskState currentState = INIT_WAIT;
    static float initialAngle = 0.0f;    // 初始航向角
    static float targetAngle = 0.0f;     // 目标角度
    static uint32_t init_start_time = 0; // 初始化开始时间
    static float prev_angle = 0.0f;      // 上一次读取的角度
    static uint8_t retry_count = 0;      // 重试次数计数
    static uint16_t debug_print_counter = 0; // 调试信息发送计数器
    static uint16_t delay_counter = 0;   // 非阻塞延时计数器
    static uint8_t cycle_count = 0;      // 循环计数器，记录已完成的循环次数
    
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
                cycle_count = 0;   // 初始化循环计数器
            }
            return 0;
            
        case INIT:
            // 初始化：记录初始航向角，设置初始速度
            initialAngle = getHeadingAngle();
            printf("任务4开始: 初始角度 = %.2f (循环 %d/%d)\r\n", initialAngle, cycle_count+1, TASK4_CYCLE_COUNT);
            
            Set_Target_Velocity(16); // 设置适当的速度
            all_leds_off();
            led1_on(); // A点提示
            printf("从A点出发\r\n");
            
            // 计算顺时针旋转TASK4_ROTATION_ANGLE_1度的目标角度
            targetAngle = initialAngle - TASK4_ROTATION_ANGLE_1; // 顺时针旋转要减去角度值
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
            // A点旋转对准C点（顺时针旋转TASK4_ROTATION_ANGLE_1度）
            if(can_print_debug) {
                printf("A点旋转对准C点: 当前角度 = %.2f, 目标角度 = %.2f\r\n", 
                       getHeadingAngle(), targetAngle);
            }
            
            if(turnToAbsoluteAngle(targetAngle)) {
                // 旋转完成，进入直线行驶状态
                printf("A点旋转完成，开始向C点直线行驶\r\n");
                currentState = MOVE_A_TO_C;
            }
            return 0;
            
        case MOVE_A_TO_C:
            // A点到C点的直线行驶
            if(can_print_debug) {
                printf("A→C直线行驶中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(moveForward_Handler()) {
                // moveForward_Handler返回1表示检测到黑线，即到达C点
                printf("到达C点，当前角度: %.2f\r\n", getHeadingAngle());
                led3_on(); // C点提示
                
                // 计算顺时针旋转TASK4_ROTATION_ANGLE_2度的目标角度
                targetAngle = getHeadingAngle() - TASK4_ROTATION_ANGLE_2; // 顺时针旋转要减去角度值
                // 规范化角度到±180度范围
                while(targetAngle > 180.0f) {
                    targetAngle -= 360.0f;
                }
                while(targetAngle < -180.0f) {
                    targetAngle += 360.0f;
                }
                
                // 重置延时计数器，进入C点延时状态
                delay_counter = 0;
                currentState = C_POINT_DELAY;
            }
            return 0;
            
        case C_POINT_DELAY:
            // C点LED提示延时状态（非阻塞）
            delay_counter++;
            if(delay_counter >= 100) { // 5ms中断，100次约等于500ms
                led3_off();
                printf("在C点旋转至目标角度: %.2f\r\n", targetAngle);
                currentState = TURN_AT_C;
            }
            return 0;
            
        case TURN_AT_C:
            // 使用openLoopSteering_Handler在C点进行旋转
            if(openLoopSteering_Handler(TASK4_STEER_TIME_C, TASK4_STEER_PWM_C)) {
                // 转向完成
                printf("C点转向完成，开始C→B循迹\r\n");
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
                
                // 计算目标角度：初始角度的反方向再逆时针旋转TASK4_ROTATION_ANGLE_3度
                // 初始方向的反方向 = 初始角度 + 180度
                // 再逆时针偏转TASK4_ROTATION_ANGLE_3度 = 再加上TASK4_ROTATION_ANGLE_3度
                targetAngle = initialAngle + 180.0f + TASK4_ROTATION_ANGLE_3;
                
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
            if(delay_counter >= 100) { // 5ms中断，100次约等于500ms
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
                currentState = MOVE_B_TO_D;
            }
            return 0;
            
        case MOVE_B_TO_D:
            // B点到D点的直线行驶
            if(can_print_debug) {
                printf("B→D直线行驶中: 当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(moveForward_Handler()) {
                // moveForward_Handler返回1表示检测到黑线，即到达D点
                printf("到达D点，当前角度: %.2f\r\n", getHeadingAngle());
                all_leds_off();
                led2_on(); // D点提示
                
                // 重置延时计数器，进入D点延时状态
                delay_counter = 0;
                currentState = D_POINT_DELAY;
            }
            return 0;
            
        case D_POINT_DELAY:
            // D点LED提示延时状态（非阻塞）
            delay_counter++;
            if(delay_counter >= 100) { // 5ms中断，100次约等于500ms
                led2_off();
                printf("在D点进行顺时针旋转\r\n");
                currentState = TURN_AT_D; // 进入D点旋转状态
            }
            return 0;
            
        case TURN_AT_D:
            // 使用openLoopSteering_Handler在D点进行顺时针旋转
            if(can_print_debug) {
                printf("D点顺时针旋转中：当前角度 = %.2f\r\n", getHeadingAngle());
            }
            
            if(openLoopSteering_Handler(TASK4_STEER_PWM_D, TASK4_STEER_PWM_D)) {
                // 转向完成
                printf("D点转向完成，开始D→A循迹\r\n");
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
                printf("到达A点, 循环%d完成，当前角度: %.2f\r\n", cycle_count+1, getHeadingAngle());
                all_leds_on(); // 所有LED亮起表示一次循环完成
                
                // 增加循环计数
                cycle_count++;
                
                // 检查是否已完成所有循环
                if(cycle_count >= TASK4_CYCLE_COUNT) {
                    printf("所有循环已完成，任务结束！\r\n");
                    Set_Pwm(0, 0); // 停止电机
                    // 重置延时计数器，进入A点延时状态
                    delay_counter = 0;
                    currentState = A_POINT_DELAY;
                } else {
                    // 未完成所有循环，准备下一次循环
                    printf("准备开始第%d/%d次循环...\r\n", cycle_count+1, TASK4_CYCLE_COUNT);
                    // 重置延时计数器，进入循环延时状态
                    delay_counter = 0;
                    currentState = CYCLE_DELAY;
                }
            }
            return 0;
            
        case CYCLE_DELAY:
            // 循环之间的延时状态（非阻塞）
            delay_counter++;
            
            // LED闪烁效果，指示等待中
            if(delay_counter % 40 == 0) {
                all_leds_toggle();
            }
            
            if(delay_counter >= TASK4_CYCLE_DELAY) { // 等待一段时间后开始下一次循环
                all_leds_off();
                // 重新初始化下一次循环
                currentState = INIT;
            }
            return 0;
            
        case A_POINT_DELAY:
            // A点任务完成LED提示延时状态（非阻塞）
            delay_counter++;
            if(delay_counter >= 200) { // 5ms中断，200次约等于1000ms
                currentState = COMPLETED;
            }
            return 0;
            
        case COMPLETED:
            // 任务完成状态
            // 重置状态，为下次任务做准备
            currentState = INIT_WAIT;
            cycle_count = 0;
            all_leds_off();
            return 1; // 返回1表示任务完成
            
        default:
            // 异常情况，重置状态
            currentState = INIT_WAIT;
            cycle_count = 0;
            return 0;
    }
}
