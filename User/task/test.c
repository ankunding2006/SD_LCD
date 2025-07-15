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
    grey_sensor_Read();
    float turn_pwm = Calculate_Turn_Value();
    printf("turn_pwm: %.1f\n", turn_pwm);
}
/********************************main_test_end*********************************/
