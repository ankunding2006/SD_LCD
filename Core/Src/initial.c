#include "initial.h"

void init_hardware(void)
{
  delay_init(168);     /* 延时初始化 */
  usart_init(115200);  /* 串口初始化为115200 */
  usmart_dev.init(84); /* USMART初始化 */
  HAL_Delay(10);       /* 延时10ms */
}

void init_lvgl(void)
{
  lv_init();                  // 初始化LVGL库
  lv_port_disp_init();        // 初始化显示驱动
  lv_port_indev_init();       // 初始化输入设备
  lv_tick_set_cb(HAL_GetTick); // 设置LVGL的tick回调函数
}

void init_led_animation(void)
{
  all_leds_off();
  HAL_Delay(500);
  led1_on();
  HAL_Delay(500);
  led1_off();
  led2_on();
  HAL_Delay(500);
  led2_off();
  led3_on();
  HAL_Delay(500);
  led3_off();
}

void init_ui_and_navigation(void)
{
  ui_init(); // 初始化UI

  /* 创建导航组并添加所有控件 */
  lv_group_t *g = lv_group_create();

  /* 添加所有按钮到导航组中，无论在哪个屏幕 */
  lv_group_add_obj(g, ui_Button1);  // Screen1的按钮
  lv_group_add_obj(g, ui_Button4);  // Screen2的按钮

  /* 让按钮1获得初始焦点 */
  lv_group_focus_obj(ui_Button1);

  /* 设置为默认导航组 */
  lv_group_set_default(g);

  /* 将输入设备与导航组关联 */
  lv_indev_set_group(indev_button, g);

  /* 强制刷新显示 */
  lv_refr_now(NULL);
}
