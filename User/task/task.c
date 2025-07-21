#include "main.h"
#include "control.h"
#include "task.h"
#include "test.h"
#include "gray_detection.h"
#include "tim.h"
#include "car_config.h"
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
#if TEST_IN_INTERRUPT == 1
    test_in_interrupt();
#endif
  }
}

/**
 * @brief 通过状态机实现小车行驶到X号"病房"(X=1,2,3,4,5,6,7,8)
 * @param 病房编号(1,2,3,4,5,6,7,8)
 * @retval 1:完成 0:进行中
 */
uint8_t Car_To_Room(void)
{
  if (room_target >= 1 && room_target <= 2)
  {
    return Car_To_Room_1_2(room_target);
  }
  else if (room_target >= 3 && room_target <= 4)
  {
    return Car_To_Room_3_4(room_target);
  }
  else if (room_target >= 5 && room_target <= 8)
  {
    return Car_To_Room_5_8(room_target);
  }

  // 如果传入无效的病房编号，则停止
  set_motor_speed(0, 0, DEFAULT_ACCELERATION);
  return 0;
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
uint8_t Car_To_Room_1_2(uint8_t room_num)
{
  // 定义状态机的各个状态
  typedef enum
  {
    CAR_STATE_INIT,               // 初始状态
    GOING_TO_CROSSING_1,          // 前往第一个路口
    TURNING_AT_CROSSING_1,        // 在第一个路口转向
    GOING_TO_ROOM,                // 前往病房
    AT_ROOM,                      // 到达病房，等待卸货
    TURNING_180_AT_ROOM,          // 在病房门口180度掉头
    RETURNING_TO_CROSSING_1,      // 返回第一个路口
    TURNING_AT_CROSSING_1_RETURN, // 在路口转向药房方向
    RETURNING_TO_PHARMACY,        // 返回药房
    TASK_COMPLETE                 // 任务完成
  } CarState_t;

  static CarState_t car_state = CAR_STATE_INIT;
  static uint8_t target_room_local = 0;

  // 如果目标病房改变，或不是1/2号房，则重置状态机
  if (room_num != target_room_local)
  {
    car_state = CAR_STATE_INIT;
    target_room_local = room_num;
    Car_To_Crossing(0); // 通过传递一个不同的目标值来重置Car_To_Crossing函数的状态
  }

  // 如果不是1号或2号病房，则停车并返回
  if (room_num != 1 && room_num != 2)
  {
    set_motor_speed(0, 0, DEFAULT_ACCELERATION);
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
    int16_t turn_duration_ms = (target_room_local == 1) ? TURN_90_DURATION_MS : -TURN_90_DURATION_MS;
    if (open_loop_steering_control(turn_duration_ms, TURN_90_SPEED, TURN_90_BASE_SPEED))
    {
      car_state = GOING_TO_ROOM; // 转向完成，进入下一个状态
    }
    break;
  }

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
    // 180度掉头
    if (open_loop_steering_control(TURN_180_DURATION_MS, TURN_180_SPEED, TURN_180_BASE_SPEED))
    {
      car_state = RETURNING_TO_CROSSING_1;
    }
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
    int16_t turn_duration_ms = (target_room_local == 1) ? -TURN_90_DURATION_MS : TURN_90_DURATION_MS;
    if (open_loop_steering_control(turn_duration_ms, TURN_90_SPEED, TURN_90_BASE_SPEED))
    {
      car_state = RETURNING_TO_PHARMACY;
    }
    break;
  }

  case RETURNING_TO_PHARMACY:
    if (Car_To_Crossing(1)) // 返回药房
    {
      car_state = TASK_COMPLETE;
    }
    break;

  case TASK_COMPLETE:
    set_motor_speed(0, 0, DEFAULT_ACCELERATION); // 任务完成，停车
    // TODO: 点亮绿色指示灯
    car_state = CAR_STATE_INIT; // 复位状态机以便下次调用
    target_room_local = 0;
    return 1; // 任务完成
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
uint8_t Car_To_Room_3_4(uint8_t room_num)
{
  // 定义状态机的各个状态
  typedef enum
  {
    CAR_STATE_INIT,               // 初始状态
    GOING_TO_CROSSING_2,          // 前往第二个路口
    TURNING_AT_CROSSING_2,        // 在第二个路口转向
    GOING_TO_ROOM,                // 前往病房
    AT_ROOM,                      // 到达病房，等待卸货
    TURNING_180_AT_ROOM,          // 在病房门口180度掉头
    RETURNING_TO_CROSSING_2,      // 返回第二个路口
    TURNING_AT_CROSSING_2_RETURN, // 在路口转向药房方向
    RETURNING_TO_PHARMACY,        // 返回药房
    TASK_COMPLETE                 // 任务完成
  } CarState_t;

  static CarState_t car_state = CAR_STATE_INIT;
  static uint8_t target_room_local = 0;
  static uint8_t room_layout_3_4 = 34; // 假设默认3号在左，4号在右。34代表左3右4, 43代表左4右3

  // 如果目标病房改变，则重置状态机
  if (room_num != target_room_local)
  {
    car_state = CAR_STATE_INIT;
    target_room_local = room_num;
    // 注意：3、4号房的任务重置不应清除路口计数，因为它依赖于连续的路口计数
  }

  // 如果不是3号或4号病房，则停车并返回
  if (target_room_local != 3 && target_room_local != 4)
  {
    set_motor_speed(0, 0, DEFAULT_ACCELERATION);
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
    // 根据全局变量 room_num (由视觉更新)来决定布局
    if (room_num == 34 || room_num == 43)
    {
      room_layout_3_4 = room_num;
    }
    // 3号在左(34)，目标3，左转。 3号在右(43)，目标3，右转
    // 4号在右(34)，目标4，右转。 4号在左(43)，目标4，左转
    int16_t turn_duration_ms = TURN_90_DURATION_MS; // 默认左转
    uint8_t is_left_turn = (room_layout_3_4 == 34 && target_room_local == 3) || (room_layout_3_4 == 43 && target_room_local == 4);

    if (!is_left_turn)
    {
      turn_duration_ms = -TURN_90_DURATION_MS; // 右转
    }

    if (open_loop_steering_control(turn_duration_ms, TURN_90_SPEED, TURN_90_BASE_SPEED))
    {
      car_state = GOING_TO_ROOM;
    }
    break;
  }

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
    if (open_loop_steering_control(TURN_180_DURATION_MS, TURN_180_SPEED, TURN_180_BASE_SPEED))
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
    int16_t turn_duration_ms = -TURN_90_DURATION_MS; // 默认右转
    uint8_t is_left_turn = (room_layout_3_4 == 34 && target_room_local == 3) || (room_layout_3_4 == 43 && target_room_local == 4);

    if (!is_left_turn)
    {
      turn_duration_ms = TURN_90_DURATION_MS; // 左转
    }

    if (open_loop_steering_control(turn_duration_ms, TURN_90_SPEED, TURN_90_BASE_SPEED))
    {
      car_state = RETURNING_TO_PHARMACY;
    }
    break;
  }

  case RETURNING_TO_PHARMACY:
    if (Car_To_Crossing(2)) // 从第二个路口返回药房
    {
      car_state = TASK_COMPLETE;
    }
    break;

  case TASK_COMPLETE:
    set_motor_speed(0, 0, DEFAULT_ACCELERATION);
    // TODO: 点亮绿色指示灯
    target_room_local = 0; // 重置目标，准备下次任务
    car_state = CAR_STATE_INIT;
    return 1; // 任务完成
  }

  return 0; // 任务进行中
}

/**
 * @brief 通过状态机实现小车行驶到5,6,7,8号"病房"
 * @note 地图介绍：5,6,7,8号病房位于第三个十字路口的两侧。
 *       该函数依赖视觉系统在到达第三路口前更新全局变量 `room_num` 为一个四位数，
 *       例如 `5678`，表示左侧是5、6号病房，右侧是7、8号病房。
 *       整个过程无需停车等待视觉识别。
 * @param room_num 目标病房编号(5,6,7,8)
 * @retval 1:任务完成 0:进行中
 */
uint8_t Car_To_Room_5_8(uint8_t room_num)
{
  // 定义状态机的各个状态
  typedef enum
  {
    CAR_STATE_INIT,          // 初始状态
    GOING_TO_CROSSING_3,     // 前往第三个路口
    TURNING_AT_CROSSING_3,   // 在第三个路口转向
    GOING_TO_T_JUNCTION,     // 前往T字路口
    MAKING_FINAL_TURN,       // 在T字路口做最终转向
    GOING_TO_ROOM_FINAL,     // 前往最终病房
    AT_ROOM,                 // 到达病房，等待卸货
    TURNING_180_AT_ROOM,     // 在病房门口180度掉头
    RETURNING_TO_T_JUNCTION, // 返回T字路口
    MAKING_RETURN_TURN_1,    // 在T字路口转向，返回第三路口
    RETURNING_TO_CROSSING_3, // 返回第三个路口
    MAKING_RETURN_TURN_2,    // 在第三路口转向，返回药房
    RETURNING_TO_PHARMACY,   // 返回药房
    TASK_COMPLETE            // 任务完成
  } CarState_t;

  static CarState_t car_state = CAR_STATE_INIT;
  static uint8_t target_room_local = 0;
  static uint16_t layout_at_crossing_3 = 0; // 保存到达第三路口时的布局
  static uint8_t is_initial_turn_left = 0;  // 记录在第三路口的转向方向

  // 如果目标病房改变，则重置状态机
  if (room_num != target_room_local)
  {
    car_state = CAR_STATE_INIT;
    target_room_local = room_num;
    Car_To_Crossing(0); // 重置路口计数器
  }

  // 如果不是5,6,7,8号病房，则停车并返回
  if (target_room_local < 5 || target_room_local > 8)
  {
    set_motor_speed(0, 0, DEFAULT_ACCELERATION);
    return 0;
  }

  switch (car_state)
  {
  case CAR_STATE_INIT:
    car_state = GOING_TO_CROSSING_3;
    // intentional fall-through

  case GOING_TO_CROSSING_3:
    if (Car_To_Crossing(3))
    {
      // 到达时，使用视觉系统在途中更新的全局布局变量
      layout_at_crossing_3 = room_num;
      car_state = TURNING_AT_CROSSING_3;
    }
    break;

  case TURNING_AT_CROSSING_3:
  {
    // 判断目标是在左侧区域还是右侧区域
    uint16_t left_rooms = layout_at_crossing_3 / 100;
    is_initial_turn_left = (target_room_local == (left_rooms / 10) || target_room_local == (left_rooms % 10));

    int16_t turn_duration_ms = is_initial_turn_left ? TURN_90_DURATION_MS : -TURN_90_DURATION_MS; // 左转或右转
    if (open_loop_steering_control(turn_duration_ms, TURN_90_SPEED, TURN_90_BASE_SPEED))
    {
      car_state = GOING_TO_T_JUNCTION;
    }
    break;
  }

  case GOING_TO_T_JUNCTION:
    if (Car_To_Crossing(1)) // 到达T字路口
    {
      car_state = MAKING_FINAL_TURN;
    }
    break;

  case MAKING_FINAL_TURN:
  {
    uint16_t area_layout = is_initial_turn_left ? (layout_at_crossing_3 / 100) : (layout_at_crossing_3 % 100);
    uint8_t is_final_turn_left = (target_room_local == (area_layout / 10));

    int16_t turn_duration_ms = is_final_turn_left ? TURN_90_DURATION_MS : -TURN_90_DURATION_MS; // 左转或右转
    if (open_loop_steering_control(turn_duration_ms, TURN_90_SPEED, TURN_90_BASE_SPEED))
    {
      car_state = GOING_TO_ROOM_FINAL;
    }
    break;
  }

  case GOING_TO_ROOM_FINAL:
    if (Car_To_Crossing(1)) // 到达病房门口
    {
      car_state = AT_ROOM;
    }
    break;

  case AT_ROOM:
    // TODO: 点亮红色指示灯, 等待卸货
    car_state = TURNING_180_AT_ROOM;
    break;

  case TURNING_180_AT_ROOM:
    if (open_loop_steering_control(TURN_180_DURATION_MS, TURN_180_SPEED, TURN_180_BASE_SPEED)) // 180度掉头
    {
      car_state = RETURNING_TO_T_JUNCTION;
    }
    break;

  case RETURNING_TO_T_JUNCTION:
    if (Car_To_Crossing(1))
    {
      car_state = MAKING_RETURN_TURN_1;
    }
    break;

  case MAKING_RETURN_TURN_1:
  {
    uint16_t area_layout = is_initial_turn_left ? (layout_at_crossing_3 / 100) : (layout_at_crossing_3 % 100);
    uint8_t is_final_turn_left = (target_room_local == (area_layout / 10));
    int16_t turn_duration_ms = is_final_turn_left ? -TURN_90_DURATION_MS : TURN_90_DURATION_MS; // 返程时转向相反
    if (open_loop_steering_control(turn_duration_ms, TURN_90_SPEED, TURN_90_BASE_SPEED))
    {
      car_state = RETURNING_TO_CROSSING_3;
    }
    break;
  }

  case RETURNING_TO_CROSSING_3:
    if (Car_To_Crossing(1))
    {
      car_state = MAKING_RETURN_TURN_2;
    }
    break;

  case MAKING_RETURN_TURN_2:
  {
    int16_t turn_duration_ms = is_initial_turn_left ? -TURN_90_DURATION_MS : TURN_90_DURATION_MS; // 返程时转向相反
    if (open_loop_steering_control(turn_duration_ms, TURN_90_SPEED, TURN_90_BASE_SPEED))
    {
      car_state = RETURNING_TO_PHARMACY;
    }
    break;
  }

  case RETURNING_TO_PHARMACY:
    if (Car_To_Crossing(3)) // 返回药房
    {
      car_state = TASK_COMPLETE;
    }
    break;

  case TASK_COMPLETE:
    set_motor_speed(0, 0, DEFAULT_ACCELERATION);
    // TODO: 点亮绿色指示灯
    target_room_local = 0; // 重置目标，准备下次任务
    car_state = CAR_STATE_INIT;
    return 1; // 任务完成
  }

  return 0; // 任务进行中
}

/**
 * @brief 控制小车行驶到第X个十字路口(X=1,2,3)
 * @note 该函数为自重置函数。当任务完成并返回1后，其内部状态会自动重置，
 *       下次使用相同参数调用时会重新执行任务。
 * @param crossing_num 目标十字路口编号(从1开始计数)。传入0会使函数保持空闲并停车。
 * @retval 1:到达目标路口并停车 0:任务进行中或处于空闲状态
 */
uint8_t Car_To_Crossing(uint8_t crossing_num)
{
  typedef enum
  {
    STATE_IDLE,   // 空闲状态
    STATE_RUNNING // 运行状态
  } State_t;

  static State_t state = STATE_IDLE;
  static uint8_t crossings_found = 0;           // 已找到的路口数量
  static uint8_t is_on_crossing = 0;            // 是否在路口上
  static uint32_t last_crossing_enter_time = 0; // 上次进入路口的时间
  static uint8_t target_crossing_num = 0;       // 目标路口编号

  if (state == STATE_IDLE)
  {
    if (crossing_num > 0)
    {
      // 收到新的任务指令
      state = STATE_RUNNING;
      target_crossing_num = crossing_num;
      crossings_found = 0;
      is_on_crossing = 0;
      last_crossing_enter_time = 0; // 重置时间以允许立即检测第一个路口
    }
    else
    {
      // crossing_num为0或无效，保持空闲状态并确保小车停止
      set_motor_speed(0, 0, DEFAULT_ACCELERATION);
      return 0;
    }
  }

  // 如果处于运行状态：
  uint32_t now = HAL_GetTick();

  // --- 路口检测逻辑 ---
  if (Calculate_Turn_Value() == INT16_MAX)
  { // 检测到路口传感器信号
    if (!is_on_crossing)
    { // 表示刚进入路口区域
      is_on_crossing = 1;
      // 防抖：只有当距离上次进入路口有一定时间间隔时才计数
      if (last_crossing_enter_time == 0 || now - last_crossing_enter_time > CROSSING_DEBOUNCE_MS)
      {
        crossings_found++;
        last_crossing_enter_time = now;
      }
    }
  }
  else
  {
    // 不再检测到路口线
    is_on_crossing = 0;
  }

  // --- 任务完成判断逻辑 ---
  if (crossings_found >= target_crossing_num && is_on_crossing)
  {
    // 到达目标位置，停车
    set_motor_speed(0, 0, DEFAULT_ACCELERATION);
    // 为下次运行重置状态
    state = STATE_IDLE;
    return 1; // 任务完成！
  }

  // --- 运动控制逻辑 ---
  line_following_task();
  return 0; // 任务进行中
}
