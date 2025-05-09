/**
 * @file car_state.c
 * @author 肖弋洋
 * @brief 汽车状态管理,这个文件主要用来储存封装后的全局变量
 * 使代码的结构更加清晰
 * @version 0.1
 * @date 2025-05-09
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "car_state.h"
#include "car_config.h"

// 全局变量定义
CarState_t car_state;

/**
 * @brief 初始化汽车状态
 *
 * 此函数用于初始化汽车状态结构体中的所有变量为默认值
 */
void CarState_Init(void)
{
  // 初始化传感器状态
  car_state.sensors.left = 0;
  car_state.sensors.middle_left = 0;
  car_state.sensors.middle = 0;
  car_state.sensors.middle_right = 0;
  car_state.sensors.right = 0;
  car_state.sensors.distance = 0;
  car_state.sensors.acceleration_z = 0.0f;

  // 初始化PID参数
  car_state.pid.balance_kp = BALANCE_KP_DEFAULT;
  car_state.pid.balance_kd = BALANCE_KD_DEFAULT;
  car_state.pid.velocity_kp = VELOCITY_KP_DEFAULT;
  car_state.pid.velocity_ki = VELOCITY_KI_DEFAULT;
  car_state.pid.turn_kp = TURN_KP_DEFAULT;
  car_state.pid.turn_kd = TURN_KD_DEFAULT;
  car_state.pid.sensor_kp = SENSOR_KP_DEFAULT;
  car_state.pid.sensor_ki = SENSOR_KI_DEFAULT;
  car_state.pid.sensor_kd = SENSOR_KD_DEFAULT;
  car_state.pid.steering_kp = STEERING_KP_DEFAULT;
  car_state.pid.steering_ki = STEERING_KI_DEFAULT;
  car_state.pid.steering_kd = STEERING_KD_DEFAULT;
  car_state.pid.steering_error_threshold = STEERING_ERROR_THRESHOLD_DEFAULT;
  car_state.pid.steering_speed = STEERING_SPEED_DEFAULT;
  car_state.pid.forward_kp = FORWARD_KP_DEFAULT;
  car_state.pid.forward_ki = FORWARD_KI_DEFAULT;
  car_state.pid.forward_kd = FORWARD_KD_DEFAULT;
  car_state.pid.forward_error_threshold = FORWARD_ERROR_THRESHOLD;

  // 初始化电机状态
  car_state.motor.left_pwm = 0;
  car_state.motor.right_pwm = 0;
  car_state.motor.left_velocity = 0.0f;
  car_state.motor.right_velocity = 0.0f;
  car_state.motor.velocity_pwm = 0;
  car_state.motor.encoder_left = 0;
  car_state.motor.encoder_right = 0;
  car_state.motor.balance_pwm = 0;
  car_state.motor.turn_pwm = 0;

  // 初始化姿态数据
  car_state.attitude.angle_balance = 0.0f;
  car_state.attitude.gyro_balance = 0.0f;
  car_state.attitude.gyro_turn = 0.0f;
  car_state.attitude.way_angle = WAY_ANGLE_DEFAULT;
  car_state.attitude.initial_angle = 0.0f;
  for (int i = 0; i < 5; i++)
  {
    car_state.attitude.initial_angle_temp[i] = 0.0f;
  }

  // 初始化运行状态
  car_state.running.flag_stop = FLAG_STOP_DEFAULT;
  car_state.running.flag_show = FLAG_SHOW_DEFAULT;
  car_state.running.target_velocity = TARGET_VELOCITY_DEFAULT;
  car_state.running.mode = MODE_DEFAULT;
  car_state.running.reset_task_flag = 0;
  car_state.running.reset_mode_start_time = 0;
  car_state.running.forward_base_pwm = FORWARDBASE_PWM;
  car_state.running.openloop_steering_base_pwm = OPENLOOP_STEERING_BASE_PWM;
  car_state.running.steering_completed = 0;
  car_state.running.steering_stable_count = 0;
  car_state.running.manual_mode = 0;
  car_state.running.beep_on_flag = 0;
  car_state.running.beep_start_time = 0;

  // 初始化遥控相关
  car_state.remote.flag_front = 0;
  car_state.remote.flag_back = 0;
  car_state.remote.flag_left = 0;
  car_state.remote.flag_right = 0;
  car_state.remote.flag_velocity = 2;
  car_state.remote.move_x = 0.0f;
  car_state.remote.move_z = 0.0f;

  // 初始化其他数据
  car_state.misc.temperature = 0;
  car_state.misc.voltage = 0;
  car_state.misc.middle_angle = 0;
  car_state.misc.ccd_zhongzhi = 0;
  car_state.misc.ccd_yuzhi = 0;
  for (int i = 0; i < 128; i++)
  {
    car_state.misc.adv[i] = 0;
  }
  car_state.misc.determine = 0;
  car_state.misc.key_state = 0;
  car_state.misc.key_state_last = 0;
  car_state.misc.ld_successful_receive_flag = 0;
}
