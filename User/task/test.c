/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include "my_menu.h"
#include "delay.h"
#include "usmart.h"
#include "lfs_port.h"
#include "Emm_V5.h"
#include "car_config.h"
#include "test.h"
#include "gray_detection.h"

/**********************************task_test***********************************/

/********************************task_test_end*********************************/

/**********************************main_test***********************************/
void main_test(void)
{
    Emm_V5_Vel_Control(0x01, 1, 500, 243, 0);
    delay_ms(1000);
    Emm_V5_Vel_Control(0x01, 0, 500, 243, 0);
    delay_ms(1000);
}

void before_main_test(void)
{
    while (1)
    {
        /* code */
    }
}
/********************************main_test_end*********************************/
