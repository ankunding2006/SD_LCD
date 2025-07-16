#include "main.h"
#include "control.h"
#include "task.h"
#include "gray_detection.h"

/*
  此处说明比赛规则：八个病房，能循迹直角弯到达对应病房执行任务以及返回起点：

  （1）视觉只做初步 主要靠循迹判断+写死
    需要做到，根据视觉传来的数字走到对应病房，执行（亮灯）任务后，原路返回
    原路返回逻辑可以借用栈存储的原理，记录来时路，然后返回弹栈，正好一个来回
  （2）全使用视觉作为决策逻辑，循迹
    需要做到，视觉识别到数字，然后给指令发车，到十字路口给决策左转右转还是直行

  我觉得用循迹和视觉放一起更加合适，视觉可以判断数字，也可以判断当前路段是否为十字路口
  但是也是写死，纯活判断的话短时间写不完的肯定，十字路口判断逻辑就循迹和视觉一起用，视觉辅助不能跑飞啥的
  串口接收也是有福了，得写点东西出来处理这些情况，以及循迹也有完整的控制逻辑，两套逻辑协同处理也麻烦

  大概做做来说，不如就循迹写死，这个效率比较高我觉得。
 */

/**
 * @brief  定时器周期溢出回调函数
 * @param  htim: 定时器句柄
 * @retval 无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM12)
  {
    // TODO: 定时器周期溢出回调函数
  }
}

/**
 * @brief 通过状态机实现小车行驶到某个"病房"
 * @note:地图介绍
 * @param 病房编号
 * @retval 1:完成 0:进行中
 */
void Car_To_Room(uint8_t room_num)
{
  // 状态机
}

/**
 * @brief 控制小车行驶到第X个十字路口(X=1,2,3)
 * @param 十字路口编号(从1开始计数),中心的道路上上总共有3个路口从下至上分别即为1,2,3
 * @retval 1:完成 0:进行中
 */
uint8_t Car_To_Crossing(uint8_t crossing_num)
{
  static uint8_t crossings_found = 0;
  static uint8_t is_on_crossing = 0;
  static uint8_t last_target = 0;

  // 复位逻辑：如果目标改变，重置状态。
  if (crossing_num != last_target)
  {
    crossings_found = 0;
    is_on_crossing = 0;
    last_target = crossing_num;
    if (crossing_num == 0)
    {                             // 调用参数为0可以作为停止/复位的信号
      set_motor_speed(0, 0, 252); // 停车
      return 1;                   // 表示“已完成”
    }
  }

  // 如果没有设置目标，则不执行任何操作。
  if (crossing_num == 0)
  {
    return 1;
  }

  // 如果任务已经完成，保持小车停止并返回1。
  if (crossings_found >= crossing_num)
  {
    set_motor_speed(0, 0, 252);
    return 1;
  }

  grey_sensor_Read();
  int turn_value = Calculate_Turn_Value();

  if (turn_value == INT16_MAX)
  { // 检测到潜在的十字路口
    if (!is_on_crossing)
    {
      is_on_crossing = 1;
      crossings_found++;

      if (crossings_found >= crossing_num)
      {
        set_motor_speed(0, 0, 252);
        return 1; // 到达目标十字路口
      }
    }
    // 如果在十字路口但不是目标十字路口，直行通过。
    set_motor_speed(CAR_BASE_SPEED, CAR_BASE_SPEED, 252);
  }
  else if (turn_value == INT16_MIN)
  {                             // 丢失循迹线
    set_motor_speed(0, 0, 252); // 停车
    is_on_crossing = 0;         // 不再位于十字路口上
  }
  else
  {                     // 正常循迹
    is_on_crossing = 0; // 回到单线后清除十字路口标志。

    int16_t left_speed = CAR_BASE_SPEED - turn_value;
    int16_t right_speed = CAR_BASE_SPEED + turn_value;

    // 将电机速度限制在有效范围内
    if (left_speed > CAR_MAX_SPEED)
      left_speed = CAR_MAX_SPEED;
    if (left_speed < 0)
      left_speed = 0;
    if (right_speed > CAR_MAX_SPEED)
      right_speed = CAR_MAX_SPEED;
    if (right_speed < 0)
      right_speed = 0;

    set_motor_speed(left_speed, right_speed, 252);
  }

  return 0; // 任务进行中
}
