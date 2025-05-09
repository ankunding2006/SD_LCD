/**
 * @file car_state.h
 * @author 肖弋洋
 * @brief 汽车状态管理,这个文件主要用来储存封装后的全局变量
 * 使代码的结构更加清晰
 * @version 0.1
 * @date 2025-05-09
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __CAR_STATE_H
#define __CAR_STATE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

  /**
   * @brief 传感器状态结构体
   */
  typedef struct
  {
    u8 left;              // 左侧传感器状态
    u8 middle_left;       // 中左传感器状态
    u8 middle;            // 中间传感器状态
    u8 middle_right;      // 中右传感器状态
    u8 right;             // 右侧传感器状态
    u32 distance;         // 超声波测距
    float acceleration_z; // Z轴加速度值
  } SensorState_t;

  /**
   * @brief PID参数结构体
   */
  typedef struct
  {
    float balance_kp;             // 平衡比例系数
    float balance_kd;             // 平衡微分系数
    float velocity_kp;            // 速度比例系数
    float velocity_ki;            // 速度积分系数
    float turn_kp;                // 转向比例系数
    float turn_kd;                // 转向微分系数
    float sensor_kp;              // 传感器比例系数
    float sensor_ki;              // 传感器积分系数
    float sensor_kd;              // 传感器微分系数
    u16 steering_kp;              // 转向控制比例系数（放大100倍）
    u16 steering_ki;              // 转向控制积分系数（放大100倍）
    u16 steering_kd;              // 转向控制微分系数（放大100倍）
    u16 steering_error_threshold; // 转向控制误差阈值（放大100倍）
    u16 steering_speed;           // 转向控制基础速度（放大100倍）
    u16 forward_kp;               // 直线行走角度修正比例系数
    u16 forward_ki;               // 直线行走角度修正积分系数
    u16 forward_kd;               // 直线行走角度修正微分系数
    u16 forward_error_threshold;  // 直线行走角度修正误差阈值
  } PID_Parameters_t;

  /**
   * @brief 电机状态结构体
   */
  typedef struct
  {
    int left_pwm;               // 左轮PWM值
    int right_pwm;              // 右轮PWM值
    float left_velocity;        // 左轮速度
    float right_velocity;       // 右轮速度
    volatile int encoder_left;  // 左编码器的脉冲计数
    volatile int encoder_right; // 右编码器的脉冲计数
    volatile int balance_pwm;   // 平衡PWM
    volatile int velocity_pwm;  // 速度PWM
    volatile int turn_pwm;      // 转向PWM
  } MotorState_t;

  /**
   * @brief 姿态数据结构体
   */
  typedef struct
  {
    float angle_balance;         // 平衡倾角
    float gyro_balance;          // 平衡陀螺仪
    float gyro_turn;             // 转向陀螺仪
    u8 way_angle;                // 获取角度的算法：1-四元数 2-卡尔曼 3-互补滤波
    float initial_angle;         // 初始航向角
    float initial_angle_temp[5]; // 记录初始角度的数组
  } AttitudeData_t;

  /**
   * @brief 运行状态结构体
   */
  typedef struct
  {
    u8 flag_stop;                   // 电机停止标志位
    u8 flag_show;                   // 显示标志位
    float target_velocity;          // 目标速度
    u8 mode;                        // 模式选择
    u8 reset_task_flag;             // 是否要重启任务标志位
    u32 reset_mode_start_time;      // 任务3重置模式开始时间
    u16 forward_base_pwm;           // 直线行走基础PWM值
    u16 openloop_steering_base_pwm; // 转向基础PWM值
    u8 steering_completed;          // 转向完成标志
    u16 steering_stable_count;      // 转向稳定计数
    u8 manual_mode;                 // 手动设置角度标志位
    u8 beep_on_flag;                // 蜂鸣器标志位
    u32 beep_start_time;            // 蜂鸣器开启时间
  } RunningState_t;

  /**
   * @brief 遥控相关结构体
   */
  typedef struct
  {
    u8 flag_front;    // 前进标志
    u8 flag_back;     // 后退标志
    u8 flag_left;     // 左转标志
    u8 flag_right;    // 右转标志
    u8 flag_velocity; // 速度标志
    float move_x;     // X轴移动量
    float move_z;     // Z轴移动量
  } RemoteControl_t;

  /**
   * @brief 其他数据结构体
   */
  typedef struct
  {
    int temperature;               // 温度变量
    int voltage;                   // 电池电压
    int middle_angle;              // 中值角度
    u8 ccd_zhongzhi;               // CCD中值
    u8 ccd_yuzhi;                  // CCD阈值
    u16 adv[128];                  // 存放CCD的数据的数组
    u16 determine;                 // 雷达跟随模式的一个标志位
    u8 key_state;                  // 按键状态
    u8 key_state_last;             // 上次按键状态
    u8 ld_successful_receive_flag; // 雷达成功接收数据标志位
  } MiscData_t;

  /**
   * @brief 汽车总状态结构体
   */
  typedef struct
  {
    SensorState_t sensors;   // 传感器状态
    PID_Parameters_t pid;    // PID参数
    MotorState_t motor;      // 电机状态
    AttitudeData_t attitude; // 姿态数据
    RunningState_t running;  // 运行状态
    RemoteControl_t remote;  // 遥控相关
    MiscData_t misc;         // 其他数据
  } CarState_t;

  // 全局变量声明
  extern CarState_t car_state;

  // 函数声明
  void CarState_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __CAR_STATE_H */
