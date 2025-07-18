#ifndef __EMM_V5_H
#define __EMM_V5_H

#include "usart.h"
#include <stdbool.h>

/**********************************************************
介绍：
这是一个用于控制Emm_V5步进电机的库文件，主要用于控制电机的速度、位置、方向等参数。
这个电机是一个通过伺服系统控制的步进电机可以精确的通过闭环的方式控制电机的位置,速度
方向,并且可以进行多机同步控制。
**********************************************************/

#define Emm_V5_HUART huart3 // 使用串口3控制电机
#define ABS(x) ((x) > 0 ? (x) : -(x))

typedef enum
{
    S_VER = 0,    /* 读取固件版本和对应的硬件版本 */
    S_RL = 1,     /* 读取读取相电阻和相电感 */
    S_PID = 2,    /* 读取PID参数 */
    S_VBUS = 3,   /* 读取总线电压 */
    S_CPHA = 5,   /* 读取相电流 */
    S_ENCL = 7,   /* 读取经过线性化校准后的编码器值 */
    S_TPOS = 8,   /* 读取电机目标位置角度 */
    S_VEL = 9,    /* 读取电机实时转速 */
    S_CPOS = 10,  /* 读取电机实时位置角度 */
    S_PERR = 11,  /* 读取电机位置误差角度 */
    S_FLAG = 13,  /* 读取使能/到位/堵转状态标志位 */
    S_Conf = 14,  /* 读取驱动参数 */
    S_State = 15, /* 读取系统状态参数 */
    S_ORG = 16,   /* 读取正在回零/回零失败状态标志位 */
} SysParams_t;

/**********************************************************
*** 注意：每个函数的参数的具体说明，请查阅对应函数的注释说明
**********************************************************/

/**
 * @brief    将当前位置清零
 * @param    addr  ：电机地址
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Reset_CurPos_To_Zero(uint8_t addr);

/**
 * @brief    解除堵转保护
 * @param    addr  ：电机地址
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Reset_Clog_Pro(uint8_t addr);

/**
 * @brief    读取系统参数
 * @param    addr  ：电机地址
 * @param    s     ：系统参数类型
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Read_Sys_Params(uint8_t addr, SysParams_t s);

/**
 * @brief    修改开环/闭环控制模式
 * @param    addr     ：电机地址
 * @param    svF      ：是否存储标志，false为不存储，true为存储
 * @param    ctrl_mode：控制模式（对应屏幕上的P_Pul菜单），0是关闭脉冲输入引脚，1是开环模式，2是闭环模式，3是让En端口复用为多圈限位开关输入引脚，Dir端口复用为到位输出高电平功能
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Modify_Ctrl_Mode(uint8_t addr, bool svF, uint8_t ctrl_mode);

/**
 * @brief    使能信号控制
 * @param    addr  ：电机地址
 * @param    state ：使能状态     ，true为使能电机，false为关闭电机
 * @param    snF   ：多机同步标志 ，false为不启用，true为启用
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_En_Control(uint8_t addr, bool state, bool snF);

/**
 * @brief    速度模式
 * @param    addr：电机地址
 * @param    dir ：方向       ，0为CW，其余值为CCW
 * @param    vel ：速度       ，范围0 - 5000RPM
 * @param    acc ：加速度     ，范围0 - 255，注意：0是直接启动
 * (加速度在245以上的电机才会得到比较合适的加速度,245以下的时候电机加速度会非常慢,具体计算公式如下:
 * 加速度档位为0表示不使用曲线加减速，直接按照设定的速度运行。曲线加减速时间计算公式：
 * t2 - t1 = (256 - acc) * 50(us)，Vt2 = Vt1 + 1(RPM))
 * @param    snF ：多机同步标志，false为不启用，true为启用
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Vel_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF);

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
HAL_StatusTypeDef Emm_V5_Pos_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, bool raF, bool snF);

/**
 * @brief    立即停止（所有控制模式都通用）
 * @param    addr  ：电机地址
 * @param    snF   ：多机同步标志，false为不启用，true为启用
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Stop_Now(uint8_t addr, bool snF);

/**
 * @brief    多机同步运动
 * @param    addr  ：电机地址
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Synchronous_motion(uint8_t addr);

/**
 * @brief    设置单圈回零的零点位置
 * @param    addr  ：电机地址
 * @param    svF   ：是否存储标志，false为不存储，true为存储
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Origin_Set_O(uint8_t addr, bool svF);

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
HAL_StatusTypeDef Emm_V5_Origin_Modify_Params(uint8_t addr, bool svF, uint8_t o_mode, uint8_t o_dir, uint16_t o_vel, uint32_t o_tm, uint16_t sl_vel, uint16_t sl_ma, uint16_t sl_ms, bool potF);

/**
 * @brief    触发回零
 * @param    addr   ：电机地址
 * @param    o_mode ：回零模式，0为单圈就近回零，1为单圈方向回零，2为多圈无限位碰撞回零，3为多圈有限位开关回零
 * @param    snF   ：多机同步标志，false为不启用，true为启用
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Origin_Trigger_Return(uint8_t addr, uint8_t o_mode, bool snF);

/**
 * @brief    强制中断并退出回零
 * @param    addr  ：电机地址
 * @retval   HAL_StatusTypeDef：HAL状态(HAL_OK, HAL_ERROR, HAL_BUSY)
 */
HAL_StatusTypeDef Emm_V5_Origin_Interrupt(uint8_t addr);

#endif
