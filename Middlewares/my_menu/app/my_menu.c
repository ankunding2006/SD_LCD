#include "my_menu.h"
#include <stdio.h>
#include <string.h>

// 为菜单项定义的回调函数
static void OnMainMenuEnter(const cotMenuItemInfo_t *pItemInfo);
static void OnMainMenuExit(const cotMenuItemInfo_t *pItemInfo);
static void OnMainMenuLoad(const cotMenuItemInfo_t *pItemInfo);
static void OnMainMenuTask(const cotMenuItemInfo_t *pItemInfo);

// 子菜单回调函数
static void OnParamMenuEnter(const cotMenuItemInfo_t *pItemInfo);
static void OnParamMenuExit(const cotMenuItemInfo_t *pItemInfo);
static void OnParamMenuLoad(const cotMenuItemInfo_t *pItemInfo);

// 主菜单配置
static cotMainMenuCfg_t sg_tMainMenu = COT_MENU_ITEM_BIND("主菜单", OnMainMenuEnter, OnMainMenuExit, OnMainMenuLoad, OnMainMenuTask, NULL);

// 主菜单项列表
cotMenuList_t sg_MainMenuTable[] = 
{
    COT_MENU_ITEM_BIND("参数设置", OnParamMenuEnter, OnParamMenuExit, OnParamMenuLoad, NULL, NULL),
    COT_MENU_ITEM_BIND("传感器状态", NULL, NULL, NULL, NULL, NULL),
    COT_MENU_ITEM_BIND("系统信息", NULL, NULL, NULL, NULL, NULL),
};

// 参数菜单项列表
cotMenuList_t sg_ParamMenuTable[] = 
{
    COT_MENU_ITEM_BIND("电机速度", NULL, NULL, NULL, NULL, NULL),
    COT_MENU_ITEM_BIND("平衡角度", NULL, NULL, NULL, NULL, NULL),
    COT_MENU_ITEM_BIND("PID参数", NULL, NULL, NULL, NULL, NULL),
};

// 菜单显示函数
static void ShowMenu(cotMenuShow_t *ptShowInfo)
{
    uint8_t showNum = 5; // 屏幕可显示的菜单项数量
    menusize_t tmpselect;
    
    // 根据屏幕高度调整可显示的菜单项数
    cotMenu_LimitShowListNum(ptShowInfo, &showNum);
    
    // 清屏
    lcd_clear(&lcd_desc, MENU_BG_COLOR);
    
    // 显示菜单标题
    lcd_set_font(&lcd_desc, FONT_1608, MENU_TEXT_COLOR, MENU_BG_COLOR);
    char title[32] = {0};
    sprintf(title, "-- %s --", (char*)ptShowInfo->uMenuDesc.pTextString);
    uint16_t title_x = (lcd_desc.hw->width - strlen(title)*8) / 2;
    lcd_print(&lcd_desc, title_x, 5, title);
    
    // 绘制分隔线
    lcd_draw_line(&lcd_desc, 0, 25, lcd_desc.hw->width, 25, BLUE);
    
    // 显示菜单项
    for (int i = 0; i < showNum; i++)
    {
        tmpselect = i + ptShowInfo->showBaseItem;
        
        // 设置选中项的颜色
        if (tmpselect == ptShowInfo->selectItem)
        {
            lcd_set_font(&lcd_desc, FONT_1608, MENU_SELECT_COLOR, MENU_SELECT_BG_COLOR);
            lcd_fill(&lcd_desc, 0, 30+i*20, lcd_desc.hw->width, 30+(i+1)*20-2, MENU_SELECT_BG_COLOR);
        }
        else
        {
            lcd_set_font(&lcd_desc, FONT_1608, MENU_TEXT_COLOR, MENU_BG_COLOR);
        }
        
        // 显示菜单项内容
        lcd_print(&lcd_desc, 10, 30+i*20, "%s", (char*)ptShowInfo->uItemsListDesc[tmpselect].pTextString);
    }
}

// 初始化菜单系统
void Menu_Init(void)
{
    cotMenu_Init(&sg_tMainMenu);
}

// 菜单处理函数，在主循环中定期调用
void Menu_Handler(void)
{
    cotMenu_Task();
}

// 菜单按键处理函数
void Menu_KeyUp(void)
{
    cotMenu_SelectPrevious(true);
}

void Menu_KeyDown(void)
{
    cotMenu_SelectNext(true);
}

void Menu_KeyEnter(void)
{
    cotMenu_Enter();
}

void Menu_KeyBack(void)
{
    cotMenu_Exit(false);
}

// 主菜单回调函数实现
static void OnMainMenuEnter(const cotMenuItemInfo_t *pItemInfo)
{
    // 进入主菜单时的操作
}

static void OnMainMenuExit(const cotMenuItemInfo_t *pItemInfo)
{
    // 退出主菜单时的操作
}

static void OnMainMenuLoad(const cotMenuItemInfo_t *pItemInfo)
{
    // 绑定主菜单列表和显示函数
    cotMenu_Bind(sg_MainMenuTable, sizeof(sg_MainMenuTable)/sizeof(cotMenuList_t), ShowMenu);
}

static void OnMainMenuTask(const cotMenuItemInfo_t *pItemInfo)
{
    // 主菜单周期性任务
}

// 参数菜单回调函数实现
static void OnParamMenuEnter(const cotMenuItemInfo_t *pItemInfo)
{
    // 进入参数菜单时的操作
    cotMenu_Bind(sg_ParamMenuTable, sizeof(sg_ParamMenuTable)/sizeof(cotMenuList_t), ShowMenu);
}

static void OnParamMenuExit(const cotMenuItemInfo_t *pItemInfo)
{
    // 退出参数菜单时的操作
}

static void OnParamMenuLoad(const cotMenuItemInfo_t *pItemInfo)
{
    // 参数菜单加载时的操作
}