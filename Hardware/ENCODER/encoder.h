#ifndef __ENCODER_H
#define __ENCODER_H
#include "sys.h"
#include "stm32f4xx_hal.h"  // 确保包含HAL库头文件
#include "main.h"  // 包含main.h获取类型定义

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

// 编码器定时器相关定义
#define ENCODER_TIM_PERIOD (uint16_t)(65535)   // 不可大于65535，因为F4的16位定时器计数范围就是65535
#define ENCODER_TIM_LEFT   TIM3               // 左轮编码器使用TIM3
#define ENCODER_TIM_RIGHT  TIM5               // 右轮编码器使用TIM5

// 函数声明
void Encoder_Init_TIM3(void);             // 初始化TIM3为编码器模式并启动
void Encoder_Init_TIM5(void);             // 初始化TIM5为编码器模式并启动
int Read_Encoder(uint8_t TIMX);           // 读取指定定时器的编码器计数

#endif
