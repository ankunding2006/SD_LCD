/**
 * @file control.c
 * @brief 控制任务
 * @note:介绍一下硬件系统,我的车轮使用了两个伺服电机作为后轮驱动,电机的地址分别为0x01和0x02
 * @version 0.1
 * @date 2025-07-15
 * @copyright Copyright (c) 2025
 */
#include "control.h"
#include "main.h"
#include <stdlib.h>
#include "Emm_V5.h"
#include "gray_detection.h"
#include "car_config.h"

uint16_t usart2_rx_buffer = 9;   // 双字节接收缓冲区
volatile uint8_t crossroads = 0; // 到十字路口的次数
volatile uint16_t room_num = 0;  // 病房编号

/**
 * @brief 设置两个电机的速度
 * @param left_speed 左电机速度(单位:转/分钟)
 * @param right_speed 右电机速度(单位:转/分钟)
 * @note 速度都为正数的时候,小车向前走,速度都为负数的时候,小车向后走,速度一个为正一个为负的时候,小车向左转,速度一个为负一个为正的时候,小车向右转
 * @param acc 加速度(0-255)
 */
void set_motor_speed(int16_t left_speed, int16_t right_speed, uint8_t acc)
{
  if (left_speed > 0)
    Emm_V5_Vel_Control(0x01, 1, left_speed, acc, 0);
  else
    Emm_V5_Vel_Control(0x01, 0, -left_speed, acc, 0);
  if (right_speed > 0)
    Emm_V5_Vel_Control(0x02, 1, right_speed, acc, 0);
  else
    Emm_V5_Vel_Control(0x02, 0, -right_speed, acc, 0);
}

/**
 * @brief 这个函数用来控制小车进行循迹
 * @return 如果完成了循迹,就返回1,否则返回0
 */
uint8_t line_following_task(void)
{
  // 1. 更新传感器数据
  grey_sensor_Read();

  // 2. 计算转向值
  int turn_value = Calculate_Turn_Value();

  if (turn_value == INT16_MIN)
  {
    // 没有检测到线，可能脱轨了
    // 策略：原地停车
    set_motor_speed(0, 0, DEFAULT_ACCELERATION);
  }
  else if (turn_value == INT16_MAX)
  {
    set_motor_speed(CAR_BASE_SPEED, CAR_BASE_SPEED, DEFAULT_ACCELERATION);
  }
  else
  {
    // 根据转向值调整左右轮速度
    // turn_value > 0 表示线在左边，需要左转，左轮慢，右轮快
    // turn_value < 0 表示线在右边，需要右转，左轮快，右轮慢
    int16_t left_speed = CAR_BASE_SPEED - turn_value;
    int16_t right_speed = CAR_BASE_SPEED + turn_value;

    // 对速度进行限幅
    if (left_speed > CAR_MAX_SPEED)
      left_speed = CAR_MAX_SPEED;
    if (left_speed < 0)
      left_speed = 0;
    if (right_speed > CAR_MAX_SPEED)
      right_speed = CAR_MAX_SPEED;
    if (right_speed < 0)
      right_speed = 0;

    set_motor_speed(left_speed, right_speed, DEFAULT_ACCELERATION); // 使用一个默认加速度
  }

  return 0; // 循迹未完成
}

/**
 * @brief Open loop steering control function
 *
 * @param angle 小车旋转的时间(ms),逆时针旋转为正,顺时针旋转为负.
 * @param Rotate_Speed 转向转速值(单位:转/分钟)
 * @param Rotate_Speed_Base 基础转速值(单位:转/分钟)
 * @return uint8_t 1:完成 0:小车正在转向中
 */
uint8_t open_loop_steering_control(int16_t angle, int16_t Rotate_Speed, int16_t Rotate_Speed_Base)
{
  typedef enum
  {
    STEER_IDLE,
    STEERING
  } SteeringState_t;
  static SteeringState_t steering_state = STEER_IDLE;
  static uint32_t turn_end_time = 0;

  if (steering_state == STEER_IDLE)
  {
    if (angle == 0)
    {
      return 1; // Not told to do anything, so we are "done"
    }

    // New command received while idle, start turning.
    int16_t left_speed, right_speed;

    if (angle > 0)
    { // Counter-clockwise turn (left)
      left_speed = Rotate_Speed_Base - Rotate_Speed;
      right_speed = Rotate_Speed_Base + Rotate_Speed;
    }
    else
    { // Clockwise turn (right)
      left_speed = Rotate_Speed_Base + Rotate_Speed;
      right_speed = Rotate_Speed_Base - Rotate_Speed;
    }

    set_motor_speed(left_speed, right_speed, DEFAULT_ACCELERATION);
    turn_end_time = HAL_GetTick() + abs(angle);
    steering_state = STEERING;
    return 0; // Turn initiated, not complete yet.
  }
  else // steering_state == STEERING
  {
    // A turn is in progress, check if it's time to stop.
    // Ignore new angle/speed values until the current turn is complete.
    if (HAL_GetTick() >= turn_end_time)
    {
      set_motor_speed(0, 0, DEFAULT_ACCELERATION); // Stop motors
      steering_state = STEER_IDLE;                 // Reset state for next command
      return 1;                                    // Turn completed
    }
    return 0; // Still turning
  }
}

/**
 * @brief 视觉接收串口初始化
 * @param 无：懒得改，写死了
 *
 */
void visual_reception_init()
{
  // HAL_UART_Receive_IT(&huart2, &usart2_rx_buffer, 1);
}

/**
 * @brief 接收视觉传来的数据，接收到正确数据后会执行“一次”处理，然后重新启动中断接收
 * 函数会把识别到的房间号以从左到右排列为一个1/2/4位数保存到room_num中,例如如果识别
 * 到数字1在则room_num=1,如果识别到从左到右排列的数字1,2则room_num=12,如果从左到右
 * 依次识别到数字7,5,6,8则room_num=7568
 * @param 无：懒得改，写死了
 * @return 1、2、3代表第几个十字路口，0为错误
 */
/* TODO: 这个函数需要修改，现在识别到数字1，2，3，4，5，6，7，8，9，0，会分别使room_num=1，2，3，4，5，6，7，8，9，0
 * 需要修改为当识别到1 2，3 4，5 6，7 8，会分别使room_num=12，34，56，78，,识别到5 6 7 8会使room_num=5678
 */
uint8_t visual_process_command(void) // 处理相应消息
{
  // switch (usart2_rx_buffer)
  // {
  // case 1:
  // case 2:
  //   room_num = usart2_rx_buffer;
  //   printf("num: %d\n", usart2_rx_buffer);
  //   usart2_rx_buffer = 9;                               // 清除接收缓冲区
  //   HAL_UART_Receive_IT(&huart2, &usart2_rx_buffer, 1); // 重新开启中断接收
  //   return 1;
  // case 3:
  // case 4:
  //   room_num = usart2_rx_buffer;
  //   printf("num: %d\n", usart2_rx_buffer);
  //   usart2_rx_buffer = 9;                               // 清除接收缓冲区
  //   HAL_UART_Receive_IT(&huart2, &usart2_rx_buffer, 1); // 重新开启中断接收
  //   return 2;
  // case 5:
  // case 6:
  // case 7:
  // case 8:
  //   room_num = usart2_rx_buffer;
  //   printf("num: %d\n", usart2_rx_buffer);
  //   usart2_rx_buffer = 9;                               // 清除接收缓冲区
  //   HAL_UART_Receive_IT(&huart2, &usart2_rx_buffer, 1); // 重新开启中断接收
  //   return 3;
  // default:
  //   if (usart2_rx_buffer != 9)
  //   {
  //     printf("bug \n");
  //     usart2_rx_buffer = 9;                               // 清除接收缓冲区
  //     HAL_UART_Receive_IT(&huart2, &usart2_rx_buffer, 1); // 重新开启中断接收
  //   }
  //   return 0;
  // }
  return 0;
}
