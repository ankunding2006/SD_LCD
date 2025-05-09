#include "setParma.h"
#include <stdio.h>

// 定义Task3相关参数变量，初始值设置为原宏定义值
float Task3_Rotation_Angle_1 = TASK3_ROTATION_ANGLE_1;     // 任务3中初始从A点对准C点需要顺时针旋转的角度(度)
float Task3_Rotation_Angle_2 = TASK3_ROTATION_ANGLE_2;     // 任务3中从B到D前需要逆时针旋转的角度(度)
int Task3_Steer_Time_C = TASK3_STEER_TIME_C;               // C点openLoopSteering的转向时间参数(中断次数)
int Task3_Steer_PWM_C = TASK3_STEER_PWM_C;                 // C点openLoopSteering的PWM参数(速度值)
int Task3_Steer_Time_D = TASK3_STEER_TIME_D;               // D点openLoopSteering的转向时间参数(中断次数)
int Task3_Steer_PWM_D = TASK3_STEER_PWM_D;                 // D点openLoopSteering的PWM参数(速度值)
int Task3_Move_Forward_Speed = TASK3_MOVE_FORWARD_SPEED;   // 任务3中直线行驶的速度(速度值)
int Task3_line_tracking_Speed = TASK3_LINE_TRACKING_SPEED; // 任务3中循迹行驶的速度(速度值)

// 定义Task4相关参数变量，初始值设置为宏定义值
float Task4_Rotation_Angle_1 = TASK4_ROTATION_ANGLE_1;     // 任务4中初始从A点对准C点需要顺时针旋转的角度(度)
float Task4_Rotation_Angle_2 = TASK4_ROTATION_ANGLE_2;     // 任务4中从B到D前需要逆时针旋转的角度(度)
float Task4_Rotation_Angle_3 = TASK4_ROTATION_ANGLE_3;     // 任务4中后3圈从B到D前需要逆时针旋转的角度(度)
int Task4_Steer_Time_C = TASK4_STEER_TIME_C;               // C点openLoopSteering的转向时间参数(中断次数)
int Task4_Steer_PWM_C = TASK4_STEER_PWM_C;                 // C点openLoopSteering的PWM参数(速度值)
int Task4_Steer_Time_D = TASK4_STEER_TIME_D;               // D点openLoopSteering的转向时间参数(中断次数)
int Task4_Steer_PWM_D = TASK4_STEER_PWM_D;                 // D点openLoopSteering的PWM参数(速度值)
int Task4_Cycle_Count = TASK4_CYCLE_COUNT;                 // 任务4循环执行次数
int Task4_Interval = TASK4_INTERVAL;                       // 任务间隔时间(毫秒)
int Task4_Move_Forward_Speed = TASK4_MOVE_FORWARD_SPEED;   // 任务4中直线行驶的速度(速度值)
int Task4_Line_Tracking_Speed = TASK4_LINE_TRACKING_SPEED; // 任务4中循迹行驶的速度(速度值)
int Task4_B_Point_Delay_Time = TASK4_B_PONIT_DELAY_TIME;

/**
 * @brief Flag_Stop取反
 * @retval None
 */
void toggle_Flag_Stop(void)
{
    car_state.running.flag_stop = !car_state.running.flag_stop;
}

/**
 * @brief 关闭TIM6中断或开启TIM6中断(反转TIM6中断)
 * @retval None
 */
void HAL_TIM6_toggle_IT(void)
{
    if (HAL_TIM_Base_GetState(&htim6) == HAL_TIM_STATE_READY)
    {
        HAL_TIM_Base_Start_IT(&htim6); // 开启TIM6中断
    }
    else
    {
        HAL_TIM_Base_Stop_IT(&htim6); // 关闭TIM6中断
    }
}

/**
 * @brief 设置开环转向基础PWM值
 * @param pwm 新的开环转向基础PWM值
 * @return 无
 */
void Set_openLoopSteeringBase_PWM(u16 pwm)
{
    car_state.running.openloop_steering_base_pwm = pwm;
    printf("设置开环转向基础PWM值: PWM = %d\r\n", pwm);
}

/**
 * @brief 获取开环转向基础PWM值
 * @return 当前开环转向基础PWM值
 */

u16 Get_openLoopSteeringBase_PWM(void)
{
    printf("当前开环转向基础PWM值: PWM = %d\r\n", car_state.running.openloop_steering_base_pwm);
    return car_state.running.openloop_steering_base_pwm;
}

/**
 * @brief 设置转向控制比例系数
 * @param kp 新的比例系数(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Kp(u16 kp)
{
    car_state.pid.steering_kp = kp;
    printf("设置转向控制比例系数: Kp = %.3f\r\n", (float)kp / 100.0f);
}

/**
 * @brief 设置转向控制积分系数
 * @param ki 新的积分系数(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Ki(u16 ki)
{
    car_state.pid.steering_ki = ki;
    printf("设置转向控制积分系数: Ki = %.3f\r\n", (float)ki / 100.0f);
}

/**
 * @brief 设置转向控制微分系数
 * @param kd 新的微分系数(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Kd(u16 kd)
{
    car_state.pid.steering_kd = kd;
    printf("设置转向控制微分系数: Kd = %.3f\r\n", (float)kd / 100.0f);
}

/**
 * @brief 设置转向控制误差阈值
 * @param threshold 新的误差阈值(度)(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Error_Threshold(u16 threshold)
{
    car_state.pid.steering_error_threshold = threshold;
    printf("设置转向控制误差阈值: %.3f度\r\n", (float)threshold / 100.0f);
}

/**
 * @brief 一次性设置所有转向控制参数
 * @param kp 比例系数(放大100倍的整数)
 * @param ki 积分系数(放大100倍的整数)
 * @param kd 微分系数(放大100倍的整数)
 * @param threshold 误差阈值(度)(放大100倍的整数)
 * @return 无
 */
void Set_All_Steering_Params(u16 kp, u16 ki, u16 kd, u16 threshold)
{
    car_state.pid.steering_kp = kp;
    car_state.pid.steering_ki = ki;
    car_state.pid.steering_kd = kd;
    car_state.pid.steering_error_threshold = threshold;
    printf("设置所有转向控制参数:\r\n");
    printf("Kp = %.3f, Ki = %.3f, Kd = %.3f\r\n", (float)kp / 100.0f, (float)ki / 100.0f, (float)kd / 100.0f);
    printf("误差阈值 = %.3f度\r\n", (float)threshold / 100.0f);
}

/**
 * @brief 获取转向控制比例系数
 * @return 当前比例系数
 */
float Get_Steering_Kp(void)
{
    float kp = (float)car_state.pid.steering_kp / 100.0f;
    printf("当前转向控制比例系数: Kp = %.3f\r\n", kp);
    return kp;
}

/**
 * @brief 获取转向控制积分系数
 * @return 当前积分系数
 */
float Get_Steering_Ki(void)
{
    float ki = (float)car_state.pid.steering_ki / 100.0f;
    printf("当前转向控制积分系数: Ki = %.3f\r\n", ki);
    return ki;
}

/**
 * @brief 获取转向控制微分系数
 * @return 当前微分系数
 */
float Get_Steering_Kd(void)
{
    float kd = (float)car_state.pid.steering_kd / 100.0f;
    printf("当前转向控制微分系数: Kd = %.3f\r\n", kd);
    return kd;
}

/**
 * @brief 获取转向控制误差阈值
 * @return 当前误差阈值(度)
 */
float Get_Steering_Error_Threshold(void)
{
    float threshold = (float)car_state.pid.steering_error_threshold / 100.0f;
    printf("当前转向控制误差阈值: %.3f度\r\n", threshold);
    return threshold;
}

/**
 * @brief 设置直线行走基础PWM值
 * @param pwm 新的基础PWM值(整数)
 * @return 无
 */
void Set_forwardBase_PWM(u16 pwm)
{
    car_state.running.forward_base_pwm = pwm;
    printf("设置直线行走基础PWM值: PWM = %u\r\n", pwm);
}

/**
 * @brief 设置直线行走角度修正比例系数
 * @param kp 新的比例系数(放大100倍的整数)
 * @return 无
 */
void Set_Forward_Kp(u16 kp)
{
    car_state.pid.forward_kp = kp;
    printf("设置直线行走角度修正比例系数: Kp = %.3f\r\n", (float)kp / 100.0f);
}

/**
 * @brief 设置直线行走角度修正积分系数
 * @param ki 新的积分系数(放大100倍的整数)
 * @return 无
 */
void Set_Forward_Ki(u16 ki)
{
    car_state.pid.forward_ki = ki;
    printf("设置直线行走角度修正积分系数: Ki = %.3f\r\n", (float)ki / 100.0f);
}

/**
 * @brief 设置直线行走角度修正微分系数
 * @param kd 新的微分系数(放大100倍的整数)
 * @return 无
 */
void Set_Forward_Kd(u16 kd)
{
    car_state.pid.forward_kd = kd;
    printf("设置直线行走角度修正微分系数: Kd = %.3f\r\n", (float)kd / 100.0f);
}

/**
 * @brief 设置直线行走角度修正误差阈值
 * @param threshold 新的误差阈值(度)(放大100倍的整数)
 * @return 无
 */
void Set_Forward_Error_Threshold(u16 threshold)
{
    car_state.pid.forward_error_threshold = threshold;
    printf("设置直线行走角度修正误差阈值: %.3f度\r\n", (float)threshold / 100.0f);
}

/**
 * @brief 一次性设置所有直线行走角度修正参数
 * @param kp 比例系数(放大100倍的整数)
 * @param ki 积分系数(放大100倍的整数)
 * @param kd 微分系数(放大100倍的整数)
 * @param threshold 误差阈值(度)(放大100倍的整数)
 * @return 无
 */
void Set_All_Forward_Params(u16 kp, u16 ki, u16 kd, u16 threshold)
{
    car_state.pid.forward_kp = kp;
    car_state.pid.forward_ki = ki;
    car_state.pid.forward_kd = kd;
    car_state.pid.forward_error_threshold = threshold;
    printf("设置所有直线行走角度修正参数:\r\n");
    printf("Kp = %.3f, Ki = %.3f, Kd = %.3f\r\n", (float)kp / 100.0f, (float)ki / 100.0f, (float)kd / 100.0f);
    printf("误差阈值 = %.3f度\r\n", (float)threshold / 100.0f);
}

/**
 * @brief 获取直线行走基础PWM值
 * @return 当前基础PWM值
 */
u16 Get_forwardBase_PWM(void)
{
    printf("当前直线行走基础PWM值: PWM = %u\r\n", car_state.running.forward_base_pwm);
    return car_state.running.forward_base_pwm;
}

/**
 * @brief 获取直线行走角度修正比例系数
 * @return 当前比例系数
 */
float Get_Forward_Kp(void)
{
    float kp = (float)car_state.pid.forward_kp / 100.0f;
    printf("当前直线行走角度修正比例系数: Kp = %.3f\r\n", kp);
    return kp;
}

/**
 * @brief 获取直线行走角度修正积分系数
 * @return 当前积分系数
 */
float Get_Forward_Ki(void)
{
    float ki = (float)car_state.pid.forward_ki / 100.0f;
    printf("当前直线行走角度修正积分系数: Ki = %.3f\r\n", ki);
    return ki;
}

/**
 * @brief 获取直线行走角度修正微分系数
 * @return 当前微分系数
 */
float Get_Forward_Kd(void)
{
    float kd = (float)car_state.pid.forward_kd / 100.0f;
    printf("当前直线行走角度修正微分系数: Kd = %.3f\r\n", kd);
    return kd;
}

/**
 * @brief 获取直线行走角度修正误差阈值
 * @return 当前误差阈值(度)
 */
float Get_Forward_Error_Threshold(void)
{
    float threshold = (float)car_state.pid.forward_error_threshold / 100.0f;
    printf("当前直线行走角度修正误差阈值: %.3f度\r\n", threshold);
    return threshold;
}

/**
 * @brief 设置任务3中从A点对准C点的旋转角度
 * @param angle 旋转角度(度，顺时针为正)
 * @return 无
 */
void Set_Task3_Rotation_Angle_1(int angle)
{
    Task3_Rotation_Angle_1 = (float)angle;
    printf("设置任务3初始旋转角度: %.2f度\r\n", (float)angle);
}

/**
 * @brief 设置任务3中B到D前的旋转角度
 * @param angle 旋转角度(度，逆时针为正)
 * @return 无
 */
void Set_Task3_Rotation_Angle_2(int angle)
{
    Task3_Rotation_Angle_2 = (float)angle;
    printf("设置任务3 B到D前旋转角度: %.2f度\r\n", (float)angle);
}

/**
 * @brief 设置任务3中C点开环转向时间
 * @param time 转向时间(中断次数)
 * @return 无
 */
void Set_Task3_Steer_Time_C(int time)
{
    Task3_Steer_Time_C = time;
    printf("设置任务3 C点转向时间: %d\r\n", time);
}

/**
 * @brief 设置任务3中C点开环转向PWM值
 * @param pwm PWM值(速度值)
 * @return 无
 */
void Set_Task3_Steer_PWM_C(int pwm)
{
    Task3_Steer_PWM_C = pwm;
    printf("设置任务3 C点转向PWM: %d\r\n", pwm);
}

/**
 * @brief 设置任务3中D点开环转向时间
 * @param time 转向时间(中断次数，负值表示顺时针旋转)
 * @return 无
 */
void Set_Task3_Steer_Time_D(int time)
{
    Task3_Steer_Time_D = time;
    printf("设置任务3 D点转向时间: %d\r\n", time);
}

/**
 * @brief 设置任务3中D点开环转向PWM值
 * @param pwm PWM值(速度值)
 * @return 无
 */
void Set_Task3_Steer_PWM_D(int pwm)
{
    Task3_Steer_PWM_D = pwm;
    printf("设置任务3 D点转向PWM: %d\r\n", pwm);
}

/**
 * @brief 设置任务3中直线行驶速度
 * @param speed 速度值
 * @return 无
 */
void Set_Task3_Move_Forward_Speed(int speed)
{
    Task3_Move_Forward_Speed = speed;
    printf("设置任务3直线行驶速度: %d\r\n", speed);
}

/**
 * @brief 获取任务3中从A点对准C点的旋转角度
 * @return 当前角度值(度)
 */
float Get_Task3_Rotation_Angle_1(void)
{
    printf("当前任务3初始旋转角度: %.2f度\r\n", Task3_Rotation_Angle_1);
    return Task3_Rotation_Angle_1;
}

/**
 * @brief 获取任务3中B到D前的旋转角度
 * @return 当前角度值(度)
 */
float Get_Task3_Rotation_Angle_2(void)
{
    printf("当前任务3 B到D前旋转角度: %.2f度\r\n", Task3_Rotation_Angle_2);
    return Task3_Rotation_Angle_2;
}

/**
 * @brief 获取任务3中C点开环转向时间
 * @return 当前转向时间(中断次数)
 */
int Get_Task3_Steer_Time_C(void)
{
    printf("当前任务3 C点转向时间: %d\r\n", Task3_Steer_Time_C);
    return Task3_Steer_Time_C;
}

/**
 * @brief 获取任务3中C点开环转向PWM值
 * @return 当前PWM值(速度值)
 */
int Get_Task3_Steer_PWM_C(void)
{
    printf("当前任务3 C点转向PWM: %d\r\n", Task3_Steer_PWM_C);
    return Task3_Steer_PWM_C;
}

/**
 * @brief 获取任务3中D点开环转向时间
 * @return 当前转向时间(中断次数)
 */
int Get_Task3_Steer_Time_D(void)
{
    printf("当前任务3 D点转向时间: %d\r\n", Task3_Steer_Time_D);
    return Task3_Steer_Time_D;
}

/**
 * @brief 获取任务3中D点开环转向PWM值
 * @return 当前PWM值(速度值)
 */
int Get_Task3_Steer_PWM_D(void)
{
    printf("当前任务3 D点转向PWM: %d\r\n", Task3_Steer_PWM_D);
    return Task3_Steer_PWM_D;
}

/**
 * @brief 获取任务3中直线行驶速度
 * @return 当前速度值
 */
int Get_Task3_Move_Forward_Speed(void)
{
    printf("当前任务3直线行驶速度: %d\r\n", Task3_Move_Forward_Speed);
    return Task3_Move_Forward_Speed;
}

/**
 * @brief 重置任务
 * @return none
 *
 */
void resetTask(void)
{
    car_state.running.reset_task_flag = 1;
    car_state.running.reset_mode_start_time = HAL_GetTick(); // 记录重置开始时间
    led1_on();
    led2_on();
    printf("即将在%dms后重置任务", RESET_WAIT_TIME);
}

/**
 * @brief 设置任务4初始旋转角度
 * @param angle 旋转角度(度，顺时针为正)
 * @return 无
 */
void Set_Task4_Rotation_Angle_1(int angle)
{
    Task4_Rotation_Angle_1 = (float)angle;
    printf("设置任务4初始旋转角度: %.2f度\r\n", (float)angle);
}

/**
 * @brief 设置任务4从B到D前的旋转角度
 * @param angle 旋转角度(度，逆时针为正)
 * @return 无
 */
void Set_Task4_Rotation_Angle_2(int angle)
{
    Task4_Rotation_Angle_2 = (float)angle;
    printf("设置任务4 B到D前旋转角度: %.2f度\r\n", (float)angle);
}

/**
 * @brief 设置任务4后3圈从B到D前的旋转角度
 * @param angle 旋转角度(度，逆时针为正)
 * @return 无
 */
void Set_Task4_Rotation_Angle_3(int angle)
{
    Task4_Rotation_Angle_3 = (float)angle;
    printf("设置任务4后3圈B到D前旋转角度: %.2f度\r\n", (float)angle);
}

/**
 * @brief 设置任务4中C点开环转向时间
 * @param time 转向时间(中断次数)
 * @return 无
 */
void Set_Task4_Steer_Time_C(int time)
{
    Task4_Steer_Time_C = time;
    printf("设置任务4 C点转向时间: %d\r\n", time);
}

/**
 * @brief 设置任务4中C点开环转向PWM值
 * @param pwm PWM值(速度值)
 * @return 无
 */
void Set_Task4_Steer_PWM_C(int pwm)
{
    Task4_Steer_PWM_C = pwm;
    printf("设置任务4 C点转向PWM: %d\r\n", pwm);
}

/**
 * @brief 设置任务4中D点开环转向时间
 * @param time 转向时间(中断次数，负值表示顺时针旋转)
 * @return 无
 */
void Set_Task4_Steer_Time_D(int time)
{
    Task4_Steer_Time_D = time;
    printf("设置任务4 D点转向时间: %d\r\n", time);
}

/**
 * @brief 设置任务4中D点开环转向PWM值
 * @param pwm PWM值(速度值)
 * @return 无
 */
void Set_Task4_Steer_PWM_D(int pwm)
{
    Task4_Steer_PWM_D = pwm;
    printf("设置任务4 D点转向PWM: %d\r\n", pwm);
}

/**
 * @brief 设置任务4循环执行次数
 * @param count 循环次数
 * @return 无
 */
void Set_Task4_Cycle_Count(int count)
{
    Task4_Cycle_Count = count;
    printf("设置任务4循环执行次数: %d\r\n", count);
}

/**
 * @brief 设置任务4间隔时间
 * @param interval 间隔时间(毫秒)
 * @return 无
 */
void Set_Task4_Interval(int interval)
{
    Task4_Interval = interval;
    printf("设置任务4间隔时间: %dms\r\n", interval);
}

/**
 * @brief 设置任务4中直线行驶速度
 * @param speed 速度值
 * @return 无
 */
void Set_Task4_Move_Forward_Speed(int speed)
{
    Task4_Move_Forward_Speed = speed;
    printf("设置任务4直线行驶速度: %d\r\n", speed);
}

/**
 * @brief 设置任务4中循迹行驶速度
 * @param speed 速度值
 * @return 无
 */
void Set_Task4_Line_Tracking_Speed(int speed)
{
    Task4_Line_Tracking_Speed = speed;
    printf("设置任务4循迹行驶速度: %d\r\n", speed);
}

/**
 * @brief 设置任务4中B点延时时间
 * @param time 延时时间(毫秒)
 * @return 无
 */
void Set_Task4_B_Point_Delay_Time(int time)
{
    Task4_B_Point_Delay_Time = time;
    printf("设置任务4 B点延时时间: %dms\r\n", time);
}

/**
 * @brief 获取任务4中初始旋转角度
 * @return 当前角度值(度)
 */
float Get_Task4_Rotation_Angle_1(void)
{
    printf("当前任务4初始旋转角度: %.2f度\r\n", Task4_Rotation_Angle_1);
    return Task4_Rotation_Angle_1;
}

/**
 * @brief 获取任务4中B到D前的旋转角度
 * @return 当前角度值(度)
 */
float Get_Task4_Rotation_Angle_2(void)
{
    printf("当前任务4 B到D前旋转角度: %.2f度\r\n", Task4_Rotation_Angle_2);
    return Task4_Rotation_Angle_2;
}

/**
 * @brief 获取任务4中后3圈从B到D前的旋转角度
 * @return 当前角度值(度)
 */
float Get_Task4_Rotation_Angle_3(void)
{
    printf("当前任务4后3圈B到D前旋转角度: %.2f度\r\n", Task4_Rotation_Angle_3);
    return Task4_Rotation_Angle_3;
}

/**
 * @brief 获取任务4中C点开环转向时间
 * @return 当前转向时间(中断次数)
 */
int Get_Task4_Steer_Time_C(void)
{
    printf("当前任务4 C点转向时间: %d\r\n", Task4_Steer_Time_C);
    return Task4_Steer_Time_C;
}

/**
 * @brief 获取任务4中C点开环转向PWM值
 * @return 当前PWM值(速度值)
 */
int Get_Task4_Steer_PWM_C(void)
{
    printf("当前任务4 C点转向PWM: %d\r\n", Task4_Steer_PWM_C);
    return Task4_Steer_PWM_C;
}

/**
 * @brief 获取任务4中D点开环转向时间
 * @return 当前转向时间(中断次数)
 */
int Get_Task4_Steer_Time_D(void)
{
    printf("当前任务4 D点转向时间: %d\r\n", Task4_Steer_Time_D);
    return Task4_Steer_Time_D;
}

/**
 * @brief 获取任务4中D点开环转向PWM值
 * @return 当前PWM值(速度值)
 */
int Get_Task4_Steer_PWM_D(void)
{
    printf("当前任务4 D点转向PWM: %d\r\n", Task4_Steer_PWM_D);
    return Task4_Steer_PWM_D;
}

/**
 * @brief 获取任务4循环执行次数
 * @return 当前循环次数
 */
int Get_Task4_Cycle_Count(void)
{
    printf("当前任务4循环执行次数: %d\r\n", Task4_Cycle_Count);
    return Task4_Cycle_Count;
}

/**
 * @brief 获取任务4间隔时间
 * @return 当前间隔时间(毫秒)
 */
int Get_Task4_Interval(void)
{
    printf("当前任务4间隔时间: %dms\r\n", Task4_Interval);
    return Task4_Interval;
}

/**
 * @brief 获取任务4中直线行驶速度
 * @return 当前速度值
 */
int Get_Task4_Move_Forward_Speed(void)
{
    printf("当前任务4直线行驶速度: %d\r\n", Task4_Move_Forward_Speed);
    return Task4_Move_Forward_Speed;
}

/**
 * @brief 获取任务4中循迹行驶速度
 * @return 当前速度值
 */
int Get_Task4_Line_Tracking_Speed(void)
{
    printf("当前任务4循迹行驶速度: %d\r\n", Task4_Line_Tracking_Speed);
    return Task4_Line_Tracking_Speed;
}

/**
 * @brief 获取任务4中B点延时时间
 * @return 当前延时时间(毫秒)
 */
int Get_Task4_B_Point_Delay_Time(void)
{
    printf("当前任务4 B点延时时间: %dms\r\n", Task4_B_Point_Delay_Time);
    return Task4_B_Point_Delay_Time;
}

/**
 * @brief 设置任务3或任务4的所有转向角度参数
 * @param task 任务编号(3或4)
 * @param angle1 任务初始旋转角度(度)
 * @param angle2 任务B到D前旋转角度(度)
 * @param angle3 任务后3圈B到D前旋转角度(度)(如果任务3则传入0)
 * @return 无返回值
 */
void Set_Task_SteeringAngle_Params(int task, int angle1, int angle2, int angle3)
{
    if (task == 3)
    {
        Set_Task3_Rotation_Angle_1(angle1);
        Set_Task3_Rotation_Angle_2(angle2);
        printf("设置任务3参数: 旋转角度1 = %.2f, 旋转角度2 = %.2f\r\n",
               Task3_Rotation_Angle_1, Task3_Rotation_Angle_2);
    }
    else if (task == 4)
    {
        Set_Task4_Rotation_Angle_1(angle1);
        Set_Task4_Rotation_Angle_2(angle2);
        Set_Task4_Rotation_Angle_3(angle3);
        printf("设置任务4参数: 旋转角度1 = %.2f, 旋转角度2 = %.2f, 旋转角度3 = %.2f\r\n",
               Task4_Rotation_Angle_1, Task4_Rotation_Angle_2, Task4_Rotation_Angle_3);
    }
    else
    {
        printf("无效的任务编号: %d\r\n", task);
    }
}

/**
 * @brief 获取任务3或任务4的所有转向角度参数
 * @param task 任务编号(3或4)
 * @return 无返回值
 */
void Get_Task_SteeringAngle_Params(int task)
{
    if (task == 3)
    {
        printf("任务3参数: 旋转角度1 = %.2f, 旋转角度2 = %.2f\r\n",
               Task3_Rotation_Angle_1, Task3_Rotation_Angle_2);
    }
    else if (task == 4)
    {
        printf("任务4参数: 旋转角度1 = %.2f, 旋转角度2 = %.2f, 旋转角度3 = %.2f\r\n",
               Task4_Rotation_Angle_1, Task4_Rotation_Angle_2, Task4_Rotation_Angle_3);
    }
    else
    {
        printf("无效的任务编号: %d\r\n", task);
    }
}
/**
 * @brief 设置任务3或任务4的所有开环转向参数
 * @param task 任务编号(3或4)
 * @param steer_time_c C点转向时间(中断次数)
 * @param steer_pwm_c C点转向PWM值(速度值)
 * @param steer_time_d D点转向时间(中断次数，负值表示顺时针旋转)
 * @param steer_pwm_d D点转向PWM值(速度值)
 * @return 无返回值
 */
void Set_Task_OpenLoopSteering_Params(int task, int steer_time_c, int steer_pwm_c, int steer_time_d, int steer_pwm_d)
{
    if (task == 3)
    {
        Set_Task3_Steer_Time_C(steer_time_c);
        Set_Task3_Steer_PWM_C(steer_pwm_c);
        Set_Task3_Steer_Time_D(steer_time_d);
        Set_Task3_Steer_PWM_D(steer_pwm_d);
        printf("设置任务3参数: C点转向时间 = %d, C点转向PWM = %d, D点转向时间 = %d, D点转向PWM = %d\r\n",
               Task3_Steer_Time_C, Task3_Steer_PWM_C, Task3_Steer_Time_D, Task3_Steer_PWM_D);
    }
    else if (task == 4)
    {
        Set_Task4_Steer_Time_C(steer_time_c);
        Set_Task4_Steer_PWM_C(steer_pwm_c);
        Set_Task4_Steer_Time_D(steer_time_d);
        Set_Task4_Steer_PWM_D(steer_pwm_d);
        printf("设置任务4参数: C点转向时间 = %d, C点转向PWM = %d, D点转向时间 = %d, D点转向PWM = %d\r\n",
               Task4_Steer_Time_C, Task4_Steer_PWM_C, Task4_Steer_Time_D, Task4_Steer_PWM_D);
    }
    else
    {
        printf("无效的任务编号: %d\r\n", task);
    }
}
/**
 * @brief 获取任务3或任务4的所有开环转向参数
 * @param task 任务编号(3或4)
 * @return 无返回值
 */
void Get_Task_OpenLoopSteering_Params(int task)
{
    if (task == 3)
    {
        printf("任务3参数: C点转向时间 = %d, C点转向PWM = %d, D点转向时间 = %d, D点转向PWM = %d\r\n",
               Task3_Steer_Time_C, Task3_Steer_PWM_C, Task3_Steer_Time_D, Task3_Steer_PWM_D);
    }
    else if (task == 4)
    {
        printf("任务4参数: C点转向时间 = %d, C点转向PWM = %d, D点转向时间 = %d, D点转向PWM = %d\r\n",
               Task4_Steer_Time_C, Task4_Steer_PWM_C, Task4_Steer_Time_D, Task4_Steer_PWM_D);
    }
    else
    {
        printf("无效的任务编号: %d\r\n", task);
    }
}
