#include "main.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include <stdlib.h>
#include "usmart.h"
#include "delay.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "usart.h"
#include "ui.h"


void init_hardware(void);
void init_lvgl(void);
void init_ui_and_navigation(void);
void init_led_animation(void);
