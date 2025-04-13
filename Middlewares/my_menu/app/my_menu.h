// my_menu.h
#ifndef __MY_MENU_H
#define __MY_MENU_H

#include "cot_menu.h"
#include "main.h"
#include "LED.h"  // 添加LED控制头文件

void Menu_Init(void);
uint8_t Get_Key(void);
void Key_Handler(uint8_t key);
void Lcd_MenuTask(void);

// 定义按键值
#define KEY_NONE    0
#define KEY_UP      1
#define KEY_DOWN    2
#define KEY_ENTER   3
#define KEY_BACK    4

#endif
