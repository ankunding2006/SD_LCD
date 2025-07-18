/**
 * @file control.c
 * @brief 控制任务
 * @note:介绍一下硬件系统,我的车轮使用了两个伺服电机作为后轮驱动,电机的地址分别为0x01和0x02
 * @version 0.2
 * @date 2025-07-16
 * @copyright Copyright (c) 2025
 */
#include "control.h"
#include "main.h"
#include <stdlib.h>
#include <string.h>
#include "Emm_V5.h"
#include "gray_detection.h"
#include "car_config.h"

uint8_t usart2_rx_buffer = 9;    // 双字节接收缓冲区
volatile uint8_t crossroads = 0; // 到十字路口的次数
volatile uint16_t room_num = 0;  // 病房编号

// --- 电机速度控制状态机 ---
typedef enum
{
  MOTOR_TASK_IDLE,       // 空闲状态，没有任务
  MOTOR_TASK_SEND_LEFT,  // 等待发送左轮速度指令
  MOTOR_TASK_WAIT_DELAY, // 在两次发送之间等待1ms
  MOTOR_TASK_SEND_RIGHT, // 等待发送右轮速度指令
} MotorTaskState_t;

typedef struct
{
  int16_t left_speed;
  int16_t right_speed;
  uint8_t acc;
} MotorSpeedTask_t;

#define MOTOR_TASK_QUEUE_SIZE 5
static MotorSpeedTask_t motor_task_queue[MOTOR_TASK_QUEUE_SIZE];
static volatile uint8_t queue_head = 0;
static volatile uint8_t queue_tail = 0;

static volatile MotorTaskState_t motor_task_state = MOTOR_TASK_IDLE;
static uint32_t delay_end_time = 0;

/**
 * @brief 添加一个新的电机速度控制任务到队列
 * @param left_speed 左电机速度
 * @param right_speed 右电机速度
 * @param acc 加速度
 * @return 0: 成功, -1: 队列已满
 */
int8_t set_motor_speed(int16_t left_speed, int16_t right_speed, uint8_t acc)
{
  uint8_t next_head = (queue_head + 1) % MOTOR_TASK_QUEUE_SIZE;
  if (next_head == queue_tail)
  {
    return -1; // 队列已满
  }
  motor_task_queue[queue_head].left_speed = left_speed;
  motor_task_queue[queue_head].right_speed = right_speed;
  motor_task_queue[queue_head].acc = acc;
  queue_head = next_head;
  return 0;
}

/**
 * @brief 电机速度任务处理器（应在主循环或定时器中调用）
 */
void motor_speed_task_handler(void)
{
  // 如果队列为空且状态机空闲，则无事可做
  if (queue_head == queue_tail && motor_task_state == MOTOR_TASK_IDLE)
  {
    return;
  }

  // 如果状态机空闲，但队列中有任务，则启动新任务
  if (motor_task_state == MOTOR_TASK_IDLE)
  {
    motor_task_state = MOTOR_TASK_SEND_LEFT;
  }

  // 获取当前任务，但不弹出
  MotorSpeedTask_t *current_task = &motor_task_queue[queue_tail];

  // 状态机执行
  if (motor_task_state == MOTOR_TASK_SEND_LEFT)
  {
    int16_t speed = current_task->left_speed;
    uint8_t dir = (speed > 0);
    if (Emm_V5_Vel_Control(0x01, dir, abs(speed), current_task->acc, 0) == HAL_OK)
    {
      // 发送成功，状态机推进到等待延时
      motor_task_state = MOTOR_TASK_WAIT_DELAY;
      delay_end_time = HAL_GetTick() + 1; // 设置1ms延时
    }
  }
  else if (motor_task_state == MOTOR_TASK_WAIT_DELAY)
  {
    // 等待1ms延时结束
    if (HAL_GetTick() >= delay_end_time)
    {
      motor_task_state = MOTOR_TASK_SEND_RIGHT;
    }
  }
  else if (motor_task_state == MOTOR_TASK_SEND_RIGHT)
  {
    // 如果我们能进入这个状态，说明延时已到
    int16_t speed = current_task->right_speed;
    uint8_t dir = (speed > 0);
    if (Emm_V5_Vel_Control(0x02, dir, abs(speed), current_task->acc, 0) == HAL_OK)
    {
      // 右轮也发送成功，此任务完成
      motor_task_state = MOTOR_TASK_IDLE;
      // 弹出队列
      queue_tail = (queue_tail + 1) % MOTOR_TASK_QUEUE_SIZE;
    }
  }
}

/**
 * @brief UART发送完成回调函数
 * @note 这个函数需要在 stm32f4xx_it.c 的 `HAL_UART_TxCpltCallback` 中被调用
 */
void control_uart_tx_cplt_callback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == Emm_V5_HUART.Instance)
  {
    // 目前我们的状态机推进逻辑是同步的，
    // 即在handler中发送成功后立刻推进状态。
    // DMA的非阻塞特性意味着我们可以在这里处理更复杂的异步逻辑，
    // 但为了简化，当前实现依赖于handler的轮询。
  }
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
 * @param steerTime 小车旋转的时间(ms),逆时针旋转为正,顺时针旋转为负.
 * @param Rotate_Speed 转向转速值(单位:转/分钟)
 * @param Rotate_Speed_Base 基础转速值(单位:转/分钟)
 * @return uint8_t 1:完成 0:小车正在转向中
 */
uint8_t open_loop_steering_control(int16_t steerTime, int16_t Rotate_Speed, int16_t Rotate_Speed_Base)
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
    if (steerTime == 0)
    {
      return 1; // Not told to do anything, so we are "done"
    }

    // New command received while idle, start turning.
    int16_t left_speed, right_speed;

    if (steerTime > 0)
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
    turn_end_time = HAL_GetTick() + abs(steerTime);
    steering_state = STEERING;
    return 0; // Turn initiated, not complete yet.
  }
  else // steering_state == STEERING
  {
    // A turn is in progress, check if it's time to stop.
    // Ignore new steerTime/speed values until the current turn is complete.
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
  HAL_UART_Receive_IT(&huart2, &usart2_rx_buffer, 1);
}

/**
 * @brief 接收视觉传来的数据，接收到正确数据后会执行“一次”处理，然后重新启动中断接收
 * 函数会把识别到的房间号以从左到右排列为一个1/2/4位数保存到room_num中,例如如果识别
 * 到数字1在则room_num=1,如果识别到从左到右排列的数字1,2则room_num=12,如果从左到右
 * 依次识别到数字7,5,6,8则room_num=7568
 * @param task：当前执行的任务
 * @return 1、2、3代表十字路口，0为错误，记得写一下为零的时候的处理
 */
uint8_t visual_process_command(uint8_t task, uint8_t task_flag) // 处理相应消息
{
  static uint16_t temp = 0;
  temp = usart2_rx_buffer;
  // 前两个任务的处理
  if (temp != usart2_rx_buffer && task < 3)
  {
    switch (temp)
    {
    // task1 然后不存在12 21的情况
    case 19:
    case 91:
      room_num = 1;
      return 1;
    case 29:
    case 92:
      room_num = 2;
      return 1;

    // task2 全列举出来就好了
    case 34:
    case 43:
      room_num = temp;
      return 2;
    case 39:
    case 94:
      room_num = 34;
      return 2;
    case 93:
    case 49:
      room_num = 43;
      return 2;

    default:
      printf("前两个任务识别抽风了,等下次识别,此次数字为 %d \n", temp);
      return 0;
    }
  }

  static int LL = 0, L = 0, R = 0, RR = 0; // LL L R RR 为十字路口的四个数字 对应了位置
  // 需要task3_flag为一个摆头之前、之后的标志位 0为发车 1为摆头之前 2为摆头之后 3为回到路口准备走 4为他到了分支的那个十字路口  默认摆头是去看左侧的数字
  // 发车时识别对应房间号
  if (temp != usart2_rx_buffer && task == 3 && task_flag == 0)
  {
    if (temp % 10 == 9)
      room_num = temp / 10;
    else if (temp / 10 == 9)
      room_num = temp % 10;
    return 3;
  }
  else if (temp != usart2_rx_buffer && task == 3 && task_flag == 1)
  {
    if (temp % 10 == 9)
      L = temp / 10;
    else if (temp / 10 == 9)
      R = temp % 10;
    else
    {
      L = temp / 10;
      R = temp % 10;
    }

    if (!L || !R)
    {
      printf("未完成十字路口处识别");
      return 0;
    }
    else
    {
      return 3;
    }
  }
  else if (temp != usart2_rx_buffer && task == 3 && task_flag == 2)
  {
    if (temp % 10 == 9)
      LL = temp / 10;
    else if (temp / 10 == 9)
      L = temp % 10;
    else
    {
      LL = temp / 10;
      L = temp % 10;
    }
    if (!L || !LL)
    {
      printf("未完成左侧识别");
      return 0;
    }
    else
    {
      return 3;
    }
  }
  else if (temp != usart2_rx_buffer && task == 3 && task_flag == 3)
  {
    if (!L && !LL && !R)
    {
      RR = 26 - L - LL - R;
      room_num = LL * 1000 + L * 100 + R * 10 + RR;
      return 3;
    }
    else
    {
      printf("未得到标准数据");
      return 0;
    }
  }
  else if (temp != usart2_rx_buffer && task == 3 && task_flag == 4)
  {
    if (1) // 第一个十字路口左转，标志位自己传，我躺了
    {
      if (temp % 10 == 9) // 识别到一个数字在左侧
      {
        if (temp / 10 == L)
        {
          room_num = L * 10 + LL;
        }
        else if (temp / 10 == LL)
        {
          room_num = LL * 10 + L;
        }
        else
        {
          printf("分支路段与主干路段对不上");
          return 0;
        }
      }
      else if (temp / 10 == 9)
      {
        if (temp / 10 == LL)
        {
          room_num = L * 10 + LL;
        }
        else if (temp / 10 == L)
        {
          room_num = LL * 10 + L;
        }
        else
        {
          printf("分支路段与主干路段对不上");
          return 0;
        }
      }
      else if (temp == LL + L * 10 || temp == LL * 10 + L)
      {
        room_num = temp;
      }
      else if (temp != LL + L * 10 && temp != LL * 10 + L)
      {
        printf("分支路段与主干路段对不上");
        return 0;
      }
      else
      {
        printf("视觉未识别到数字,等待识别正确");
        return 100; // 意思是等待识别
      }
    }

    if (1) // 第一个十字路口右转，标志位自己传，我躺了
    {
      if (temp % 10 == 9) // 识别到一个数字在左侧
      {
        if (temp / 10 == R)
        {
          room_num = R * 10 + RR;
        }
        else if (temp / 10 == RR)
        {
          room_num = RR * 10 + R;
        }
        else
        {
          printf("分支路段与主干路段对不上");
          return 0;
        }
      }
      else if (temp / 10 == 9)
      {
        if (temp / 10 == RR)
        {
          room_num = R * 10 + RR;
        }
        else if (temp / 10 == R)
        {
          room_num = RR * 10 + R;
        }
        else
        {
          printf("分支路段与主干路段对不上");
          return 0;
        }
      }
      else if (temp == RR + R * 10 || temp == RR * 10 + R)
      {
        room_num = temp;
      }
      else if (temp != RR + R * 10 && temp != RR * 10 + R)
      {
        printf("分支路段与主干路段对不上");
        return 0;
      }
      else
      {
        printf("视觉未识别到数字,等待识别正确");
        return 100; // 意思是等待识别
      }
    }
  }
  
  printf("未进入相应处理程序");
  return 0;
}
