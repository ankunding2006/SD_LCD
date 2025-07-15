/**
 ****************************************************************************************************
 * @file        usart.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2023-06-05
 * @brief       串口初始化代码(一般是串口1)，支持printf
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 探索者 F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211014
 * 第一次发布
 * V1.1 20230605
 * 删除USART_UX_IRQHandler()函数的超时处理和修改HAL_UART_RxCpltCallback()
 ****************************************************************************************************
 */
#include "main.h"
#include "sys.h"
#include "user_usart.h"

/* 如果使用os,则包括下面的头文件即可 */
#if SYS_SUPPORT_OS
#include "os.h" /* os 使用 */
#endif

/******************************************************************************************/
/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)           /* 使用AC6编译器时 */
__asm(".global __use_no_semihosting\n\t"); /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");   /* AC6下需要声明main函数为无参数格式，否则部分例程可能出现半主机模式 */

#else
/* 使用AC5编译器时, 要在这里定义__FILE 和 不使用半主机模式 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* 不使用半主机模式，至少需要重定义_ttywrch\_sys_exit\_sys_command_string函数,以同时兼容AC6和AC5模式 */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE 在 stdio.h里面定义. */
FILE __stdout;

/* 重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口 */
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0)
        ; /* 等待上一个字符发送完成 */

    USART1->DR = (uint8_t)ch; /* 将要发送的字符 ch 写入到DR寄存器 */
    return ch;
}
#endif
/***********************************************END*******************************************/

#if USART_EN_RX /* 如果使能了接收 */

/* 接收缓冲, 最大USART_REC_LEN个字节. */
uint8_t g_usart_rx_buf[USART_REC_LEN];

/*  接收状态
 *  bit15，      接收完成标志
 *  bit14，      接收到0x0d
 *  bit13~0，    接收到的有效字节数目
 */
uint16_t g_usart_rx_sta = 0;

uint8_t g_rx_buffer[RXBUFFERSIZE]; /* HAL库使用的串口接收缓冲 */

/**
 * @brief       Rx传输回调函数
 * @param       huart: UART句柄类型指针
 * @retval      无
 */
void USART_UX_Handler(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART_UX) /* 如果是串口1 */
    {
        if ((g_usart_rx_sta & 0x8000) == 0) /* 接收未完成 */
        {
            if (g_usart_rx_sta & 0x4000) /* 接收到了0x0d */
            {
                if (g_rx_buffer[0] != 0x0a)
                {
                    g_usart_rx_sta = 0; /* 接收错误,重新开始 */
                }
                else
                {
                    g_usart_rx_sta |= 0x8000; /* 接收完成了 */
                }
            }
            else /* 还没收到0X0D */
            {
                if (g_rx_buffer[0] == 0x0d)
                {
                    g_usart_rx_sta |= 0x4000;
                }
                else
                {
                    g_usart_rx_buf[g_usart_rx_sta & 0X3FFF] = g_rx_buffer[0];
                    g_usart_rx_sta++;
                    if (g_usart_rx_sta > (USART_REC_LEN - 1))
                    {
                        g_usart_rx_sta = 0; /* 接收数据错误,重新开始接收 */
                    }
                }
            }
        }

        HAL_UART_Receive_IT(huart, (uint8_t *)g_rx_buffer, RXBUFFERSIZE);
    }
}

#endif
