#ifndef __SETPARMA_H
#define __SETPARMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// 参数设置函数声明
void Set_openLoopSteeringBase_PWM(u16 pwm);
void Set_Steering_Kp(u16 kp);
void Set_Steering_Ki(u16 ki);
void Set_Steering_Kd(u16 kd);
void Set_Steering_Error_Threshold(u16 threshold);
void Set_All_Steering_Params(u16 kp, u16 ki, u16 kd, u16 threshold);
u16 Get_openLoopSteeringBase_PWM(void);
float Get_Steering_Kp(void);
float Get_Steering_Ki(void);
float Get_Steering_Kd(void);
float Get_Steering_Error_Threshold(void);

// 直线行走角度修正参数设置函数
void Set_forwardBase_PWM(u16 pwm);
void Set_Forward_Kp(u16 kp);
void Set_Forward_Ki(u16 ki);
void Set_Forward_Kd(u16 kd);
void Set_Forward_Error_Threshold(u16 threshold);
void Set_All_Forward_Params(u16 kp, u16 ki, u16 kd, u16 threshold);
u16 Get_forwardBase_PWM(void);
float Get_Forward_Kp(void);
float Get_Forward_Ki(void);
float Get_Forward_Kd(void);
float Get_Forward_Error_Threshold(void);

// Task3相关参数设置函数声明
void Set_Task3_Rotation_Angle_1(int angle);
void Set_Task3_Rotation_Angle_2(int angle);
void Set_Task3_Rotation_Angle_3(int angle);
void Set_Task3_Steer_Time_C(int time);
void Set_Task3_Steer_PWM_C(int pwm);
void Set_Task3_Steer_Time_D(int time);
void Set_Task3_Steer_PWM_D(int pwm);
void Set_Task3_Move_Forward_Speed(int speed);
float Get_Task3_Rotation_Angle_1(void);
float Get_Task3_Rotation_Angle_2(void);
float Get_Task3_Rotation_Angle_3(void);
int Get_Task3_Steer_Time_C(void);
int Get_Task3_Steer_PWM_C(void);
int Get_Task3_Steer_Time_D(void);
int Get_Task3_Steer_PWM_D(void);
int Get_Task3_Move_Forward_Speed(void);
void resetTask(void);

#ifdef __cplusplus
}
#endif

#endif /* __SETPARMA_H */
