#include "control.h"
#include "Test.h"
#include "Task.h"
#include "setParma.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "main.h"

/**
 * @brief 这个函数用来控制小车进行循迹
 * @note 这个函数是小车循迹的主函数，主要是通过灰度传感器来控制小车的行驶方向
 * 这个函数主要来控制小车走过AD,BC这两条曲线
 * @return 如果完成了循迹(即所有传感器都返回白色)就返回1,否则返回0
 */
int lineTracking_Handler(void)
{
  static u8 init = 0;           // 初始化标志
  static u32 startTime = 0;     // 开始时间
  static u16 prev_turn_pwm = 0; // 上一次的转向PWM值
  if (init == 0)                // 如果没有初始化
  {
    init = 1;                  // 设置初始化标志
    startTime = HAL_GetTick(); // 记录开始时间
    return 0;                  // 返回0，表示没有完成循迹,正在循迹中
  }
  car_state.motor.encoder_left = Read_Encoder(3);                                              // 读取左轮编码器的值，前进为正，后退为负
  car_state.motor.encoder_right = Read_Encoder(5);                                             // 修改为TIM5，前进为正，后退为负
                                                                                               // 左轮A相接TIM2_CH1,右轮A相接TIM4_CH2,故这里两个编码器的脉冲极性相同
  Get_Velocity_Form_Encoder(car_state.motor.encoder_left, car_state.motor.encoder_right);      // 编码器读数转速度（mm/s）
  int actual_velocity = Velocity(car_state.motor.encoder_left, car_state.motor.encoder_right); // 获取速度控制的PWM,速度控制

  grey_sensor_Read();           // 读取灰度传感器数据
#if NOLINEDETECT == 1           // 调试模式，不使用灰度传感器
  car_state.motor.turn_pwm = 0; // 调试模式，不使用灰度传感器
#else
  car_state.motor.turn_pwm = Calculate_Turn_Pwm(); // 计算转向PWM值,如果所有传感器都返回白色，返回1

  if (car_state.motor.turn_pwm == INT16_MIN)
  {
    if (HAL_GetTick() - startTime <= LINE_TRACKING_DEAD_TIME)
    {
      car_state.motor.left_pwm = actual_velocity - prev_turn_pwm;
      car_state.motor.right_pwm = actual_velocity + prev_turn_pwm;
      if (car_state.running.flag_stop == 1)
        Set_Pwm(0, 0);
      else
        Set_Pwm(car_state.motor.left_pwm, car_state.motor.right_pwm); // 赋值给PWM寄存器
      return 0;                                                       // 返回0，表示没有完成循迹,正在循迹中
    }
    else
    {
      Set_Pwm(0, 0); // 如果没有传感器检测到黑线，停止电机
      init = 0;      // 重置初始化标志
      startTime = 0; // 重置开始时间
      return 1;      // 返回1，表示完成循迹
    }
  }
#endif

  // 使用计算出的实际速度，不修改Target_Velocity
  car_state.motor.left_pwm = actual_velocity - car_state.motor.turn_pwm;
  car_state.motor.right_pwm = actual_velocity + car_state.motor.turn_pwm;
  if (car_state.running.flag_stop == 1)
    Set_Pwm(0, 0);
  else
    Set_Pwm(car_state.motor.left_pwm, car_state.motor.right_pwm); // 赋值给PWM寄存器
  prev_turn_pwm = car_state.motor.turn_pwm;                       // 更新上一次的转向PWM值

  return 0; // 返回0，表示没有完成循迹,正在循迹中
}

/**
 * @brief Open loop steering control function
 *
 * @param angle 小车旋转的时间,逆时针旋转为正,顺时针旋转为负
 * @return u8 1:完成 0:小车正在转向中
 */
u8 openLoopSteering_Handler(int SteerTime, int PWM_Value)
{
  static u8 isInitialized = 0; // 初始化标志
  static u32 startTime = 0;    // 开始时间

  if (isInitialized == 0)
  {
    startTime = HAL_GetTick(); // 记录开始时间
    isInitialized = 1;         // 设置初始化标志
    return 0;                  // 返回未完成
  }

  if (HAL_GetTick() - startTime >= abs(SteerTime))
  {
    Set_Pwm(0, 0);     // 停止电机
    isInitialized = 0; // 重置初始化标志
    return 1;          // 返回完成
  }
  else
  {
    if (SteerTime > 0)
    {
      // 逆时针转向
      car_state.motor.left_pwm = car_state.running.openloop_steering_base_pwm - PWM_Value;  // 左轮反转
      car_state.motor.right_pwm = car_state.running.openloop_steering_base_pwm + PWM_Value; // 右轮正转
    }
    else
    {
      // 顺时针转向
      car_state.motor.left_pwm = car_state.running.openloop_steering_base_pwm + PWM_Value;  // 左轮正转
      car_state.motor.right_pwm = car_state.running.openloop_steering_base_pwm - PWM_Value; // 右轮反转
    }
    if (car_state.running.flag_stop == 1) // 如果停止标志为1，停止电机
      Set_Pwm(0, 0);                      // 停止电机
    else
      Set_Pwm(car_state.motor.left_pwm, car_state.motor.right_pwm); // 赋值给PWM寄存器
    return 0;                                                       // 返回未完成
  }
}

/**
 * @brief Open loop steering control function
 *
 * @param angle 小车旋转的时间,逆时针旋转为正,顺时针旋转为负
 * @param PWM_Value 转向PWM值
 * @param PWM_Base 基础PWM值
 * @return u8 1:完成 0:小车正在转向中
 */
u8 openLoopSteeringWithBase_PWM_Handler(int SteerTime, int PWM_Value, int openloop_steering_base_pwm)
{
  static u8 isInitialized = 0; // 初始化标志
  static u32 startTime = 0;    // 开始时间

  if (isInitialized == 0)
  {
    startTime = HAL_GetTick(); // 记录开始时间
    isInitialized = 1;         // 设置初始化标志
    return 0;                  // 返回未完成
  }

  if (HAL_GetTick() - startTime >= abs(SteerTime))
  {
    Set_Pwm(0, 0);     // 停止电机
    isInitialized = 0; // 重置初始化标志
    return 1;          // 返回完成
  }
  else
  {
    if (SteerTime > 0)
    {
      // 逆时针转向
      car_state.motor.left_pwm = car_state.running.openloop_steering_base_pwm - PWM_Value;  // 左轮反转
      car_state.motor.right_pwm = car_state.running.openloop_steering_base_pwm + PWM_Value; // 右轮正转
    }
    else
    {
      // 顺时针转向
      car_state.motor.left_pwm = car_state.running.openloop_steering_base_pwm + PWM_Value;  // 左轮正转
      car_state.motor.right_pwm = car_state.running.openloop_steering_base_pwm - PWM_Value; // 右轮反转
    }
    if (car_state.running.flag_stop == 1) // 如果停止标志为1，停止电机
      Set_Pwm(0, 0);                      // 停止电机
    else
      Set_Pwm(car_state.motor.left_pwm, car_state.motor.right_pwm); // 赋值给PWM寄存器
    return 0;                                                       // 返回未完成
  }
}

/**
 * @brief Local steering control function
 * @note This function is used to control the steering of the vehicle
 * 通过pid的方式使小车转过指定的角度
 * @param angle 小车目标应该转过的角度,逆时针为正,顺时针为负
 * @return u8 1:完成 0:小车正在转向中
 */
u8 localSteeringControl_Handler(float angle)
{
  static float targetAngle = 0;           // 目标角度
  static float lastError = 0;             // 上一次误差
  static float integral = 0;              // 积分项
  static float startAngle = 0;            // 开始角度
  static u8 isInitialized = 0;            // 初始化标志
  float currentAngle = getHeadingAngle(); // 获取当前角度
  float error, derivative, output;

  // 初始化，记录起始角度并计算目标角度（相对角度转换为绝对角度）
  if (isInitialized == 0)
  {
    startAngle = currentAngle;
    targetAngle = startAngle + angle; // 计算目标绝对角度

    // 规范化目标角度到±180度范围内
    while (targetAngle > 180.0f)
    {
      targetAngle -= 360.0f;
    }
    while (targetAngle < -180.0f)
    {
      targetAngle += 360.0f;
    }

    printf("开始旋转: 起始角度=%.2f, 目标角度=%.2f, 相对角度=%.2f\r\n",
           startAngle, targetAngle, angle);

    lastError = 0;
    integral = 0;
    car_state.running.steering_stable_count = 0;
    car_state.running.steering_completed = 0;
    isInitialized = 1;

    return 0; // 刚初始化，返回未完成
  }

  // 计算当前误差
  error = targetAngle - currentAngle;

  // 处理误差过大的情况（过±180度），确保选择最短路径
  if (error > 180.0f)
  {
    error -= 360.0f;
  }
  else if (error < -180.0f)
  {
    error += 360.0f;
  }

  // 计算积分项
  integral += error;

  // 积分限幅
  if (integral > STEERING_I_LIMIT)
  {
    integral = STEERING_I_LIMIT;
  }
  else if (integral < -STEERING_I_LIMIT)
  {
    integral = -STEERING_I_LIMIT;
  }

  // 如果误差很小，逐渐减小积分项，防止过冲
  if (fabs(error) < (float)car_state.pid.steering_error_threshold / 100.0f)
  {
    integral *= 0.9f;
  }

  // 计算微分项
  derivative = error - lastError;
  lastError = error;

  // 计算PID输出 - 使用放大100倍后的参数值，并转换回float
  output = ((float)car_state.pid.steering_kp / 100.0f) * error + ((float)car_state.pid.steering_ki / 1000.0f) * integral + ((float)car_state.pid.steering_kd / 100.0f) * derivative;

  // 输出限幅
  if (output > STEERING_MAX_OUTPUT)
  {
    output = STEERING_MAX_OUTPUT;
  }
  else if (output < STEERING_MIN_OUTPUT)
  {
    output = STEERING_MIN_OUTPUT;
  }

  if (output >= 0)
  {
    // 逆时针转向 (output 为正或零)
    // 左轮反转，右轮正转
    car_state.motor.left_pwm = -(PWM_Base + output);
    car_state.motor.right_pwm = PWM_Base + output;
  }
  else
  {
    // 顺时针转向 (output 为负)
    // 左轮正转，右轮反转，注意 output 本身为负数
    car_state.motor.left_pwm = PWM_Base - output;     // -output 为正值，增加左轮正转速度
    car_state.motor.right_pwm = -(PWM_Base - output); // -output 为正值，增加右轮反转速度的绝对值
  }

  // 误差在阈值范围内且变化率小，计数稳定时间
  if (fabs(error) < (float)car_state.pid.steering_error_threshold / 100.0f && fabs(derivative) < 0.5f)
  {
    car_state.running.steering_stable_count++;
    printf("误差: %.2f, 当前角度: %.2f, 目标角度: %.2f\r\n", error, currentAngle, targetAngle);
    // 如果稳定计数达到设定时间，认为转向完成
    if (car_state.running.steering_stable_count >= STEERING_STABLE_TIME)
    {
      // 重置状态，为下一次转向做准备
      isInitialized = 0;
      Set_Pwm(0, 0); // 停止电机
      car_state.running.steering_completed = 1;
      printf("转向完成！最终角度: %.2f\r\n", currentAngle);
      return 1; // 转向完成
    }
  }
  else
  {
    // 不稳定，重置稳定计数
    car_state.running.steering_stable_count = 0;
  }

  // 执行电机控制
  if (car_state.running.flag_stop == 1)
  {
    Set_Pwm(0, 0); // 停止标志为1时停止电机
  }
  else
  {
    Set_Pwm(car_state.motor.left_pwm, car_state.motor.right_pwm); // 设置电机PWM
  }

  return 0; // 转向未完成
}

/**
 * @brief This function is used to control the vehicle to move in a straight line
 * @note This function adjusts the vehicle's speed and direction to maintain a straight path
 * using gyroscope angle feedback for direction correction
 * @param void
 * @return u8 1:完成直线移动(即有传感器检测到了黑线) 0:小车正在直行中(没有传感器检测黑线)
 */
int moveForward_Handler(void)
{
  // 状态机状态枚举
  typedef enum
  {
    INIT,          // 初始化状态
    FORWARD_MOVING // 正常直线行驶状态
  } MoveForwardState;

  // 静态变量
  static MoveForwardState state = INIT;
  static float integral = 0.0f;     // 积分项
  static float lastError = 0.0f;    // 上一次误差
  static uint8_t debug_counter = 0; // 调试计数器
  static float referenceAngle = 0;
  float currentAngle, error, derivative, angleCorrection;
  bool can_print_debug = false;
  if (++debug_counter >= DEBUG_PRINT_COUNT)
  {
    debug_counter = 0;
    can_print_debug = true;
  }
  // 读取灰度传感器数据
  grey_sensor_Read();
  // 检查是否有传感器检测到黑线
  car_state.motor.turn_pwm = Calculate_Turn_Pwm();
  if (car_state.motor.turn_pwm != INT16_MIN)
  {
    Set_Pwm(0, 0); // 如果有传感器检测到黑线，停止电机
    // 重置所有状态变量，为下次调用准备
    state = INIT;
    integral = 0.0f;
    lastError = 0.0f;
    if (can_print_debug == true)
    {
      printf("传感器检测到黑线，停止电机\r\n");
    }
    return 1; // 返回1，表示完成直线移动
  }

  // 状态机处理
  switch (state)
  {
  case INIT:
    // 初始化状态 - 重置积分和误差
    referenceAngle = getHeadingAngle();
    // 边界条件处理 - 确保参考角度在±180度范围内
    while (referenceAngle > 180.0f)
    {
      referenceAngle -= 360.0f;
    }
    while (referenceAngle < -180.0f)
    {
      referenceAngle += 360.0f;
    }
    printf("开始按指定角度 %.2f 度直线行驶\r\n", referenceAngle);
    integral = 0.0f;
    lastError = 0.0f;
    state = FORWARD_MOVING;
    Set_Pwm(FORWARDBASE_PWM, FORWARDBASE_PWM);
    return 0;

  case FORWARD_MOVING:
    // 正常的直线行走阶段
    // 获取当前角度并计算误差
    currentAngle = getHeadingAngle();
    error = currentAngle - referenceAngle;

    // 处理误差过大的情况（过±180度），确保选择最短路径
    if (error > 180.0f)
    {
      error -= 360.0f;
    }
    else if (error < -180.0f)
    {
      error += 360.0f;
    }

    // 计算积分项
    integral += error;

    // 积分限幅
    if (integral > FORWARD_I_LIMIT)
    {
      integral = FORWARD_I_LIMIT;
    }
    else if (integral < -FORWARD_I_LIMIT)
    {
      integral = -FORWARD_I_LIMIT;
    }

    // 如果误差很小，逐渐减小积分项，防止过冲
    if (fabs(error) < (float)car_state.pid.forward_error_threshold / 100.0f)
    {
      integral *= 0.95f;
    }

    // 计算微分项
    derivative = error - lastError;
    lastError = error;

    // 计算PID输出 - 使用放大100倍后的参数值，并转换回float
    angleCorrection = ((float)car_state.pid.forward_kp / 100.0f) * error +
                      ((float)car_state.pid.forward_ki / 100.0f) * integral +
                      ((float)car_state.pid.forward_kd / 100.0f) * derivative;

    // 输出限幅
    if (angleCorrection > STEERING_MAX_OUTPUT)
    {
      angleCorrection = STEERING_MAX_OUTPUT;
    }
    else if (angleCorrection < STEERING_MIN_OUTPUT)
    {
      angleCorrection = STEERING_MIN_OUTPUT;
    }

    // 基于角度纠正计算左右轮差速
    car_state.motor.left_pwm = car_state.running.forward_base_pwm + angleCorrection;
    car_state.motor.right_pwm = car_state.running.forward_base_pwm - angleCorrection;

    // 打印调试信息（降低频率，避免刷屏）
    if (can_print_debug == true)
    { // 每隔一定次数打印一次
      printf("直线修正: 当前角度=%.2f, 参考角度=%.2f, 误差=%.2f, 修正值=%.2f\r\n",
             currentAngle, referenceAngle, error, angleCorrection);
    }
    break;
  }

  // 执行电机控制
  if (car_state.running.flag_stop == 1)
    Set_Pwm(0, 0); // 停止标志为1时停止电机
  else
    Set_Pwm(car_state.motor.left_pwm, car_state.motor.right_pwm); // 赋值给PWM寄存器

  return 0; // 返回0，表示没有完成直线移动，正在直行中
}

/**
 * @brief 指定角度直线行驶函数
 * @note 此函数可以接受一个参考角度作为参数，使小车按照指定的角度直线行驶
 * @param referenceAngle 小车行驶的参考角度(度)
 * @return int 1:完成直线行驶(检测到黑线) 0:小车正在直行中
 */
int moveForwardWithAngle_Handler(float referenceAngle)
{
  // 状态机状态枚举
  typedef enum
  {
    INIT,          // 初始化状态
    FORWARD_MOVING // 正常直线行驶状态
  } MoveForwardState;

  // 静态变量
  static MoveForwardState state = INIT;
  static float integral = 0.0f;     // 积分项
  static float lastError = 0.0f;    // 上一次误差
  static uint8_t debug_counter = 0; // 调试计数器
  float currentAngle, error, derivative, angleCorrection;

  bool can_print_debug = false;
  if (++debug_counter >= DEBUG_PRINT_COUNT)
  {
    debug_counter = 0;
    can_print_debug = true;
  }

  // 边界条件处理 - 确保参考角度在±180度范围内
  while (referenceAngle > 180.0f)
  {
    referenceAngle -= 360.0f;
  }
  while (referenceAngle < -180.0f)
  {
    referenceAngle += 360.0f;
  }

  // 读取灰度传感器数据
  grey_sensor_Read();
  // 检查是否有传感器检测到黑线
  car_state.motor.turn_pwm = Calculate_Turn_Pwm();
  if (car_state.motor.turn_pwm != INT16_MIN)
  {
    Set_Pwm(0, 0); // 如果有传感器检测到黑线，停止电机
    // 重置所有状态变量，为下次调用准备
    state = INIT;
    integral = 0.0f;
    lastError = 0.0f;
    if (can_print_debug == true)
    {
      printf("传感器检测到黑线，停止电机\r\n");
    }
    return 1; // 返回1，表示完成直线移动
  }

  // 状态机处理
  switch (state)
  {
  case INIT:
    // 初始化状态 - 重置积分和误差
    printf("开始按指定角度 %.2f 度直线行驶\r\n", referenceAngle);
    integral = 0.0f;
    lastError = 0.0f;
    state = FORWARD_MOVING;
    Set_Pwm(FORWARDBASE_PWM, FORWARDBASE_PWM);
    return 0;

  case FORWARD_MOVING:
    // 正常的直线行走阶段
    // 获取当前角度并计算误差
    currentAngle = getHeadingAngle();
    error = currentAngle - referenceAngle;

    // 处理误差过大的情况（过±180度），确保选择最短路径
    if (error > 180.0f)
    {
      error -= 360.0f;
    }
    else if (error < -180.0f)
    {
      error += 360.0f;
    }

    // 计算积分项
    integral += error;

    // 积分限幅
    if (integral > FORWARD_I_LIMIT)
    {
      integral = FORWARD_I_LIMIT;
    }
    else if (integral < -FORWARD_I_LIMIT)
    {
      integral = -FORWARD_I_LIMIT;
    }

    // 如果误差很小，逐渐减小积分项，防止过冲
    if (fabs(error) < (float)car_state.pid.forward_error_threshold / 100.0f)
    {
      integral *= 0.95f;
    }

    // 计算微分项
    derivative = error - lastError;
    lastError = error;

    // 计算PID输出 - 使用放大100倍后的参数值，并转换回float
    angleCorrection = ((float)car_state.pid.forward_kp / 100.0f) * error +
                      ((float)car_state.pid.forward_ki / 100.0f) * integral +
                      ((float)car_state.pid.forward_kd / 100.0f) * derivative;

    // 输出限幅
    if (angleCorrection > STEERING_MAX_OUTPUT)
    {
      angleCorrection = STEERING_MAX_OUTPUT;
    }
    else if (angleCorrection < STEERING_MIN_OUTPUT)
    {
      angleCorrection = STEERING_MIN_OUTPUT;
    }

    // 基于角度纠正计算左右轮差速
    car_state.motor.left_pwm = car_state.running.forward_base_pwm + angleCorrection;
    car_state.motor.right_pwm = car_state.running.forward_base_pwm - angleCorrection;

    // 打印调试信息（降低频率，避免刷屏）
    if (can_print_debug == true)
    { // 每隔一定次数打印一次
      printf("直线修正: 当前角度=%.2f, 参考角度=%.2f, 误差=%.2f, 修正值=%.2f\r\n",
             currentAngle, referenceAngle, error, angleCorrection);
    }
    break;
  }

  // 执行电机控制
  if (car_state.running.flag_stop == 1)
    Set_Pwm(0, 0); // 停止标志为1时停止电机
  else
    Set_Pwm(car_state.motor.left_pwm, car_state.motor.right_pwm); // 赋值给PWM寄存器

  return 0; // 返回0，表示没有完成直线移动，正在直行中
}

/**
 * @brief 控制小车转向到指定的绝对角度
 * @note 这个函数控制小车转向到一个指定的绝对航向角度，不是相对转动角度
 * 当车头指向正西方向时候返回值为0度,正北方向为-90度,正南方向为90度
 * @param targetAbsoluteAngle 目标绝对角度，范围[-180, 180]度
 * @return u8 1:转向完成 0:小车正在转向中
 */
u8 turnToAbsoluteAngle(float targetAbsoluteAngle)
{
  static float lastError = 0;             // 上一次误差
  static float integral = 0;              // 积分项
  static u8 isInitialized = 0;            // 初始化标志
  static uint8_t debug_counter = 0;       // 调试计数器
  static float angleDifference = 0;       // 目标角度与起始角度的差值
  float currentAngle = getHeadingAngle(); // 获取当前角度
  float error, derivative, output;
  bool can_print_debug = false;
  if (++debug_counter >= DEBUG_PRINT_COUNT)
  {
    debug_counter = 0;
    can_print_debug = true;
  }

  // 确保目标角度在[-180, 180]范围内
  while (targetAbsoluteAngle > 180.0f)
  {
    targetAbsoluteAngle -= 360.0f;
  }
  while (targetAbsoluteAngle < -180.0f)
  {
    targetAbsoluteAngle += 360.0f;
  }

  // 初始化，重置控制变量
  if (isInitialized == 0)
  {
    printf("开始旋转到绝对角度: 当前角度=%.2f, 目标角度=%.2f\r\n",
           currentAngle, targetAbsoluteAngle);

    angleDifference = targetAbsoluteAngle - currentAngle;
    // 处理角度差值，确保在[-180, 180]范围内
    if (angleDifference > 180.0f)
    {
      angleDifference -= 360.0f;
    }
    else if (angleDifference < -180.0f)
    {
      angleDifference += 360.0f;
    }
    lastError = 0;
    integral = 0;
    car_state.running.steering_stable_count = 0;
    car_state.running.steering_completed = 0;
    isInitialized = 1;

    return 0; // 刚初始化，返回未完成
  }

  // 计算当前误差
  error = targetAbsoluteAngle - currentAngle;

  // 关键处理: 处理误差过大的情况（过±180度），确保选择最短路径
  if (error > 180.0f)
  {
    error -= 360.0f;
  }
  else if (error < -180.0f)
  {
    error += 360.0f;
  }

  // 计算积分项
  integral += error;

  // 积分限幅
  if (integral > STEERING_I_LIMIT)
  {
    integral = STEERING_I_LIMIT;
  }
  else if (integral < -STEERING_I_LIMIT)
  {
    integral = -STEERING_I_LIMIT;
  }

  // 如果误差很小，逐渐减小积分项，防止过冲
  if (fabs(error) < (float)car_state.pid.steering_error_threshold / 100.0f * 0.85f)
  {
    integral *= 0.9f;
  }

  // 计算微分项
  derivative = error - lastError;
  lastError = error;

  // 计算PID输出 - 使用放大100倍后的参数值，并转换回float
  output = ((float)car_state.pid.steering_kp / 100.0f) * error +
           ((float)car_state.pid.steering_ki / 100.0f) * integral +
           ((float)car_state.pid.steering_kd / 100.0f) * derivative;

  // 输出限幅
  if (output > STEERING_MAX_OUTPUT)
  {
    output = STEERING_MAX_OUTPUT;
  }
  else if (output < STEERING_MIN_OUTPUT)
  {
    output = STEERING_MIN_OUTPUT;
  }

  if (output >= 0)
  {
    // 逆时针转向 (output 为正或零)
    // 左轮反转，右轮正转
    car_state.motor.left_pwm = -(PWM_Base + output);
    car_state.motor.right_pwm = PWM_Base + output;
  }
  else
  {
    // 顺时针转向 (output 为负)
    // 左轮正转，右轮反转，注意 output 本身为负数
    car_state.motor.left_pwm = PWM_Base - output;     // -output 为正值，增加左轮正转速度
    car_state.motor.right_pwm = -(PWM_Base - output); // -output 为正值，增加右轮反转速度的绝对值
  }

  // 误差在阈值范围内且变化率小，计数稳定时间
  if (fabs(error) < (float)car_state.pid.steering_error_threshold / 100.0f && fabs(derivative) < 1.2f)
  {
    car_state.running.steering_stable_count++;

    // 调试输出
    if (car_state.running.steering_stable_count % 20 == 0)
    { // 减少打印频率
      printf("误差: %.2f, 当前角度: %.2f, 目标角度: %.2f\r\n",
             error, currentAngle, targetAbsoluteAngle);
    }

    // 如果稳定计数达到设定时间，认为转向完成
    if (car_state.running.steering_stable_count >= STEERING_STABLE_TIME)
    {
      // 重置状态，为下一次转向做准备
      isInitialized = 0;
      Set_Pwm(0, 0); // 停止电机
      car_state.running.steering_completed = 1;
      printf("转向到绝对角度完成！最终角度: %.2f\r\n", currentAngle);
      return 1; // 转向完成
    }
  }
  else
  {
    // 不稳定，重置稳定计数
    car_state.running.steering_stable_count = 0;
  }

  if (can_print_debug == true)
  {
    // 打印调试信息
    printf("PID调试: 当前角度=%.2f, 目标角度=%.2f, 误差=%.2f, 修正值=%.2f\r\n",
           currentAngle, targetAbsoluteAngle, error, output);
  }
  if (angleDifference > 0)
  {
    // 逆时针转向
    car_state.motor.left_pwm = 0; // 左轮停止
  }
  else
  {
    // 顺时针转向
    car_state.motor.right_pwm = 0; // 右轮停止
  }
  // 执行电机控制
  if (car_state.running.flag_stop == 1)
  {
    Set_Pwm(0, 0); // 停止标志为1时停止电机
  }
  else
  {
    Set_Pwm(car_state.motor.left_pwm, car_state.motor.right_pwm); // 设置电机PWM
  }

  return 0; // 转向未完成
}

/**
 * @brief angleSetWithKey_Handler函数
 * @note 通过按键设置角度的处理函数
 * @return None
 */
void angleSetWithKey_Handler(void)
{
  typedef enum
  {
    INITIAL,                   // 初始状态
    SET_INITIAL_ANGLE,         // 设置初始角度
    SET_TASK_ROTATION_ANGLE_1, // 设置任务旋转角度1
    SET_TASK_ROTATION_ANGLE_3, // 设置任务旋转角度3
    SET_TASK_ROTATION_ANGLE_4  // 设置任务旋转角度4
  } AngleSetState;

  static AngleSetState state = INITIAL;     // 初始化状态机
  static uint32_t key_press_start_time = 0; // 按键按下开始时间
  static uint8_t is_key_pressed = 0;        // 按键是否被按下
  static uint8_t long_press_handled = 0;    // 长按是否已处理标志

  // 获取按键状态
  car_state.misc.key_state = Get_start_task_Pin_value();

  // 按键状态处理
  if (car_state.misc.key_state == 1) // 检测到按下按键
  {
    if (is_key_pressed == 0) // 第一次检测到按下
    {
      key_press_start_time = HAL_GetTick(); // 记录按下开始时间
      is_key_pressed = 1;                   // 标记按键已按下
      long_press_handled = 0;               // 重置长按处理标志
      printf("按键按下，开始计时\r\n");     // 调试信息
    }
    else // 按键持续按下
    {
      uint32_t press_duration = HAL_GetTick() - key_press_start_time;

      // 调试输出：每500ms打印一次按键持续时间
      if (press_duration % 500 == 0)
      {
        printf("按键持续按下时间: %lu ms\r\n", press_duration);
      }

      if (press_duration >= LONG_PRESS_THRESHOLD && long_press_handled == 0)
      {
        // 满足长按条件，直接启动小车
        printf("检测到长按（%lums），启动小车！\r\n", press_duration);
        state = INITIAL; // 重置状态机
        // 启动小车
        if (car_state.running.flag_stop == 1)
        {                       // 只有在停止状态下才启动
          HAL_TIM6_toggle_IT(); // 切换定时器6中断
          resetTask();          // 重置任务
          state = INITIAL;      // 重置状态机
          toggle_Flag_Stop();   // 切换停止标志
          all_leds_off();
          car_state.running.manual_mode = 1; // 手动设置了角度
        }
        // 标记长按已处理
        long_press_handled = 1;
      }
    }
  }
  else // 按键松开
  {
    if (is_key_pressed) // 之前按键是按下的，现在松开了
    {
      uint32_t press_duration = HAL_GetTick() - key_press_start_time;
      printf("按键释放，按下持续时间: %lu ms\r\n", press_duration);

      is_key_pressed = 0; // 重置按键按下标记

      // 只有短按才处理状态机逻辑
      if (press_duration < LONG_PRESS_THRESHOLD)
      {
        float reverseInitialAngle = car_state.attitude.initial_angle + 180; // 计算反向初始角度
        // 规范化反向初始角度到[-180, 180]范围内
        while (reverseInitialAngle > 180.0f)
        {
          reverseInitialAngle -= 360.0f;
        }
        while (reverseInitialAngle < -180.0f)
        {
          reverseInitialAngle += 360.0f;
        }

        // 状态机处理逻辑
        switch (state)
        {
        case INITIAL:
          state = SET_INITIAL_ANGLE;       // 设置初始角度
          car_state.running.flag_stop = 1; // 设置停止标志
          HAL_TIM6_toggle_IT();            // 切换定时器6中断
          Set_Pwm(0, 0);                   // 停止电机
          led1_on();                       // 打开LED1
          break;

        case SET_INITIAL_ANGLE:
          state = SET_TASK_ROTATION_ANGLE_1;                                  // 设置任务旋转角度1
          car_state.attitude.initial_angle = getHeadingAngle();               // 获取当前航向角度
          printf("设置初始角度: %.2f\r\n", car_state.attitude.initial_angle); // 打印初始角度
          led2_on();                                                          // 打开LED2
          break;

        case SET_TASK_ROTATION_ANGLE_1:
          Task4_Rotation_Angle_1 = Task3_Rotation_Angle_1 = car_state.attitude.initial_angle - getHeadingAngle(); // 计算任务旋转角度1
          // 规范化任务旋转角度1到[-180, 180]范围内
          while (Task3_Rotation_Angle_1 > 180.0f)
          {
            Task3_Rotation_Angle_1 -= 360.0f;
          }
          while (Task3_Rotation_Angle_1 < -180.0f)
          {
            Task3_Rotation_Angle_1 += 360.0f;
          }
          state = SET_TASK_ROTATION_ANGLE_4;                             // 设置任务旋转角度4
          printf("设置任务旋转角度1: %.2f\r\n", Task3_Rotation_Angle_1); // 打印任务旋转角度1
          led3_on();                                                     // 打开LED3
          break;

        case SET_TASK_ROTATION_ANGLE_4:
          state = SET_TASK_ROTATION_ANGLE_3;                                             // 设置任务旋转角度4
          Task4_Rotation_Angle_3 = car_state.attitude.initial_angle - getHeadingAngle(); // 计算任务旋转角度4
          while (Task4_Rotation_Angle_3 > 180.0f)
          {
            Task4_Rotation_Angle_3 -= 360.0f;
          }
          while (Task4_Rotation_Angle_3 < -180.0f)
          {
            Task4_Rotation_Angle_3 += 360.0f;
          }

          led1_off();                                                    // 关闭LED1
          printf("设置任务旋转角度2: %.2f\r\n", Task3_Rotation_Angle_2); // 打印任务旋转角度3
          break;

        case SET_TASK_ROTATION_ANGLE_3:
          state = INITIAL;                                                                           // 返回初始状态
          Task4_Rotation_Angle_2 = Task3_Rotation_Angle_2 = getHeadingAngle() - reverseInitialAngle; // 计算任务旋转角度3
          // 规范化任务旋转角度3到[-180, 180]范围内
          while (Task3_Rotation_Angle_2 > 180.0f)
          {
            Task3_Rotation_Angle_2 -= 360.0f;
          }
          while (Task3_Rotation_Angle_2 < -180.0f)
          {
            Task3_Rotation_Angle_2 += 360.0f;
          }

          led1_off();                                                    // 关闭LED1
          led2_off();                                                    // 关闭LED2
          led3_off();                                                    // 关闭LED3
          car_state.running.manual_mode = 1;                             // 手动设置了角度
          printf("设置任务旋转角度3: %.2f\r\n", Task4_Rotation_Angle_3); // 打印任务旋转角度3
          printf("初始角度: %.2f , 任务旋转角度1: %.2f, 任务旋转角度2: %.2f, 任务旋转角度3: %.2f\r\n",
                 car_state.attitude.initial_angle, Task3_Rotation_Angle_1, Task3_Rotation_Angle_2, Task4_Rotation_Angle_3);
          HAL_TIM6_toggle_IT(); // 切换定时器6中断
          resetTask();          // 重置任务
          toggle_Flag_Stop();   // 切换停止标志
          break;
        }
      }
    }
  }
}

/**
 * @brief 指定角度直线行驶函数,直线行驶直到灰度传感器的某半部分检测到黑线
 * @note 此函数可以接受一个参考角度作为参数，使小车按照指定的角度直线行驶
 * @param referenceAngle 小车行驶的参考角度(度)
 * @param start_graySensor 起始灰度传感器编号(从1开始)
 * @param end_graySensor 结束灰度传感器编号
 * @return int 1:完成直线行驶(检测到黑线) 0:小车正在直行中
 */
int moveForwardWithAngle_UntileSomeGraySencorActived_Handler(float referenceAngle, u8 start_graySensor, u8 end_graySensor)
{
  // 状态机状态枚举
  typedef enum
  {
    INIT,          // 初始化状态
    FORWARD_MOVING // 正常直线行驶状态
  } MoveForwardState;

  // 静态变量
  static MoveForwardState state = INIT;
  static float integral = 0.0f;     // 积分项
  static float lastError = 0.0f;    // 上一次误差
  static uint8_t debug_counter = 0; // 调试计数器
  float currentAngle, error, derivative, angleCorrection;

  bool can_print_debug = false;
  if (++debug_counter >= DEBUG_PRINT_COUNT)
  {
    debug_counter = 0;
    can_print_debug = true;
  }

  // 边界条件处理 - 确保参考角度在±180度范围内
  while (referenceAngle > 180.0f)
  {
    referenceAngle -= 360.0f;
  }
  while (referenceAngle < -180.0f)
  {
    referenceAngle += 360.0f;
  }

  // 读取灰度传感器数据
  grey_sensor_Read();
  // 检查是否有传感器检测到黑线
  car_state.motor.turn_pwm = IsCertainGraySenorsAcrived(start_graySensor - 1, end_graySensor - 1);
  if (car_state.motor.turn_pwm != INT16_MIN)
  {
    Set_Pwm(0, 0); // 如果有传感器检测到黑线，停止电机
    // 重置所有状态变量，为下次调用准备
    state = INIT;
    integral = 0.0f;
    lastError = 0.0f;
    if (can_print_debug == true)
    {
      printf("传感器检测到黑线，停止电机\r\n");
    }
    return 1; // 返回1，表示完成直线移动
  }

  // 状态机处理
  switch (state)
  {
  case INIT:
    // 初始化状态 - 重置积分和误差
    printf("开始按指定角度 %.2f 度直线行驶\r\n", referenceAngle);
    integral = 0.0f;
    lastError = 0.0f;
    state = FORWARD_MOVING;
    Set_Pwm(FORWARDBASE_PWM, FORWARDBASE_PWM);
    return 0;

  case FORWARD_MOVING:
    // 正常的直线行走阶段
    // 获取当前角度并计算误差
    currentAngle = getHeadingAngle();
    error = currentAngle - referenceAngle;

    // 处理误差过大的情况（过±180度），确保选择最短路径
    if (error > 180.0f)
    {
      error -= 360.0f;
    }
    else if (error < -180.0f)
    {
      error += 360.0f;
    }

    // 计算积分项
    integral += error;

    // 积分限幅
    if (integral > FORWARD_I_LIMIT)
    {
      integral = FORWARD_I_LIMIT;
    }
    else if (integral < -FORWARD_I_LIMIT)
    {
      integral = -FORWARD_I_LIMIT;
    }

    // 如果误差很小，逐渐减小积分项，防止过冲
    if (fabs(error) < (float)car_state.pid.forward_error_threshold / 100.0f)
    {
      integral *= 0.95f;
    }

    // 计算微分项
    derivative = error - lastError;
    lastError = error;

    // 计算PID输出 - 使用放大100倍后的参数值，并转换回float
    angleCorrection = ((float)car_state.pid.forward_kp / 100.0f) * error +
                      ((float)car_state.pid.forward_ki / 100.0f) * integral +
                      ((float)car_state.pid.forward_kd / 100.0f) * derivative;

    // 输出限幅
    if (angleCorrection > STEERING_MAX_OUTPUT)
    {
      angleCorrection = STEERING_MAX_OUTPUT;
    }
    else if (angleCorrection < STEERING_MIN_OUTPUT)
    {
      angleCorrection = STEERING_MIN_OUTPUT;
    }

    // 基于角度纠正计算左右轮差速
    car_state.motor.left_pwm = car_state.running.forward_base_pwm + angleCorrection;
    car_state.motor.right_pwm = car_state.running.forward_base_pwm - angleCorrection;

    // 打印调试信息（降低频率，避免刷屏）
    if (can_print_debug == true)
    { // 每隔一定次数打印一次
      printf("直线修正: 当前角度=%.2f, 参考角度=%.2f, 误差=%.2f, 修正值=%.2f\r\n",
             currentAngle, referenceAngle, error, angleCorrection);
    }
    break;
  }

  // 执行电机控制
  if (car_state.running.flag_stop == 1)
    Set_Pwm(0, 0); // 停止标志为1时停止电机
  else
    Set_Pwm(car_state.motor.left_pwm, car_state.motor.right_pwm); // 赋值给PWM寄存器

  return 0; // 返回0，表示没有完成直线移动，正在直行中
}
