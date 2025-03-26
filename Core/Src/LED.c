#include "LED.h"

// LED1 控制函数
void led1_toggle(void)
{
  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
}

void led1_on(void)
{
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
}

void led1_off(void)
{
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
}

// LED2 控制函数
void led2_toggle(void)
{
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
}

void led2_on(void)
{
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}

void led2_off(void)
{
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
}

// LED3 控制函数
void led3_toggle(void)
{
  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
}

void led3_on(void)
{
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
}

void led3_off(void)
{
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
}

// 全部LED控制函数
void all_leds_on(void)
{
  led1_on();
  led2_on();
  led3_on();
}

void all_leds_off(void)
{
  led1_off();
  led2_off();
  led3_off();
}

void all_leds_toggle(void)
{
  led1_toggle();
  led2_toggle();
  led3_toggle();
}

// 为了兼容原来的函数，保留这些函数但修改为使用LED1
void led_toggle(void)
{
  led1_toggle();
}

void led_on(void)
{
  led1_on();
}

void led_off(void)
{
  led1_off();
}
