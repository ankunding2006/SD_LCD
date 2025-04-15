/*----------------------------------------------------------------------------------------------------------------------/
*               本程序只供购买者学习使用，版权著作权属于无名科创团队，无名科创团队将飞控程序源码提供给购买者，
*               购买者要为无名科创团队提供保护，未经作者许可，不得将源代码提供给他人，不得将源代码放到网上供他人免费下载， 
*               更不能以此销售牟利，如发现上述行为，无名科创团队将诉之以法律解决！！！
-----------------------------------------------------------------------------------------------------------------------/
*               生命不息、奋斗不止；前人栽树，后人乘凉！！！
*               开源不易，且学且珍惜，祝早日逆袭、进阶成功！！！
*               学习优秀者，简历可推荐到DJI、ZEROTECH、XAG、AEE、GDU、AUTEL、EWATT、HIGH GREAT等公司就业
*               求职简历请发送：15671678205@163.com，需备注求职意向单位、岗位、待遇等
*               无名科创开源飞控QQ群：540707961
*               CSDN博客：http://blog.csdn.net/u011992534
*               优酷ID：NamelessCotrun无名小哥
*               B站教学视频：https://space.bilibili.com/67803559/#/video
*               客户使用心得、改进意见征集贴：http://www.openedv.com/forum.php?mod=viewthread&tid=234214&extra=page=1
*               淘宝店铺：https://shop348646912.taobao.com/?spm=2013.1.1000126.2.5ce78a88ht1sO2
*               百度贴吧:无名科创开源飞控
*               修改日期:2018/8/30
*               版本：慧飞者——V1.0.1
*               版权所有，盗版必究。
*               Copyright(C) 武汉科技大学无名科创团队 2017-2025
*               All rights reserved
----------------------------------------------------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "delay.h"
u8   fac_us;
u16  fac_ms;
void delay_init(u8 SYSCLK)
{
  SysTick->CTRL &= 0xfffffffb;
  fac_us=SYSCLK/8;
  fac_ms=(u16)fac_us*1000;
}
void delay_us(u32 nus)
{
  u32 temp;
  SysTick->LOAD = nus*fac_us;
  SysTick->VAL = 0x00;
  SysTick->CTRL = 0x01;
  temp = SysTick->CTRL;
  while((temp&0x01)&&(!(temp&(1<<16))))temp = SysTick->CTRL;
  SysTick->CTRL=0x00;
  SysTick->VAL =0X00;
  
}
void delay_ms(u16 nms)
{
  u32 temp;
  SysTick->LOAD = (u32)nms*fac_ms;
  SysTick->VAL = 0x00;
  SysTick->CTRL = 0x01;
  temp = SysTick->CTRL;
  while((temp&0x01)&&(!(temp&(1<<16)))) temp = SysTick->CTRL;
  SysTick->CTRL=0x00;
  SysTick->VAL =0X00;
}



void Delay(vu32 nCount)
{
  for(; nCount!= 0;nCount--);
}

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: 
** 函数名称: Delay_Ms
** 功能描述: 延时1MS (可通过仿真来判断他的准确度)           
** 参数描述：time (ms) 注意time<65535 
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/  
void Delay_Ms(uint16_t time)  //延时函数  
{   
  uint16_t i,j;  
  for(i=0;i<time;i++)  
    for(j=0;j<10260;j++);  
}  
/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: 
** 函数名称: Delay_Us 
** 功能描述: 延时1us (可通过仿真来判断他的准确度) 
** 参数描述：time (us) 注意time<65535                 
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/  
void Delay_Us(uint16_t time)  //延时函数  
{   
  uint16_t i,j;  
  for(i=0;i<time;i++)  
    for(j=0;j<9;j++);  
}  


