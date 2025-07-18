#include "Emm_V5.h"
#include <stdbool.h>

/**********************************************************
***	Emm_V5.0步进闭环控制例程
***	编写作者：ZHANGDATOU
***	技术支持：张大头闭环伺服
***	淘宝店铺：https://zhangdatou.taobao.com
***	CSDN博客：http s://blog.csdn.net/zhangdatou666
***	qq交流群：262438510
**********************************************************/

extern UART_HandleTypeDef Emm_V5_HUART;
// 定义一个静态的发送缓冲区，以确保DMA安全访问
static uint8_t Emm_V5_TX_Buffer[32];

/**
 * @brief    将当前位置清零
 * @param    addr  ：电机地址
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Reset_CurPos_To_Zero(uint8_t addr)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr; // 地址
  Emm_V5_TX_Buffer[1] = 0x0A; // 功能码
  Emm_V5_TX_Buffer[2] = 0x6D; // 辅助码
  Emm_V5_TX_Buffer[3] = 0x6B; // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 4);
}

/**
 * @brief    解除堵转保护
 * @param    addr  ：电机地址
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Reset_Clog_Pro(uint8_t addr)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr; // 地址
  Emm_V5_TX_Buffer[1] = 0x0E; // 功能码
  Emm_V5_TX_Buffer[2] = 0x52; // 辅助码
  Emm_V5_TX_Buffer[3] = 0x6B; // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 4);
}

/**
 * @brief    读取系统参数
 * @param    addr  ：电机地址
 * @param    s     ：系统参数类型
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Read_Sys_Params(uint8_t addr, SysParams_t s)
{
  uint8_t i = 0;

  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[i] = addr;
  ++i; // 地址

  switch (s) // 功能码
  {
  case S_VER:
    Emm_V5_TX_Buffer[i] = 0x1F;
    ++i;
    break;
  case S_RL:
    Emm_V5_TX_Buffer[i] = 0x20;
    ++i;
    break;
  case S_PID:
    Emm_V5_TX_Buffer[i] = 0x21;
    ++i;
    break;
  case S_VBUS:
    Emm_V5_TX_Buffer[i] = 0x24;
    ++i;
    break;
  case S_CPHA:
    Emm_V5_TX_Buffer[i] = 0x27;
    ++i;
    break;
  case S_ENCL:
    Emm_V5_TX_Buffer[i] = 0x31;
    ++i;
    break;
  case S_TPOS:
    Emm_V5_TX_Buffer[i] = 0x33;
    ++i;
    break;
  case S_VEL:
    Emm_V5_TX_Buffer[i] = 0x35;
    ++i;
    break;
  case S_CPOS:
    Emm_V5_TX_Buffer[i] = 0x36;
    ++i;
    break;
  case S_PERR:
    Emm_V5_TX_Buffer[i] = 0x37;
    ++i;
    break;
  case S_FLAG:
    Emm_V5_TX_Buffer[i] = 0x3A;
    ++i;
    break;
  case S_ORG:
    Emm_V5_TX_Buffer[i] = 0x3B;
    ++i;
    break;
  case S_Conf:
    Emm_V5_TX_Buffer[i] = 0x42;
    ++i;
    Emm_V5_TX_Buffer[i] = 0x6C;
    ++i;
    break;
  case S_State:
    Emm_V5_TX_Buffer[i] = 0x43;
    ++i;
    Emm_V5_TX_Buffer[i] = 0x7A;
    ++i;
    break;
  default:
    return HAL_ERROR; // 无效的参数类型
  }

  Emm_V5_TX_Buffer[i] = 0x6B;
  ++i; // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, i);
}

/**
 * @brief    修改开环/闭环控制模式
 * @param    addr     ：电机地址
 * @param    svF      ：是否存储标志，false为不存储，true为存储
 * @param    ctrl_mode：控制模式（对应屏幕上的P_Pul菜单），0是关闭脉冲输入引脚，1是开环模式，2是闭环模式，3是让En端口复用为多圈限位开关输入引脚，Dir端口复用为到位输出高电平功能
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Modify_Ctrl_Mode(uint8_t addr, bool svF, uint8_t ctrl_mode)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr;      // 地址
  Emm_V5_TX_Buffer[1] = 0x46;      // 功能码
  Emm_V5_TX_Buffer[2] = 0x69;      // 辅助码
  Emm_V5_TX_Buffer[3] = svF;       // 是否存储标志，false为不存储，true为存储
  Emm_V5_TX_Buffer[4] = ctrl_mode; // 控制模式（对应屏幕上的P_Pul菜单），0是关闭脉冲输入引脚，1是开环模式，2是闭环模式，3是让En端口复用为多圈限位开关输入引脚，Dir端口复用为到位输出高电平功能
  Emm_V5_TX_Buffer[5] = 0x6B;      // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 6);
}

/**
 * @brief    使能信号控制
 * @param    addr  ：电机地址
 * @param    state ：使能状态     ，true为使能电机，false为关闭电机
 * @param    snF   ：多机同步标志 ，false为不启用，true为启用
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_En_Control(uint8_t addr, bool state, bool snF)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr;           // 地址
  Emm_V5_TX_Buffer[1] = 0xF3;           // 功能码
  Emm_V5_TX_Buffer[2] = 0xAB;           // 辅助码
  Emm_V5_TX_Buffer[3] = (uint8_t)state; // 使能状态
  Emm_V5_TX_Buffer[4] = snF;            // 多机同步运动标志
  Emm_V5_TX_Buffer[5] = 0x6B;           // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 6);
}

/**
 * @brief    速度模式
 * @param    addr：电机地址
 * @param    dir ：方向       ，0为CW，其余值为CCW
 * @param    vel ：速度       ，范围0 - 5000RPM
 * @param    acc ：加速度     ，范围0 - 255，注意：0是直接启动
 * @param    snF ：多机同步标志，false为不启用，true为启用
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Vel_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr;                // 地址
  Emm_V5_TX_Buffer[1] = 0xF6;                // 功能码
  Emm_V5_TX_Buffer[2] = dir;                 // 方向
  Emm_V5_TX_Buffer[3] = (uint8_t)(vel >> 8); // 速度(RPM)高8位字节
  Emm_V5_TX_Buffer[4] = (uint8_t)(vel >> 0); // 速度(RPM)低8位字节
  Emm_V5_TX_Buffer[5] = acc;                 // 加速度，注意：0是直接启动
  Emm_V5_TX_Buffer[6] = snF;                 // 多机同步运动标志
  Emm_V5_TX_Buffer[7] = 0x6B;                // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 8);
}

/**
 * @brief    位置模式
 * @param    addr：电机地址
 * @param    dir ：方向        ，0为CW，其余值为CCW
 * @param    vel ：速度(RPM)   ，范围0 - 5000RPM
 * @param    acc ：加速度      ，范围0 - 255，注意：0是直接启动
 * @param    clk ：脉冲数      ，范围0- (2^32 - 1)个
 * @param    raF ：相位/绝对标志，false为相对运动，true为绝对值运动
 * @param    snF ：多机同步标志 ，false为不启用，true为启用
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Pos_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, bool raF, bool snF)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr;                 // 地址
  Emm_V5_TX_Buffer[1] = 0xFD;                 // 功能码
  Emm_V5_TX_Buffer[2] = dir;                  // 方向
  Emm_V5_TX_Buffer[3] = (uint8_t)(vel >> 8);  // 速度(RPM)高8位字节
  Emm_V5_TX_Buffer[4] = (uint8_t)(vel >> 0);  // 速度(RPM)低8位字节
  Emm_V5_TX_Buffer[5] = acc;                  // 加速度，注意：0是直接启动
  Emm_V5_TX_Buffer[6] = (uint8_t)(clk >> 24); // 脉冲数(bit24 - bit31)
  Emm_V5_TX_Buffer[7] = (uint8_t)(clk >> 16); // 脉冲数(bit16 - bit23)
  Emm_V5_TX_Buffer[8] = (uint8_t)(clk >> 8);  // 脉冲数(bit8  - bit15)
  Emm_V5_TX_Buffer[9] = (uint8_t)(clk >> 0);  // 脉冲数(bit0  - bit7 )
  Emm_V5_TX_Buffer[10] = raF;                 // 相位/绝对标志
  Emm_V5_TX_Buffer[11] = snF;                 // 多机同步运动标志
  Emm_V5_TX_Buffer[12] = 0x6B;                // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 13);
}

/**
 * @brief    立即停止（所有控制模式都通用）
 * @param    addr  ：电机地址
 * @param    snF   ：多机同步标志，false为不启用，true为启用
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Stop_Now(uint8_t addr, bool snF)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr; // 地址
  Emm_V5_TX_Buffer[1] = 0xFE; // 功能码
  Emm_V5_TX_Buffer[2] = 0x98; // 辅助码
  Emm_V5_TX_Buffer[3] = snF;  // 多机同步运动标志
  Emm_V5_TX_Buffer[4] = 0x6B; // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 5);
}

/**
 * @brief    多机同步运动
 * @param    addr  ：电机地址
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Synchronous_motion(uint8_t addr)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr; // 地址
  Emm_V5_TX_Buffer[1] = 0xFF; // 功能码
  Emm_V5_TX_Buffer[2] = 0x66; // 辅助码
  Emm_V5_TX_Buffer[3] = 0x6B; // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 4);
}

/**
 * @brief    设置单圈回零的零点位置
 * @param    addr  ：电机地址
 * @param    svF   ：是否存储标志，false为不存储，true为存储
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Origin_Set_O(uint8_t addr, bool svF)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr; // 地址
  Emm_V5_TX_Buffer[1] = 0x93; // 功能码
  Emm_V5_TX_Buffer[2] = 0x88; // 辅助码
  Emm_V5_TX_Buffer[3] = svF;  // 是否存储标志
  Emm_V5_TX_Buffer[4] = 0x6B; // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 5);
}

/**
 * @brief    修改回零参数
 * @param    addr  ：电机地址
 * @param    svF   ：是否存储标志，false为不存储，true为存储
 * @param    o_mode ：回零模式，0为单圈就近回零，1为单圈方向回零，2为多圈无限位碰撞回零，3为多圈有限位开关回零
 * @param    o_dir  ：回零方向，0为CW，其余值为CCW
 * @param    o_vel  ：回零速度，单位：RPM（转/分钟）
 * @param    o_tm   ：回零超时时间，单位：毫秒
 * @param    sl_vel ：无限位碰撞回零检测转速，单位：RPM（转/分钟）
 * @param    sl_ma  ：无限位碰撞回零检测电流，单位：Ma（毫安）
 * @param    sl_ms  ：无限位碰撞回零检测时间，单位：Ms（毫秒）
 * @param    potF   ：上电自动触发回零，false为不使能，true为使能
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Origin_Modify_Params(uint8_t addr, bool svF, uint8_t o_mode, uint8_t o_dir, uint16_t o_vel, uint32_t o_tm, uint16_t sl_vel, uint16_t sl_ma, uint16_t sl_ms, bool potF)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr;                    // 地址
  Emm_V5_TX_Buffer[1] = 0x4C;                    // 功能码
  Emm_V5_TX_Buffer[2] = 0xAE;                    // 辅助码
  Emm_V5_TX_Buffer[3] = svF;                     // 是否存储标志
  Emm_V5_TX_Buffer[4] = o_mode;                  // 回零模式
  Emm_V5_TX_Buffer[5] = o_dir;                   // 回零方向
  Emm_V5_TX_Buffer[6] = (uint8_t)(o_vel >> 8);   // 回零速度(RPM)高8位字节
  Emm_V5_TX_Buffer[7] = (uint8_t)(o_vel >> 0);   // 回零速度(RPM)低8位字节
  Emm_V5_TX_Buffer[8] = (uint8_t)(o_tm >> 24);   // 回零超时时间(bit24 - bit31)
  Emm_V5_TX_Buffer[9] = (uint8_t)(o_tm >> 16);   // 回零超时时间(bit16 - bit23)
  Emm_V5_TX_Buffer[10] = (uint8_t)(o_tm >> 8);   // 回零超时时间(bit8  - bit15)
  Emm_V5_TX_Buffer[11] = (uint8_t)(o_tm >> 0);   // 回零超时时间(bit0  - bit7 )
  Emm_V5_TX_Buffer[12] = (uint8_t)(sl_vel >> 8); // 无限位碰撞回零检测转速(RPM)高8位字节
  Emm_V5_TX_Buffer[13] = (uint8_t)(sl_vel >> 0); // 无限位碰撞回零检测转速(RPM)低8位字节
  Emm_V5_TX_Buffer[14] = (uint8_t)(sl_ma >> 8);  // 无限位碰撞回零检测电流(Ma)高8位字节
  Emm_V5_TX_Buffer[15] = (uint8_t)(sl_ma >> 0);  // 无限位碰撞回零检测电流(Ma)低8位字节
  Emm_V5_TX_Buffer[16] = (uint8_t)(sl_ms >> 8);  // 无限位碰撞回零检测时间(Ms)高8位字节
  Emm_V5_TX_Buffer[17] = (uint8_t)(sl_ms >> 0);  // 无限位碰撞回零检测时间(Ms)低8位字节
  Emm_V5_TX_Buffer[18] = potF;                   // 上电自动触发回零
  Emm_V5_TX_Buffer[19] = 0x6B;                   // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 20);
}

/**
 * @brief    触发回零
 * @param    addr   ：电机地址
 * @param    o_mode ：回零模式，0为单圈就近回零，1为单圈方向回零，2为多圈无限位碰撞回零，3为多圈有限位开关回零
 * @param    snF   ：多机同步标志，false为不启用，true为启用
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Origin_Trigger_Return(uint8_t addr, uint8_t o_mode, bool snF)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr;   // 地址
  Emm_V5_TX_Buffer[1] = 0x9A;   // 功能码
  Emm_V5_TX_Buffer[2] = o_mode; // 回零模式
  Emm_V5_TX_Buffer[3] = snF;    // 多机同步运动标志
  Emm_V5_TX_Buffer[4] = 0x6B;   // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 5);
}

/**
 * @brief    强制中断并退出回零
 * @param    addr  ：电机地址
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Origin_Interrupt(uint8_t addr)
{
  // 检查串口是否正忙
  if (Emm_V5_HUART.gState != HAL_UART_STATE_READY)
    return HAL_BUSY;

  // 装载命令
  Emm_V5_TX_Buffer[0] = addr; // 地址
  Emm_V5_TX_Buffer[1] = 0x9C; // 功能码
  Emm_V5_TX_Buffer[2] = 0x48; // 辅助码
  Emm_V5_TX_Buffer[3] = 0x6B; // 校验字节

  // 发送命令
  return HAL_UART_Transmit_DMA(&Emm_V5_HUART, Emm_V5_TX_Buffer, 4);
}
