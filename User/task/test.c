/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdlib.h>
#include "my_menu.h"
#include "Emm_V5.h"
#include "test.h"
#include "gray_detection.h"
#include "control.h"
#include "task.h"
#include "car_config.h"

void main_test(void)
{
    line_following_task();
}

void before_main_test(void)
{
    while (1)
    {
        /* code */
    }
}

void test_car_to_room(void)
{
    Car_To_Room(TEST_ROOM_NUM);
}
