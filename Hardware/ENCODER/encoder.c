#include "encoder.h"
#include "tim.h"  // ���tim.h�Է���htim3��htim5����
#include "main.h" // ȷ������main.h�Ի�ȡ���Ͷ���

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

/**************************************************************************
�������ܣ���λʱ���ȡ����������
��ڲ�������ʱ��
����  ֵ���ٶ�ֵ
**************************************************************************/
int Read_Encoder(uint8_t TIMX)
{
    int Encoder_TIM = 0;
    
    switch(TIMX)
    {
        case 3:  
            // TIM3��16λ��ʱ��
            Encoder_TIM = (short)TIM3->CNT;  
            TIM3->CNT = 0; 
            break;
            
        case 5:  
            // TIM5��32λ��ʱ����������ֻȡ��16λ����TIM3����һ��
            // �����Ҫ����ȫ��32λ�������޸�Ϊ: Encoder_TIM = (int)TIM5->CNT;
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
�������ܣ�TIM3��������ʼ��
��ڲ�������
����  ֵ����
**************************************************************************/
void Encoder_Init_TIM3(void)
{
    // TIM3����CubeMX������Ϊ������ģʽ
    // ����TIM3�ı�����ģʽ
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
}

/**************************************************************************
�������ܣ�TIM5��������ʼ��
��ڲ�������
����  ֵ����
**************************************************************************/
void Encoder_Init_TIM5(void)
{
    // TIM5����CubeMX������Ϊ������ģʽ
    // ����TIM5�ı�����ģʽ
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);
}
