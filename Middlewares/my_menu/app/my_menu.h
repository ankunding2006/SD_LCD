#ifndef __MY_MENU_H
#define __MY_MENU_H

#include "cot_menu.h"
#include "lcd.h"

// 菜单颜色定义
#define MENU_BG_COLOR        WHITE
#define MENU_TEXT_COLOR      BLACK
#define MENU_SELECT_BG_COLOR BLUE
#define MENU_SELECT_COLOR    WHITE

// 菜单项定义 (示例)
typedef enum {
    MENU_MAIN = 0,
    MENU_PARAM,
    MENU_SENSOR,
    MENU_SYSTEM,
    // 可按需添加更多菜单项
} MENU_ID;

// 用于菜单项的函数原型
void Menu_Init(void);
void Menu_Handler(void);

// 菜单按键操作函数
void Menu_KeyUp(void);
void Menu_KeyDown(void);
void Menu_KeyEnter(void);
void Menu_KeyBack(void);

#endif