#include "stm32f10x.h"
#include "gray_detection.h"



_gray_state gray_state; 

float gray_status[2]={0},gray_status_backup[2][20]={0};//灰度传感器状态与历史值
uint32_t gray_status_worse=0;	//灰度管异常状态计数器






/***************************************************
函数名: void gpio_input_check_channel_12_linewidth_10mm(void)
说明:	12路灰度管gpio检测
入口:	无
出口:	无
备注:	无
作者:	无名创新
****************************************************/
void gpio_input_check_channel_12_linewidth_10mm(void)
{	
	for(uint16_t i=19;i>0;i--)
	{
		gray_status_backup[0][i]=gray_status_backup[0][i-1];
	}
	gray_status_backup[0][0]=gray_status[0];
	switch(gray_state.state)
	{
		case 0x0001:gray_status[0]=-11;gray_status_worse/=2;break;									//000000000001b
		case 0x0003:gray_status[0]=-10;gray_status_worse/=2;break;									//000000000011b
	  case 0x0002:gray_status[0]=-9;	gray_status_worse/=2;break;									//000000000010b
		case 0x0006:gray_status[0]=-8;	gray_status_worse/=2;break;									//000000000110b
		case 0x0004:gray_status[0]=-7;	gray_status_worse/=2;break;									//000000000100b
		case 0x000C:gray_status[0]=-6;	gray_status_worse/=2;break;									//000000001100b
		case 0x0008:gray_status[0]=-5;	gray_status_worse/=2;break;									//000000001000b
		case 0x0018:gray_status[0]=-4;	gray_status_worse/=2;break;									//000000011000b
		case 0x0010:gray_status[0]=-3;	gray_status_worse/=2;break;									//000000010000b
		case 0x0030:gray_status[0]=-2;	gray_status_worse/=2;break;									//000000110000b
		case 0x0020:gray_status[0]=-1;	gray_status_worse/=2;break;									//000000100000b
		case 0x0060:gray_status[0]=0;		gray_status_worse/=2;break;									//000001100000b
		case 0x0040:gray_status[0]=1;		gray_status_worse/=2;break;									//000001000000b
		case 0x00C0:gray_status[0]=2;		gray_status_worse/=2;break;									//000011000000b
		case 0x0080:gray_status[0]=3;		gray_status_worse/=2;break;									//000010000000b
		case 0x0180:gray_status[0]=4;		gray_status_worse/=2;break;									//000110000000b
		case 0x0100:gray_status[0]=5;		gray_status_worse/=2;break;									//000100000000b
		case 0x0300:gray_status[0]=6;		gray_status_worse/=2;break;									//001100000000b
		case 0x0200:gray_status[0]=7;		gray_status_worse/=2;break;									//001000000000b
		case 0x0600:gray_status[0]=8;		gray_status_worse/=2;break;									//011000000000b
		case 0x0400:gray_status[0]=9;		gray_status_worse/=2;break;									//010000000000b
		case 0x0C00:gray_status[0]=10;	gray_status_worse/=2;break;									//110000000000b
		case 0x0800:gray_status[0]=11;	gray_status_worse/=2;break;									//100000000000b
		case 0x0000:gray_status[0]=gray_status_backup[0][0];gray_status_worse++;break; //00000000b
		default://其它特殊情况单独判断
		{
			gray_status[0]=gray_status_backup[0][0];
			gray_status_worse++;
		}
	}	
	
	static uint16_t tmp_cnt=0;
	switch(gray_state.state)//停止线检测
	{
		case 0x0030:tmp_cnt++;break;//000000110000b
		case 0x0020:tmp_cnt++;break;//000000100000b
		case 0x0060:tmp_cnt++;break;//000001100000b
		case 0x0040:tmp_cnt++;break;//000001000000b
		case 0x00C0:tmp_cnt++;break;//000011000000b
		case 0x00F0://000011110000b
		{
			if(tmp_cnt>=10)
			{
				tmp_cnt=0;	
			}
		}
		break;
	}		
}



void gpio_input_check_channel_12_linewidth_20mm(void)
{
	for(uint16_t i=19;i>0;i--)
	{
		gray_status_backup[1][i]=gray_status_backup[1][i-1];
	}
	gray_status_backup[1][0]=gray_status[1];
	switch(gray_state.state)
	{
		case 0x0001:gray_status[1]=-11; break;									//000000000001b
		case 0x0003:gray_status[1]=-10; break;									//000000000011b
		case 0x0002:gray_status[1]=-9;  break;									//000000000010b
		case 0x0007:gray_status[1]=-9;  break;									//000000000111b
		case 0x0006:gray_status[1]=-8;  break;									//000000000110b
		case 0x0004:gray_status[1]=-7;  break;									//000000000100b
		case 0x000E:gray_status[1]=-7;  break;									//000000001110b
		case 0x000C:gray_status[1]=-6;  break;									//000000001100b
		case 0x0008:gray_status[1]=-5;  break;									//000000001000b
		case 0x001C:gray_status[1]=-5;  break;									//000000011100b
		case 0x0018:gray_status[1]=-4;  break;									//000000011000b
		case 0x0010:gray_status[1]=-3;  break;									//000000010000b
		case 0x0038:gray_status[1]=-3;  break;									//000000111000b
		case 0x0030:gray_status[1]=-2;  break;									//000000110000b
		case 0x0020:gray_status[1]=-1;  break;									//000000100000b
		case 0x0070:gray_status[1]=-1;  break;									//000001110000b
		case 0x0060:gray_status[1]= 0;  break;									//000001100000b
		case 0x0040:gray_status[1]= 1;  break;									//000001000000b
		case 0x00E0:gray_status[1]= 1;  break;									//000011100000b
		case 0x00C0:gray_status[1]= 2;  break;									//000011000000b
		case 0x0080:gray_status[1]= 3;  break;									//000010000000b
		case 0x01C0:gray_status[1]= 3;  break;									//000111000000b
		case 0x0180:gray_status[1]= 4;  break;									//000110000000b
		case 0x0100:gray_status[1]= 5;  break;									//000100000000b
		case 0x0380:gray_status[1]= 5;  break;									//001110000000b
		case 0x0300:gray_status[1]= 6;  break;									//001100000000b
		case 0x0200:gray_status[1]= 7;  break;									//001000000000b
		case 0x0700:gray_status[1]= 7;  break;									//011100000000b
		case 0x0600:gray_status[1]= 8;  break;									//011000000000b
		case 0x0400:gray_status[1]= 9;  break;									//010000000000b
		case 0x0E00:gray_status[1]= 9;  break;									//111000000000b
		case 0x0C00:gray_status[1]= 10; break;									//110000000000b
		case 0x0800:gray_status[1]= 11; break;									//100000000000b
		case 0x0000:
		default://其它特殊情况单独判断
		{
			gray_status[1]=0;
		}
	}	
	
}
