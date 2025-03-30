// 在一个新文件(my_menu.c)中实现
#include "cot_menu.h"
#include "my_menu.h"
#include "lcd.h"
#include <string.h>

extern lcd lcd_desc; // 使用main.c中已经初始化的LCD对象

static void ShowMenu(cotMenuShow_t *ptShowInfo)
{
    static menusize_t lastSelectItem = 0xFF;
    static menusize_t lastShowBaseItem = 0xFF;
    static char lastTitle[32] = {0};
    
    uint8_t showNum = 4; // 一次显示的菜单项数量
    menusize_t tmpselect;
    bool needFullRedraw = false;
    
    // 检查是否需要完全重绘
    if (lastShowBaseItem != ptShowInfo->showBaseItem || 
        strcmp(lastTitle, ptShowInfo->uMenuDesc.pTextString) != 0) {
        needFullRedraw = true;
    }
    
    // 限制显示的菜单项数量
    cotMenu_LimitShowListNum(ptShowInfo, &showNum);
    
    if (needFullRedraw) {
        // 完全重绘 - 先清除显示区域
        lcd_clear(&lcd_desc, BLACK);
        
        // 显示菜单标题
        lcd_set_font(&lcd_desc, FONT_1608, YELLOW, BLACK);
        lcd_print(&lcd_desc, 5, 5, "%s", ptShowInfo->uMenuDesc.pTextString);
        
        // 显示所有菜单项
        for (int i = 0; i < showNum; i++) {
            tmpselect = i + ptShowInfo->showBaseItem;
            
            if (tmpselect == ptShowInfo->selectItem) {
                // 选中项使用不同的颜色
                lcd_fill(&lcd_desc, 0, 30 + i * 30, lcd_desc.hw->width, 30 + (i + 1) * 30 - 2, YELLOW);
                lcd_set_font(&lcd_desc, FONT_1608, BLACK, YELLOW);
            } else {
                lcd_set_font(&lcd_desc, FONT_1608, WHITE, BLACK);
            }
            
            lcd_print(&lcd_desc, 10, 30 + i * 30, "%s", ptShowInfo->uItemsListDesc[tmpselect].pTextString);
        }
    } else if (lastSelectItem != ptShowInfo->selectItem) {
        // 只有选中项发生变化时才更新相关项
        
        // 找出上一个选中项和当前选中项的显示索引
        int lastIndex = -1;
        int currIndex = -1;
        
        for (int i = 0; i < showNum; i++) {
            tmpselect = i + ptShowInfo->showBaseItem;
            
            if (tmpselect == lastSelectItem)
                lastIndex = i;
                
            if (tmpselect == ptShowInfo->selectItem)
                currIndex = i;
        }
        
        // 只更新变化的项
        if (lastIndex >= 0) {
            // 恢复上一个选中项为普通显示
            lcd_fill(&lcd_desc, 0, 30 + lastIndex * 30, lcd_desc.hw->width, 30 + (lastIndex + 1) * 30 - 2, BLACK);
            lcd_set_font(&lcd_desc, FONT_1608, WHITE, BLACK);
            lcd_print(&lcd_desc, 10, 30 + lastIndex * 30, "%s", 
                      ptShowInfo->uItemsListDesc[lastIndex + ptShowInfo->showBaseItem].pTextString);
        }
        
        if (currIndex >= 0) {
            // 将当前选中项设置为高亮显示
            lcd_fill(&lcd_desc, 0, 30 + currIndex * 30, lcd_desc.hw->width, 30 + (currIndex + 1) * 30 - 2, YELLOW);
            lcd_set_font(&lcd_desc, FONT_1608, BLACK, YELLOW);
            lcd_print(&lcd_desc, 10, 30 + currIndex * 30, "%s", 
                      ptShowInfo->uItemsListDesc[currIndex + ptShowInfo->showBaseItem].pTextString);
        }
    }
    
    // 保存当前状态用于下次比较
    lastSelectItem = ptShowInfo->selectItem;
    lastShowBaseItem = ptShowInfo->showBaseItem;
    strncpy(lastTitle, ptShowInfo->uMenuDesc.pTextString, sizeof(lastTitle)-1);
}

// 在my_menu.c中继续添加

// 前向声明
void MainMenu_Enter(const cotMenuItemInfo_t *pItemInfo);
void Settings_Enter(const cotMenuItemInfo_t *pItemInfo);
void Info_Enter(const cotMenuItemInfo_t *pItemInfo);
void About_Enter(const cotMenuItemInfo_t *pItemInfo);

// 主菜单配置
static cotMainMenuCfg_t sg_tMainMenu = {"Main Menu", MainMenu_Enter, NULL, NULL, NULL};

// 主菜单项
cotMenuList_t sg_MainMenuTable[] =
    {
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

// 菜单回调函数实现
void MainMenu_Enter(const cotMenuItemInfo_t *pItemInfo)
{
    cotMenu_Bind(sg_MainMenuTable, COT_GET_MENU_NUM(sg_MainMenuTable), ShowMenu);
}

void Settings_Enter(const cotMenuItemInfo_t *pItemInfo)
{
    cotMenu_Bind(sg_SettingsMenuTable, COT_GET_MENU_NUM(sg_SettingsMenuTable), ShowMenu);
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

// 初始化菜单系统
void Menu_Init(void)
{
    cotMenu_Init(&sg_tMainMenu);
    cotMenu_MainEnter(); // 进入主菜单
}

// 示例：按键处理函数
void Key_Handler(uint8_t key)
{
    switch (key)
    {
    case KEY_UP:
        cotMenu_SelectPrevious(true); // 向上选择
        break;

    case KEY_DOWN:
        cotMenu_SelectNext(true); // 向下选择
        break;

    case KEY_ENTER:
        cotMenu_Enter(); // 进入选中的菜单项
        break;

    case KEY_BACK:
        cotMenu_Exit(true); // 返回上级菜单
        break;
    }
}

uint8_t Get_Key(void)
{
    static uint8_t key_up = 1; // 按键松开标志
    
    // 检查是否有按键被按下（低电平有效）
    if (key_up && 
        (HAL_GPIO_ReadPin(UP_GPIO_Port, UP_Pin) == GPIO_PIN_RESET || 
         HAL_GPIO_ReadPin(DOWN_GPIO_Port, DOWN_Pin) == GPIO_PIN_RESET || 
         HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == GPIO_PIN_RESET || 
         HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == GPIO_PIN_RESET))
    {
        HAL_Delay(10); // 延时消抖
        
        // 二次确认，确保不是抖动
        if (HAL_GPIO_ReadPin(UP_GPIO_Port, UP_Pin) == GPIO_PIN_RESET || 
            HAL_GPIO_ReadPin(DOWN_GPIO_Port, DOWN_Pin) == GPIO_PIN_RESET || 
            HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == GPIO_PIN_RESET || 
            HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == GPIO_PIN_RESET)
        {
            key_up = 0; // 标记按键已按下
            
            // 返回具体按下的按键
            if (HAL_GPIO_ReadPin(UP_GPIO_Port, UP_Pin) == GPIO_PIN_RESET)
                return KEY_UP;
            else if (HAL_GPIO_ReadPin(DOWN_GPIO_Port, DOWN_Pin) == GPIO_PIN_RESET)
                return KEY_DOWN;
            else if (HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == GPIO_PIN_RESET)
                return KEY_ENTER;
            else if (HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == GPIO_PIN_RESET)
                return KEY_BACK;
        }
    }
    else if (HAL_GPIO_ReadPin(UP_GPIO_Port, UP_Pin) == GPIO_PIN_SET && 
             HAL_GPIO_ReadPin(DOWN_GPIO_Port, DOWN_Pin) == GPIO_PIN_SET && 
             HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == GPIO_PIN_SET && 
             HAL_GPIO_ReadPin(MENU_GPIO_Port, MENU_Pin) == GPIO_PIN_SET)
    {
        key_up = 1; // 所有按键都松开了
    }
    
    return KEY_NONE; // 没有按键按下或者按键未松开
}

void Lcd_MenuTask(void)
{
    uint8_t key;

    key = Get_Key();
    if (key != KEY_NONE)
    {
        //反转LED灯
        led_toggle();
        Key_Handler(key);
    }
    cotMenu_Task(); // 菜单任务处理
}
