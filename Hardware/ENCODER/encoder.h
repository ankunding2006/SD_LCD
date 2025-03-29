#ifndef __ENCODER_H
#define __ENCODER_H
#include "sys.h"
#include "stm32f4xx_hal.h"  // ȷ������HAL��ͷ�ļ�
#include "main.h"  // ����main.h��ȡ���Ͷ���

/***********************************************
��˾����Ȥ�Ƽ�����ݸ�����޹�˾
Ʒ�ƣ�WHEELTEC
������wheeltec.net
�Ա����̣�shop114407458.taobao.com 
����ͨ: https://minibalance.aliexpress.com/store/4455017
�汾��V1.0
�޸�ʱ�䣺2023-01-04

Brand: WHEELTEC
Website: wheeltec.net
Taobao shop: shop114407458.taobao.com 
Aliexpress: https://minibalance.aliexpress.com/store/4455017
Version: V1.0
Update��2023-01-04

All rights reserved
***********************************************/

// ��������ʱ����ض���
#define ENCODER_TIM_PERIOD (uint16_t)(65535)   // ���ɴ���65535����ΪF4��16λ��ʱ��������Χ����65535
#define ENCODER_TIM_LEFT   TIM3               // ���ֱ�����ʹ��TIM3
#define ENCODER_TIM_RIGHT  TIM5               // ���ֱ�����ʹ��TIM5

// ��������
void Encoder_Init_TIM3(void);             // ��ʼ��TIM3Ϊ������ģʽ������
void Encoder_Init_TIM5(void);             // ��ʼ��TIM5Ϊ������ģʽ������
int Read_Encoder(uint8_t TIMX);           // ��ȡָ����ʱ���ı���������

#endif
