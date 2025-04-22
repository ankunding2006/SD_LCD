#include "setParma.h"
#include <stdio.h>

/**
 * @brief 设置转向控制比例系数
 * @param kp 新的比例系数(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Kp(u16 kp)
{
    Steering_Kp = kp;
    printf("设置转向控制比例系数: Kp = %.3f\r\n", (float)kp/100.0f);
}

/**
 * @brief 设置转向控制积分系数
 * @param ki 新的积分系数(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Ki(u16 ki)
{
    Steering_Ki = ki;
    printf("设置转向控制积分系数: Ki = %.3f\r\n", (float)ki/100.0f);
}

/**
 * @brief 设置转向控制微分系数
 * @param kd 新的微分系数(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Kd(u16 kd)
{
    Steering_Kd = kd;
    printf("设置转向控制微分系数: Kd = %.3f\r\n", (float)kd/100.0f);
}

/**
 * @brief 设置转向控制误差阈值
 * @param threshold 新的误差阈值(度)(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Error_Threshold(u16 threshold)
{
    Steering_Error_Threshold = threshold;
    printf("设置转向控制误差阈值: %.3f度\r\n", (float)threshold/100.0f);
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
    Steering_Kp = kp;
    Steering_Ki = ki;
    Steering_Kd = kd;
    Steering_Error_Threshold = threshold;
    printf("设置所有转向控制参数:\r\n");
    printf("Kp = %.3f, Ki = %.3f, Kd = %.3f\r\n", (float)kp/100.0f, (float)ki/100.0f, (float)kd/100.0f);
    printf("误差阈值 = %.3f度\r\n", (float)threshold/100.0f);
}

/**
 * @brief 获取转向控制比例系数
 * @return 当前比例系数
 */
float Get_Steering_Kp(void)
{
    float kp = (float)Steering_Kp/100.0f;
    printf("当前转向控制比例系数: Kp = %.3f\r\n", kp);
    return kp;
}

/**
 * @brief 获取转向控制积分系数
 * @return 当前积分系数
 */
float Get_Steering_Ki(void)
{
    float ki = (float)Steering_Ki/100.0f;
    printf("当前转向控制积分系数: Ki = %.3f\r\n", ki);
    return ki;
}

/**
 * @brief 获取转向控制微分系数
 * @return 当前微分系数
 */
float Get_Steering_Kd(void)
{
    float kd = (float)Steering_Kd/100.0f;
    printf("当前转向控制微分系数: Kd = %.3f\r\n", kd);
    return kd;
}

/**
 * @brief 获取转向控制误差阈值
 * @return 当前误差阈值(度)
 */
float Get_Steering_Error_Threshold(void)
{
    float threshold = (float)Steering_Error_Threshold/100.0f;
    printf("当前转向控制误差阈值: %.3f度\r\n", threshold);
    return threshold;
}
