/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdlib.h>
#include "my_menu.h"
#include "Emm_V5.h"
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
