#include "usmart.h"
#include "usmart_str.h"

/******************************************************************************************/
/* 用户配置区
 * 这下面要包含所用到的函数所申明的头文件(用户自己添加)
 */

#include "control.h" // 控制函数头文件
#include "encoder.h" // 编码器函数头文件
#include "gray_detection.h" // 添加灰度传感器头文件
#include "tim.h"


/* 函数名列表初始化(用户自己添加)
 * 用户直接在这里输入要执行的函数名及其查找串
 */
struct _m_usmart_nametab usmart_nametab[] =
{
#if USMART_USE_WRFUNS == 1      /* 如果使能了读写操作 */
    {(void *)read_addr, "uint32_t read_addr(uint32_t addr)"},
    {(void *)write_addr, "void write_addr(uint32_t addr, uint32_t val)"}, 
#endif
    {(void*)Set_Pwm, "void Set_Pwm(int motor_left, int motor_right)"},
    {(void *)Read_Encoder, "int Read_Encoder(uint8_t TIMX)"},
    {(void *)toggle_Flag_Stop, "void toggle_Flag_Stop(void)"},
    {(void *)HAL_TIM6_toggle_IT, "void HAL_TIM6_toggle_IT(void)"},
    {(void *)Set_Target_Velocity, "void Set_Target_Velocity(int Target_Velocity)"}, 
    {(void *)getHeadingAngle, "float getHeadingAngle(void)"},
    
    // 新增转向控制参数调节函数 - 使用整数参数(放大100倍)
    {(void *)Set_Steering_Kp, "void Set_Steering_Kp(u16 kp)"},
    {(void *)Set_Steering_Ki, "void Set_Steering_Ki(u16 ki)"},
    {(void *)Set_Steering_Kd, "void Set_Steering_Kd(u16 kd)"},
    {(void *)Set_Steering_Error_Threshold, "void Set_Steering_Error_Threshold(u16 threshold)"},
    {(void *)Set_All_Steering_Params, "void Set_All_Steering_Params(u16 kp, u16 ki, u16 kd, u16 threshold)"},
    {(void *)Get_Steering_Kp, "float Get_Steering_Kp(void)"},
    {(void *)Get_Steering_Ki, "float Get_Steering_Ki(void)"},
    {(void *)Get_Steering_Kd, "float Get_Steering_Kd(void)"},
    {(void *)Get_Steering_Error_Threshold, "float Get_Steering_Error_Threshold(void)"},
};

/******************************************************************************************/

/* 函数控制管理器初始化
 * 得到各个受控函数的名字
 * 得到函数总数量
 */
struct _m_usmart_dev usmart_dev =
{
    usmart_nametab,
    usmart_init,
    usmart_cmd_rec,
    usmart_exe,
    usmart_scan,
    sizeof(usmart_nametab) / sizeof(struct _m_usmart_nametab), /* 函数数量 */
    0,      /* 参数数量 */
    0,      /* 函数ID */
    1,      /* 参数显示类型,0,10进制;1,16进制 */
    0,      /* 参数类型.bitx:,0,数字;1,字符串 */
    0,      /* 每个参数的长度暂存表,需要MAX_PARM个0初始化 */
    0,      /* 函数的参数,需要PARM_LEN个0初始化 */
};
