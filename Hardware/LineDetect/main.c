#include "stm32f10x.h"
#include "Uart.h"
#include "stdio.h"
#include "stdint.h"
#include "delay.h"
#include "gray_detection.h"
#include "soft_i2c.h"
#include "nchd12.h"


/*************************************************************
灰度传感器硬件连接如下：
12路灰度传感器输出从右边到左边依次编号为P1、P2、...、P12通过I2C方式读取12路灰度传感器数据
SCL接STM32F103C8T6单片机PA0
SDA接STM32F103C8T6单片机PA1
串口检测结果打印：
USART1_RXD  PA10接USB转TTL模块的TXD
USART1_TXD  PA9 接USB转TTL模块的RXD
程序运行状态指示灯为PC13,可以根据自己手头板子实际硬件来调整
**************************************************************/




extern float gray_status[2];
extern _gray_state gray_state; 
void nNVIC_Init(void)
{ 	
	NVIC_InitTypeDef NVIC_InitStructure;//定义一个NVIC向量表结构体变量
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断组 为2 
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//配置串口1为中断源
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//设置占先优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//设置副优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能串口1中断
	NVIC_Init(&NVIC_InitStructure);//根据参数初始化中断寄存器
		
	NVIC_InitStructure.NVIC_IRQChannel =TIM2_IRQn ;//计数定时器
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void led_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//定义一个GPIO结构体变量
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	   	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   
	GPIO_Init(GPIOC, &GPIO_InitStructure);					  	
  GPIO_SetBits(GPIOC,GPIO_Pin_13);
}

void TIM2_Configuration(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  TIM_DeInit(TIM2);
  TIM_TimeBaseStructure.TIM_Period = 20000;//20ms
  TIM_TimeBaseStructure.TIM_Prescaler = 72-1; //1us
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM2, ENABLE);
}
u8 data_1;
int main(void)
{
	SystemInit();	//系统时钟配置system_stm32f10x.c中
	delay_init(72);//滴答定时器延时
	led_gpio_init();//核心板板载LED初始化
	i2c_CheckDevice(0x40);//灰度传感器IIC初始化
	Usart1_Configuration(115200);	//串口配置 设置波特率为115200
	TIM2_Configuration();//定时器中断初始化
	nNVIC_Init();//中断向量表注册函数
	printf("---------------无名创新12路灰度传感器模块----------------\r\n");
	while(1)													
	{ 
		GPIO_SetBits(GPIOC,GPIO_Pin_13);
		delay_ms(50);
		GPIO_ResetBits(GPIOC,GPIO_Pin_13);
		delay_ms(50);
		//检测结果打印	
		printf("%d%d%d%d%d%d%d%d%d%d%d%d \t %d \t %f \t %f\n", 
						gray_state.gray.bit12,
						gray_state.gray.bit11,
						gray_state.gray.bit10,
						gray_state.gray.bit9,
						gray_state.gray.bit8,
						gray_state.gray.bit7,
						gray_state.gray.bit6,
						gray_state.gray.bit5,
						gray_state.gray.bit4,
						gray_state.gray.bit3,
						gray_state.gray.bit2,
						gray_state.gray.bit1,
						gray_state.state,gray_status[0],gray_status[1]);
		
	}
}



void TIM2_IRQHandler(void)//10ms
{
  if( TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET )
  {
		gray_state.state=pca9555_read_bit12(0x20<<1);//通过I2C方式读取12路灰度传感器数据
		gpio_input_check_channel_12_linewidth_10mm();//针对10mm引导线的偏差提取
		gpio_input_check_channel_12_linewidth_20mm();//针对20mm引导线的偏差提取
    TIM_ClearITPendingBit(TIM2,TIM_FLAG_Update);
  }
}

