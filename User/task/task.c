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
    visual_process_command();
  }
}

/**
 * @brief 通过状态机实现小车行驶到1,2号"病房"
 * @note:地图介绍：1,2号病房分别在第一个路口的左右两侧,小车需要从药房出发,先循迹直行到达
 * 第一个路口,然后左转或右转,到达1,2号病房,然后等待取药(具体函数待完成),取药完成后,通过
 * open_loop_steering_control旋转180度,然后循迹直行到路口,然后右转或左转,再直行到达药房,
 * 任务结束
 * @param 病房编号(1,2)
 * @retval 1:完成 0:进行中
 */
int Car_To_Room_1_2(uint8_t room_num)
{
  // 定义状态机的各个状态
  typedef enum
  {
    CAR_STATE_INIT,               // 初始状态
    GOING_TO_CROSSING_1,          // 前往第一个路口
    TURNING_AT_CROSSING_1,        // 在第一个路口转向
    CHECK_TURN_1_COMPLETE,        // 检查转向是否完成
    GOING_TO_ROOM,                // 前往病房
    AT_ROOM,                      // 到达病房，等待卸货
    TURNING_180_AT_ROOM,          // 在病房门口180度掉头
    CHECK_TURN_180_COMPLETE,      // 检查180度掉头是否完成
    RETURNING_TO_CROSSING_1,      // 返回第一个路口
    TURNING_AT_CROSSING_1_RETURN, // 在路口转向药房方向
    CHECK_TURN_RETURN_COMPLETE,   // 检查返回转向是否完成
    RETURNING_TO_PHARMACY,        // 返回药房
    TASK_COMPLETE                 // 任务完成
  } CarState_t;

  static CarState_t car_state = CAR_STATE_INIT;
  static uint8_t target_room = 0;

  // 如果目标病房改变，或不是1/2号房，则重置状态机
  if (room_num != target_room)
  {
    car_state = CAR_STATE_INIT;
    target_room = room_num;
    Car_To_Crossing(0); // 通过传递一个不同的目标值来重置Car_To_Crossing函数的状态
  }

  // 如果不是1号或2号病房，则停车并返回
  if (room_num != 1 && room_num != 2)
  {
    set_motor_speed(0, 0, 252);
    return 0;
  }

  switch (car_state)
  {
  case CAR_STATE_INIT:
    car_state = GOING_TO_CROSSING_1;
    // 直接进入下一个状态
  case GOING_TO_CROSSING_1:
    if (Car_To_Crossing(1)) // 行驶到第一个路口
    {
      car_state = TURNING_AT_CROSSING_1;
    }
    break;

  case TURNING_AT_CROSSING_1:
  {
    // 1号病房左转，2号病房右转。
    // TODO:转向时间需要调试
    int16_t turn_duration_ms = (target_room == 1) ? 500 : -500; // 500ms -> 90度
    open_loop_steering_control(turn_duration_ms, 150, 0);       // 启动转向
    car_state = CHECK_TURN_1_COMPLETE;
    break;
  }

  case CHECK_TURN_1_COMPLETE:
    car_state = GOING_TO_ROOM; // 转向完成，进入下一个状态
    break;

  case GOING_TO_ROOM:
  {
    if (Car_To_Crossing(1)) // 行驶到病房
    {
      car_state = AT_ROOM;
    }
    break;
  }

  case AT_ROOM:
    // TODO: 点亮红色指示灯
    // TODO:等待卸货
    // TODO: 熄灭红色指示灯
    car_state = TURNING_180_AT_ROOM;
    break;

  case TURNING_180_AT_ROOM:
    // 180度掉头，转向时间需要调试
    // TODO:转向时间需要调试
    if (open_loop_steering_control(1000, 150, 0)) // 1000ms -> 180度
    {
      car_state = CHECK_TURN_180_COMPLETE;
    }
    break;

  case CHECK_TURN_180_COMPLETE:
    car_state = RETURNING_TO_CROSSING_1;
    break;

  case RETURNING_TO_CROSSING_1:
    if (Car_To_Crossing(1)) // 返回至路口
    {
      car_state = TURNING_AT_CROSSING_1_RETURN;
    }
    break;

  case TURNING_AT_CROSSING_1_RETURN:
  {
    // 返回时，从1号病房回来需要右转，从2号病房回来需要左转
    int16_t turn_duration_ms = (target_room == 1) ? -500 : 500;
    open_loop_steering_control(turn_duration_ms, 150, 0);
    car_state = CHECK_TURN_RETURN_COMPLETE;
    break;
  }

  case CHECK_TURN_RETURN_COMPLETE:
    car_state = RETURNING_TO_PHARMACY;
    break;

  case RETURNING_TO_PHARMACY:
    if (Car_To_Crossing(1)) // 返回药房
    {
      car_state = TASK_COMPLETE;
    }
    break;

  case TASK_COMPLETE:
    set_motor_speed(0, 0, 252); // 任务完成，停车
    // TODO: 点亮绿色指示灯
    car_state = CAR_STATE_INIT; // 复位状态机以便下次调用
    target_room = 0;
    break;
  }
  return 0;
}

/**
 * @brief 通过状态机实现小车行驶到3,4号"病房"
 * @note:地图介绍：3,4号病房分别在第二个路口的左右两侧,小车需要从药房出发,先循迹直行到达
 * 第二个路口,再通过读取room_num这个变量获取数字的排布(3,4病房的编号左右并非固定)
 * 如果3,4号病房在第二个路口的左侧,则room_num的值为34,如果3,4号病房在第二个路口的右侧,
 * 则room_num的值为43,根据病房编号然后左转或右转,到达3,4号病房,然后等待取药(具
 * 体函数待完成),取药完成后,通过open_loop_steering_control旋转180度,然后循迹直行到路口,
 * 然后右转或左转,再直行通过两个路口,到达药房,任务结束
 * @param 病房编号(3,4)
 * @retval 1:完成 0:进行中
 */
int Car_To_Room_3_4(uint8_t room_num)
{
  // 定义状态机的各个状态
  typedef enum
  {
    CAR_STATE_INIT,               // 初始状态
    GOING_TO_CROSSING_2,          // 前往第二个路口
    TURNING_AT_CROSSING_2,        // 在第二个路口转向
    CHECK_TURN_2_COMPLETE,        // 检查转向是否完成
    GOING_TO_ROOM,                // 前往病房
    AT_ROOM,                      // 到达病房，等待卸货
    TURNING_180_AT_ROOM,          // 在病房门口180度掉头
    CHECK_TURN_180_COMPLETE,      // 检查180度掉头是否完成
    RETURNING_TO_CROSSING_2,      // 返回第二个路口
    TURNING_AT_CROSSING_2_RETURN, // 在路口转向药房方向
    CHECK_TURN_RETURN_COMPLETE,   // 检查返回转向是否完成
    RETURNING_TO_PHARMACY,        // 返回药房
    TASK_COMPLETE                 // 任务完成
  } CarState_t;

  static CarState_t car_state = CAR_STATE_INIT;
  static uint8_t target_room = 0;
  static uint8_t room_layout_3_4 = 34; // 假设默认3号在左，4号在右。34代表左3右4, 43代表左4右3

  // 如果目标病房改变，则重置状态机
  if (room_num != target_room)
  {
    car_state = CAR_STATE_INIT;
    target_room = room_num;
  }

  // 如果不是3号或4号病房，则停车并返回
  if (target_room != 3 && target_room != 4)
  {
    set_motor_speed(0, 0, 252);
    return 0;
  }

  switch (car_state)
  {
  case CAR_STATE_INIT:
    car_state = GOING_TO_CROSSING_2;
    // intentional fall-through

  case GOING_TO_CROSSING_2:
    if (Car_To_Crossing(2)) // 行驶到第二个路口
    {
      car_state = TURNING_AT_CROSSING_2;
    }
    break;

  case TURNING_AT_CROSSING_2:
  {
    // 根据传入的room_num进行初始化或设置
    // 如果传入34或43，则认为是设置病房布局
    if (room_num == 34 || room_num == 43)
    {
      room_layout_3_4 = room_num;
    }
    // 3号在左(34)，目标3，左转。 3号在右(43)，目标3，右转
    // 4号在右(34)，目标4，右转。 4号在左(43)，目标4，左转
    int16_t turn_duration_ms = 500; // 默认左转
    uint8_t is_left_turn = (room_layout_3_4 == 34 && target_room == 3) || (room_layout_3_4 == 43 && target_room == 4);

    if (!is_left_turn)
    {
      turn_duration_ms = -500; // 右转
    }

    open_loop_steering_control(turn_duration_ms, 150, 0);
    car_state = CHECK_TURN_2_COMPLETE;
    break;
  }

  case CHECK_TURN_2_COMPLETE:
    if (open_loop_steering_control(0, 0, 0))
    {
      car_state = GOING_TO_ROOM;
    }
    break;

  case GOING_TO_ROOM:
    if (Car_To_Crossing(1)) // 从路口行驶到病房门口
    {
      car_state = AT_ROOM;
    }
    break;

  case AT_ROOM:
    // TODO: 点亮红色指示灯
    // TODO: 等待卸货
    // TODO: 熄灭红色指示灯
    car_state = TURNING_180_AT_ROOM;
    break;

  case TURNING_180_AT_ROOM:
    // TODO: 180度掉头时间需要调试
    open_loop_steering_control(1000, 150, 0);
    car_state = CHECK_TURN_180_COMPLETE;
    break;

  case CHECK_TURN_180_COMPLETE:
    if (open_loop_steering_control(0, 0, 0))
    {
      car_state = RETURNING_TO_CROSSING_2;
    }
    break;

  case RETURNING_TO_CROSSING_2:
    if (Car_To_Crossing(1)) // 从病房返回到路口
    {
      car_state = TURNING_AT_CROSSING_2_RETURN;
    }
    break;

  case TURNING_AT_CROSSING_2_RETURN:
  {
    // 返回时转向方向相反
    int16_t turn_duration_ms = -500; // 默认右转
    uint8_t is_left_turn = (room_layout_3_4 == 34 && target_room == 3) || (room_layout_3_4 == 43 && target_room == 4);

    if (!is_left_turn)
    {
      turn_duration_ms = 500; // 左转
    }

    open_loop_steering_control(turn_duration_ms, 150, 0);
    car_state = CHECK_TURN_RETURN_COMPLETE;
    break;
  }

  case CHECK_TURN_RETURN_COMPLETE:
    if (open_loop_steering_control(0, 0, 0))
    {
      car_state = RETURNING_TO_PHARMACY;
    }
    break;

  case RETURNING_TO_PHARMACY:
    if (Car_To_Crossing(2)) // 从第二个路口返回药房
    {
      car_state = TASK_COMPLETE;
    }
    break;

  case TASK_COMPLETE:
    set_motor_speed(0, 0, 252);
    // TODO: 点亮绿色指示灯
    target_room = 0; // 重置目标，准备下次任务
    car_state = CAR_STATE_INIT;
    return 1; // 任务完成
  }

  return 0; // 任务进行中
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
  static uint8_t task_completed = 0; // 新增：任务完成标志

  // 复位逻辑：如果目标路口编号改变，则重置整个状态机
  if (crossing_num != last_target)
  {
    crossings_found = 0;
    is_on_crossing = 0;
    last_target = crossing_num;
    task_completed = 0;
  }

  // 如果任务已经完成，保持小车停止并返回1
  if (task_completed)
  {
    set_motor_speed(0, 0, 252);
    return 1;
  }

  // crossings_found 和 is_on_crossing 状态更新逻辑
  // 检测到4个或更多传感器，判定为到达了十字路口区域
  static uint32_t pre_time = 0;
  uint64_t now_time = HAL_GetTick();
  if (Calculate_Turn_Value() == INT16_MAX)
  {
    // 通过时间间隔判断是否为新进入一个路口，防止重复计数
    if (now_time - pre_time > 1000)
    {
      crossings_found++;
    }
    is_on_crossing = 1;
    pre_time = now_time;
  }
  else
  {
    // 如果之前在路口上，并且现在离开了，则更新is_on_crossing状态
    if (is_on_crossing && (now_time - pre_time > 500)) // 离开路口0.5秒后
    {
      is_on_crossing = 0;
    }
  }

  // 任务完成判断
  if (crossings_found >= crossing_num && is_on_crossing)
  {
    set_motor_speed(0, 0, 252); // 到达目标，停车
    task_completed = 1;         // 标记任务完成
    return 1;                   // 返回完成状态
  }
  // 任务自动重置逻辑：如果任务已完成且小车已离开路口，则重置计数器以备下次调用
  else if (task_completed && !is_on_crossing)
  {
    crossings_found = 0;
    task_completed = 0;
  }
  else
  {
    line_following_task(); // 任务未完成，继续循迹
  }

  return 0; // 任务进行中
}
