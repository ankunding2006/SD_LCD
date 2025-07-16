#include "main.h"
#include "control.h"
#include "task.h"
#include "gray_detection.h"
#include "tim.h"
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
 * @brief 控制小车行驶到第X个十字路口(X=1,2,3,4,5) 4为最上面左侧，5为最上面右侧
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
  }

  // 任务控制逻辑
  // 如果任务已经完成，保持小车停止并返回1
  if (crossings_found >= crossing_num && is_on_crossing)
  {
    set_motor_speed(0, 0, 252);
    return 1;
  }
  // 此段为crossing_nums = 0 的情况，这个是到达药房返回的逻辑
  else if (!crossing_num)
  {
    //返回起点
  }
  else
  {
    line_following_task(); // 循迹 脱线会停车
  }

  // crossings_found 和 is_on_crossing 逻辑
  // 检测到6个或更多传感器，判定为到达了直角弯区域，让直角弯计数加一
  static uint32_t pre_time = 0;      // (ms)
  uint64_t now_time = HAL_GetTick(); // 两个时间进行处理
  if (Calculate_Turn_Value() == INT16_MAX)
  {
    // 间隔较长视为进一次直角弯 0-1也是间隔较长 无bug
    if (now_time - pre_time > 1000) 
    {
      crossings_found++;
    }
    is_on_crossing = 1;
    pre_time = now_time;
  }
  else
  {
    if (now_time - pre_time > 1000)
    {
      is_on_crossing = 0;
    }
  }

  return 0; // 任务进行中
}
