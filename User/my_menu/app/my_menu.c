// 在一个新文件(my_menu.c)中实现
#include "cot_menu.h"
#include "my_menu.h"
#include "lcd.h"
#include <string.h>

extern lcd lcd_desc; // 使用main.c中已经初始化的LCD对象

// 菜单布局参数
#define MENU_SHOW_NUM         4       // 显示项数量
#define MENU_ITEM_HEIGHT      30      // 菜单项高度
#define MENU_TITLE_Y          5       // 标题Y坐标
#define MENU_ITEMS_START_Y    30      // 菜单项开始Y坐标
#define MENU_ITEM_TEXT_OFFSET 10      // 菜单项文本左侧偏移量

// 使用全局变量跟踪菜单状态
static menusize_t g_lastSelectItem = 0xFF;
static menusize_t g_lastShowBaseItem = 0xFF;
static char g_lastTitle[32] = {0};
static bool g_menuNeedFullRedraw = false;

static void ShowMenu(cotMenuShow_t *ptShowInfo)
{
    // 边界检查
    if (!ptShowInfo || !ptShowInfo->uMenuDesc.pTextString) {
        return;
    }

    // 确保selectItem在有效范围内
    if (ptShowInfo->selectItem >= ptShowInfo->itemsNum) {
        ptShowInfo->selectItem = ptShowInfo->itemsNum - 1;
    }
    
    uint8_t showNum = MENU_SHOW_NUM;  // 一次显示的菜单项数量
    menusize_t tmpselect;
    bool needFullRedraw = g_menuNeedFullRedraw;
    
    // 检查菜单层级的变化 (通过比较标题判断)
    if (strcmp(g_lastTitle, ptShowInfo->uMenuDesc.pTextString) != 0) {
        needFullRedraw = true;
    }
    
    // 检查是否需要完全重绘
    if (g_lastShowBaseItem != ptShowInfo->showBaseItem || needFullRedraw) {
        needFullRedraw = true;
    }

    // 检查是否发生了从最后一项到第一项(或反之)的循环
    if (g_lastSelectItem == ptShowInfo->itemsNum - 1 && ptShowInfo->selectItem == 0) {
        needFullRedraw = true;
    } else if (g_lastSelectItem == 0 && ptShowInfo->selectItem == ptShowInfo->itemsNum - 1) {
        needFullRedraw = true;
    }
    
    // 限制显示的菜单项数量
    cotMenu_LimitShowListNum(ptShowInfo, &showNum);
    
    if (needFullRedraw) {
        // 完全重绘 - 先清除显示区域
        lcd_clear(&lcd_desc, BLACK);
        
        // 显示菜单标题
        lcd_set_font(&lcd_desc, FONT_1608, YELLOW, BLACK);
        lcd_print(&lcd_desc, 5, MENU_TITLE_Y, "%s", ptShowInfo->uMenuDesc.pTextString);
        
        // 如果有父级菜单，显示导航提示
        if (ptShowInfo->pExtendData) {
            lcd_set_font(&lcd_desc, FONT_1206, GRAY, BLACK);
            lcd_print(&lcd_desc, 5, MENU_TITLE_Y + 16, "< %s", (char*)ptShowInfo->pExtendData);
        }
        
        // 显示所有菜单项
        for (int i = 0; i < showNum; i++) {
            tmpselect = i + ptShowInfo->showBaseItem;
            
            // 先清除该位置的背景
            lcd_fill(&lcd_desc, 0, MENU_ITEMS_START_Y + i * MENU_ITEM_HEIGHT, 
                     lcd_desc.hw->width, MENU_ITEMS_START_Y + (i + 1) * MENU_ITEM_HEIGHT - 2, BLACK);
            
            if (tmpselect == ptShowInfo->selectItem) {
                // 选中项使用不同的颜色
                lcd_fill(&lcd_desc, 0, MENU_ITEMS_START_Y + i * MENU_ITEM_HEIGHT, 
                         lcd_desc.hw->width, MENU_ITEMS_START_Y + (i + 1) * MENU_ITEM_HEIGHT - 2, YELLOW);
                lcd_set_font(&lcd_desc, FONT_1608, BLACK, YELLOW);
            } else {
                lcd_set_font(&lcd_desc, FONT_1608, WHITE, BLACK);
            }
            
            lcd_print(&lcd_desc, MENU_ITEM_TEXT_OFFSET, MENU_ITEMS_START_Y + i * MENU_ITEM_HEIGHT, 
                     "%s", ptShowInfo->uItemsListDesc[tmpselect].pTextString);
        }
        
        // 显示滚动指示器
        if (ptShowInfo->showBaseItem > 0) {
            lcd_set_font(&lcd_desc, FONT_1608, CYAN, BLACK);
            lcd_print(&lcd_desc, lcd_desc.hw->width - 20, 10, "^");
        }

        if (ptShowInfo->showBaseItem + showNum < ptShowInfo->itemsNum) {
            lcd_set_font(&lcd_desc, FONT_1608, CYAN, BLACK);
            lcd_print(&lcd_desc, lcd_desc.hw->width - 20, MENU_ITEMS_START_Y + (showNum - 1) * MENU_ITEM_HEIGHT, "v");
        }
        
    } else if (g_lastSelectItem != ptShowInfo->selectItem) {
        // 找出上一个选中项和当前选中项的显示索引
        int lastIndex = -1;
        int currIndex = -1;
        
        // 优化索引查找
        if (g_lastSelectItem >= ptShowInfo->showBaseItem && 
            g_lastSelectItem < ptShowInfo->showBaseItem + showNum) {
            lastIndex = g_lastSelectItem - ptShowInfo->showBaseItem;
        }

        if (ptShowInfo->selectItem >= ptShowInfo->showBaseItem && 
            ptShowInfo->selectItem < ptShowInfo->showBaseItem + showNum) {
            currIndex = ptShowInfo->selectItem - ptShowInfo->showBaseItem;
        }
        
        // 确保清除旧的高亮显示
        if (lastIndex >= 0) {
            // 完全清除上一个选中项的区域
            lcd_fill(&lcd_desc, 0, MENU_ITEMS_START_Y + lastIndex * MENU_ITEM_HEIGHT, 
                     lcd_desc.hw->width, MENU_ITEMS_START_Y + (lastIndex + 1) * MENU_ITEM_HEIGHT - 2, BLACK);
            lcd_set_font(&lcd_desc, FONT_1608, WHITE, BLACK);
            lcd_print(&lcd_desc, MENU_ITEM_TEXT_OFFSET, MENU_ITEMS_START_Y + lastIndex * MENU_ITEM_HEIGHT, "%s", 
                      ptShowInfo->uItemsListDesc[lastIndex + ptShowInfo->showBaseItem].pTextString);
        }
        
        // 绘制新的高亮显示
        if (currIndex >= 0) {
            // 先清除当前选中项的区域
            lcd_fill(&lcd_desc, 0, MENU_ITEMS_START_Y + currIndex * MENU_ITEM_HEIGHT, 
                     lcd_desc.hw->width, MENU_ITEMS_START_Y + (currIndex + 1) * MENU_ITEM_HEIGHT - 2, BLACK);
            // 再绘制高亮背景
            lcd_fill(&lcd_desc, 0, MENU_ITEMS_START_Y + currIndex * MENU_ITEM_HEIGHT, 
                     lcd_desc.hw->width, MENU_ITEMS_START_Y + (currIndex + 1) * MENU_ITEM_HEIGHT - 2, YELLOW);
            lcd_set_font(&lcd_desc, FONT_1608, BLACK, YELLOW);
            lcd_print(&lcd_desc, MENU_ITEM_TEXT_OFFSET, MENU_ITEMS_START_Y + currIndex * MENU_ITEM_HEIGHT, "%s", 
                      ptShowInfo->uItemsListDesc[currIndex + ptShowInfo->showBaseItem].pTextString);
        }
        
        // 更新滚动指示器
        if (ptShowInfo->showBaseItem > 0) {
            lcd_set_font(&lcd_desc, FONT_1608, CYAN, BLACK);
            lcd_print(&lcd_desc, lcd_desc.hw->width - 20, 10, "^");
        }

        if (ptShowInfo->showBaseItem + showNum < ptShowInfo->itemsNum) {
            lcd_set_font(&lcd_desc, FONT_1608, CYAN, BLACK);
            lcd_print(&lcd_desc, lcd_desc.hw->width - 20, MENU_ITEMS_START_Y + (showNum - 1) * MENU_ITEM_HEIGHT, "v");
        }
    }
    
    // 保存当前状态用于下次比较
    g_lastSelectItem = ptShowInfo->selectItem;
    g_lastShowBaseItem = ptShowInfo->showBaseItem;
    strncpy(g_lastTitle, ptShowInfo->uMenuDesc.pTextString, sizeof(g_lastTitle)-1);
    g_menuNeedFullRedraw = false;  // 重置重绘标志
}

// 前向声明
void MainMenu_Enter(const cotMenuItemInfo_t *pItemInfo);
void Settings_Enter(const cotMenuItemInfo_t *pItemInfo);
void Info_Enter(const cotMenuItemInfo_t *pItemInfo);
void About_Enter(const cotMenuItemInfo_t *pItemInfo);
void Settings_Exit(const cotMenuItemInfo_t *pItemInfo);
void LED_Menu_Enter(const cotMenuItemInfo_t *pItemInfo);
void LED1_Toggle_Handler(const cotMenuItemInfo_t *pItemInfo);
void LED2_Toggle_Handler(const cotMenuItemInfo_t *pItemInfo);
void LED3_Toggle_Handler(const cotMenuItemInfo_t *pItemInfo);
void All_LEDs_On_Handler(const cotMenuItemInfo_t *pItemInfo);
void All_LEDs_Off_Handler(const cotMenuItemInfo_t *pItemInfo);

// 主菜单配置
static cotMainMenuCfg_t sg_tMainMenu = {"Main Menu", MainMenu_Enter, NULL, NULL, NULL};

// 主菜单项
cotMenuList_t sg_MainMenuTable[] = {
    COT_MENU_ITEM_BIND("LED Control", LED_Menu_Enter, NULL, NULL, NULL, NULL),
    COT_MENU_ITEM_BIND("Settings", Settings_Enter, NULL, NULL, NULL, NULL),
    COT_MENU_ITEM_BIND("Info", Info_Enter, NULL, NULL, NULL, NULL),
    COT_MENU_ITEM_BIND("About", About_Enter, NULL, NULL, NULL, NULL),
};

// 设置子菜单项
cotMenuList_t sg_SettingsMenuTable[] =
    {
        COT_MENU_ITEM_BIND("Brightness", NULL, NULL, NULL, NULL, NULL),
        COT_MENU_ITEM_BIND("Contrast", NULL, NULL, NULL, NULL, NULL),
        COT_MENU_ITEM_BIND("Language", NULL, NULL, NULL, NULL, NULL),
    };

// LED控制菜单项
cotMenuList_t sg_LEDMenuTable[] = {
    COT_MENU_ITEM_BIND("LED1 Toggle", LED1_Toggle_Handler, NULL, NULL, NULL, NULL),
    COT_MENU_ITEM_BIND("LED2 Toggle", LED2_Toggle_Handler, NULL, NULL, NULL, NULL),
    COT_MENU_ITEM_BIND("LED3 Toggle", LED3_Toggle_Handler, NULL, NULL, NULL, NULL),
    COT_MENU_ITEM_BIND("All LEDs On", All_LEDs_On_Handler, NULL, NULL, NULL, NULL),
    COT_MENU_ITEM_BIND("All LEDs Off", All_LEDs_Off_Handler, NULL, NULL, NULL, NULL),
};

// 菜单回调函数实现
void MainMenu_Enter(const cotMenuItemInfo_t *pItemInfo)
{
    g_menuNeedFullRedraw = true;  // 进入主菜单时强制重绘
    cotMenu_Bind(sg_MainMenuTable, COT_GET_MENU_NUM(sg_MainMenuTable), ShowMenu);
}

void Settings_Enter(const cotMenuItemInfo_t *pItemInfo)
{
    g_menuNeedFullRedraw = true;  // 进入设置子菜单时强制重绘
    cotMenu_Bind(sg_SettingsMenuTable, COT_GET_MENU_NUM(sg_SettingsMenuTable), ShowMenu);
}

void Settings_Exit(const cotMenuItemInfo_t *pItemInfo)
{
    g_menuNeedFullRedraw = true;  // 退出设置子菜单时强制重绘
}

void Info_Enter(const cotMenuItemInfo_t *pItemInfo)
{
    // 显示信息页面
    lcd_fill(&lcd_desc, 0, 0, lcd_desc.hw->width, lcd_desc.hw->height, BLACK);
    lcd_set_font(&lcd_desc, FONT_1608, WHITE, BLACK);
    lcd_print(&lcd_desc, 10, 10, "Info Page");
    lcd_print(&lcd_desc, 10, 40, "STM32F4 Project");
    lcd_print(&lcd_desc, 10, 60, "LCD: ST7789 2.0\"");
}

void About_Enter(const cotMenuItemInfo_t *pItemInfo)
{
    // 显示关于页面
    lcd_fill(&lcd_desc, 0, 0, lcd_desc.hw->width, lcd_desc.hw->height, BLACK);
    lcd_set_font(&lcd_desc, FONT_1608, WHITE, BLACK);
    lcd_print(&lcd_desc, 10, 10, "About");
    lcd_print(&lcd_desc, 10, 40, "Version: 1.0");
    lcd_print(&lcd_desc, 10, 60, "Date: 2024/09");
}

// LED控制菜单回调函数
void LED1_Toggle_Handler(const cotMenuItemInfo_t *pItemInfo)
{
    led1_toggle();
    cotMenu_Exit(false); // 执行完LED操作后返回上一级菜单
}

void LED2_Toggle_Handler(const cotMenuItemInfo_t *pItemInfo)
{
    led2_toggle();
    cotMenu_Exit(false); // 执行完LED操作后返回上一级菜单
}

void LED3_Toggle_Handler(const cotMenuItemInfo_t *pItemInfo)
{
    led3_toggle();
    cotMenu_Exit(false); // 执行完LED操作后返回上一级菜单
}

void All_LEDs_On_Handler(const cotMenuItemInfo_t *pItemInfo)
{
    all_leds_on();
    cotMenu_Exit(false); // 执行完LED操作后返回上一级菜单
}

void All_LEDs_Off_Handler(const cotMenuItemInfo_t *pItemInfo)
{
    all_leds_off();
    cotMenu_Exit(false); // 执行完LED操作后返回上一级菜单
}

void LED_Menu_Enter(const cotMenuItemInfo_t *pItemInfo)
{
    g_menuNeedFullRedraw = true;
    cotMenu_Bind(sg_LEDMenuTable, COT_GET_MENU_NUM(sg_LEDMenuTable), ShowMenu);
}

// 初始化菜单系统
void Menu_Init(void)
{
    g_lastSelectItem = 0xFF;
    g_lastShowBaseItem = 0xFF;
    g_lastTitle[0] = 0;
    g_menuNeedFullRedraw = true;
    
    cotMenu_Init(&sg_tMainMenu);
    cotMenu_MainEnter(); // 进入主菜单
}

// 示例：按键处理函数
void Key_Handler(uint8_t key)
{
    // switch (key)
    // {
    // case KEY_UP:
    //     cotMenu_SelectPrevious(true); // 向上选择
    //     break;

    // case KEY_DOWN:
    //     cotMenu_SelectNext(true); // 向下选择
    //     break;

    // case KEY_ENTER:
    //     cotMenu_Enter(); // 进入选中的菜单项
    //     break;

    // case KEY_BACK:
    //     if(cotMenu_Exit(true) == 0) { // 返回上级菜单
    //         // 退出成功时设置重绘标志
    //         g_menuNeedFullRedraw = true;
    //     }
    //     break;
    // }
}

// uint8_t Get_Key(void)
// {
//     static uint8_t key_up = 1; // 按键松开标志
    
//     // 检查是否有按键被按下（低电平有效）
//     if (key_up && 
//         (HAL_GPIO_ReadPin(UP_GPIO_Port, UP_Pin) == GPIO_PIN_RESET || 
//          HAL_GPIO_ReadPin(DOWN_GPIO_Port, DOWN_Pin) == GPIO_PIN_RESET || 
//          HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == GPIO_PIN_RESET || 
//          HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == GPIO_PIN_RESET))
//     {
//         HAL_Delay(10); // 延时消抖
        
//         // 二次确认，确保不是抖动
//         if (HAL_GPIO_ReadPin(UP_GPIO_Port, UP_Pin) == GPIO_PIN_RESET || 
//             HAL_GPIO_ReadPin(DOWN_GPIO_Port, DOWN_Pin) == GPIO_PIN_RESET || 
//             HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == GPIO_PIN_RESET || 
//             HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == GPIO_PIN_RESET)
//         {
//             key_up = 0; // 标记按键已按下
            
//             // 返回具体按下的按键
//             if (HAL_GPIO_ReadPin(UP_GPIO_Port, UP_Pin) == GPIO_PIN_RESET)
//                 return KEY_UP;
//             else if (HAL_GPIO_ReadPin(DOWN_GPIO_Port, DOWN_Pin) == GPIO_PIN_RESET)
//                 return KEY_DOWN;
//             else if (HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == GPIO_PIN_RESET)
//                 return KEY_ENTER;
//             else if (HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == GPIO_PIN_RESET)
//                 return KEY_BACK;
//         }
//     }
//     else if (HAL_GPIO_ReadPin(UP_GPIO_Port, UP_Pin) == GPIO_PIN_SET && 
//              HAL_GPIO_ReadPin(DOWN_GPIO_Port, DOWN_Pin) == GPIO_PIN_SET && 
//              HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == GPIO_PIN_SET && 
//              HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == GPIO_PIN_SET)
//     {
//         key_up = 1; // 所有按键都松开了
//     }
    
//     return KEY_NONE; // 没有按键按下或者按键未松开
// }

void Lcd_MenuTask(void)
{
    uint8_t key;

    //! key = Get_Key();
    if (key != KEY_NONE)
    {
        //反转LED灯
        led_toggle();
        //! Key_Handler(key);
    }
    cotMenu_Task(); // 菜单任务处理
}
