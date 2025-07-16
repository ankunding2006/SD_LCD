/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdlib.h>
#include "my_menu.h"
#include "Emm_V5.h"
#include "test.h"
#include "gray_detection.h"
#include "control.h"
#include "task.h"

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

// 假设这是中断调用的程序
void test()
{
    uint8_t cross_nums = visual_process_command(); // 处理消息
    Car_To_Crossing(cross_nums);
    
}