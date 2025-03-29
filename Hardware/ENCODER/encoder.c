#include "encoder.h"
#include "tim.h"  // 添加tim.h以访问htim3和htim5变量
#include "main.h" // 确保包含main.h以获取类型定义

/***********************************************
公司：轮趣科技（东莞）有限公司
品牌：WHEELTEC
官网：wheeltec.net
淘宝店铺：shop114407458.taobao.com 
速卖通: https://minibalance.aliexpress.com/store/4455017
版本：V1.0
修改时间：2023-01-04

Brand: WHEELTEC
Website: wheeltec.net
Taobao shop: shop114407458.taobao.com 
Aliexpress: https://minibalance.aliexpress.com/store/4455017
Version: V1.0
Update：2023-01-04

All rights reserved
***********************************************/

/**************************************************************************
函数功能：单位时间读取编码器计数
入口参数：定时器
返回  值：速度值
**************************************************************************/
int Read_Encoder(uint8_t TIMX)
{
    int Encoder_TIM = 0;
    
    switch(TIMX)
    {
        case 3:  
            // TIM3是16位定时器
            Encoder_TIM = (short)TIM3->CNT;  
            TIM3->CNT = 0; 
            break;
            
        case 5:  
            // TIM5是32位定时器，但我们只取低16位以与TIM3保持一致
            // 如果需要利用全部32位，可以修改为: Encoder_TIM = (int)TIM5->CNT;
            Encoder_TIM = (short)TIM5->CNT;  
            TIM5->CNT = 0;
            break;
            
        default:  
            Encoder_TIM = 0;
            break;
    }
    
    return Encoder_TIM;
}

/**************************************************************************
函数功能：TIM3编码器初始化
入口参数：无
返回  值：无
**************************************************************************/
void Encoder_Init_TIM3(void)
{
    // TIM3已在CubeMX中配置为编码器模式
    // 启动TIM3的编码器模式
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
}

/**************************************************************************
函数功能：TIM5编码器初始化
入口参数：无
返回  值：无
**************************************************************************/
void Encoder_Init_TIM5(void)
{
    // TIM5已在CubeMX中配置为编码器模式
    // 启动TIM5的编码器模式
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);
}
