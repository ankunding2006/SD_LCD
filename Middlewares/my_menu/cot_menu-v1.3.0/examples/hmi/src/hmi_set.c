#include "hmi_set.h"
#include "hmi_more_set.h"
#include "cot_menu.h"
#include "language.h"
#include <stdio.h>
#include <string.h>


const MenuImage_t sgc_SettingImage = {
"$$$$$$$$$$",
"%"
};

static void OnLanguageFunction(const cotMenuItemInfo_t *pItemInfo);
static void OnBluetoothFunction(const cotMenuItemInfo_t *pItemInfo);
static void OnBatteryFunction(const cotMenuItemInfo_t *pItemInfo);
static void OnStorageFunction(const cotMenuItemInfo_t *pItemInfo);

/* 设置菜单 */
cotMenuList_t sg_SetMenuTable[] = 
{
    COT_MENU_ITEM_BIND(TEXT_LANGUAGE,   NULL, NULL, NULL, OnLanguageFunction, NULL),
    COT_MENU_ITEM_BIND(TEXT_BLUETOOTH,  NULL, NULL, NULL, OnBluetoothFunction, NULL),
    COT_MENU_ITEM_BIND(TEXT_BATTERY,    NULL, NULL, NULL, OnBatteryFunction, NULL),
    COT_MENU_ITEM_BIND(TEXT_STORE,      NULL, NULL, NULL, OnStorageFunction, NULL),
    COT_MENU_ITEM_BIND(TEXT_MORE,       Hmi_MoreSetEnter, Hmi_MoreSetExit, Hmi_MoreSetLoad, Hmi_MoreSetTask, NULL),
};

/* 设置菜单显示效果 */
static void ShowSetMenu(cotMenuShow_t *ptShowInfo)
{
    uint8_t showNum = 3;
    menusize_t  tmpselect;

    cotMenu_LimitShowListNum(ptShowInfo, &showNum);

    printf("\e[0;30;46m ------------- %s ------------- \e[0m\n", get_text(ptShowInfo->uMenuDesc.textId));

    for (int i = 0; i < showNum; i++)
    {
        tmpselect = i + ptShowInfo->showBaseItem;

        if (tmpselect == ptShowInfo->selectItem)
        {
            printf("\e[0;30;47m %d. %-16s\e[0m |\n", tmpselect + 1, get_text(ptShowInfo->uItemsListDesc[tmpselect].textId));
        }
        else
        {
            printf("\e[7;30;47m %d. %-16s\e[0m |\n", tmpselect + 1, get_text(ptShowInfo->uItemsListDesc[tmpselect].textId));
        }
    }
}

void Hmi_SetEnter(const cotMenuItemInfo_t *pItemInfo)
{
    cotMenu_Bind(sg_SetMenuTable, COT_GET_MENU_NUM(sg_SetMenuTable), ShowSetMenu);
}

void Hmi_SetLoad(const cotMenuItemInfo_t *pItemInfo)
{
    
}

void Hmi_SetExit(const cotMenuItemInfo_t *pItemInfo)
{

}

void Hmi_SetTask(const cotMenuItemInfo_t *pItemInfo)
{
    int cmd;

    printf("%s(0-%s; 1-%s; 2-%s; 3-%s; 4-%s): ", 
            get_text(TEXT_SELECT_OPTION), get_text(TEXT_RETURN), get_text(TEXT_RETURN_MAIN_MENU),
            get_text(TEXT_ENTER), get_text(TEXT_NEXT), get_text(TEXT_PREVIOUS));
    scanf(" %d", &cmd); // 空格作用是忽略上次的回车
 
    switch (cmd)
    {
    case 0:
        cotMenu_Exit(false);
        break;
    case 1:
        cotMenu_Reset();
        break;
    case 2:
        cotMenu_Enter();
        break;
    case 3:
        cotMenu_SelectNext(true);
        break;
    case 4:
        cotMenu_SelectPrevious(true);
        break;

    default:
        break;    
    }
}


static void OnLanguageFunction(const cotMenuItemInfo_t *pItemInfo)
{
    int cmd;

    printf("--------------------------\n");
    printf("     %s%s\n", get_text(TEXT_LANGUAGE), get_text(TEXT_FUNCTION_TEST));
    printf("--------------------------\n");
    
    printf("%s(0-%s; 1-%s): ", 
            get_text(TEXT_SELECT_OPTION), 
            "中文", 
            "English");
    scanf(" %d", &cmd); // 空格作用是忽略上次的回车

    if (cmd == 0)
    {
        set_language(SYSTEM_LANGUAGE_CHINESE);
    }
    else
    {
        set_language(SYSTEM_LANGUAGE_ENGLISH);
    }

    cotMenu_Exit(0); // 切换后自动退出
}

static void OnBluetoothFunction(const cotMenuItemInfo_t *pItemInfo)
{
    int cmd;

    printf("--------------------------\n");
    printf("     %s%s\n", get_text(TEXT_BLUETOOTH), get_text(TEXT_FUNCTION_TEST));
    printf("--------------------------\n");

    printf("%s(0-%s): ", 
            get_text(TEXT_SELECT_OPTION), get_text(TEXT_EXIT));
    scanf(" %d", &cmd); // 空格作用是忽略上次的回车

    if (cmd == 0)
    {
        cotMenu_Exit(false);
    }
}

static void OnBatteryFunction(const cotMenuItemInfo_t *pItemInfo)
{
    int cmd;

    printf("--------------------------\n");
    printf("     %s%s\n", get_text(TEXT_BATTERY), get_text(TEXT_FUNCTION_TEST));
    printf("--------------------------\n");

    printf("%s(0-%s): ", 
            get_text(TEXT_SELECT_OPTION), get_text(TEXT_EXIT));
    scanf(" %d", &cmd); // 空格作用是忽略上次的回车

    if (cmd == 0)
    {
        cotMenu_Exit(false);
    }
}

static void OnStorageFunction(const cotMenuItemInfo_t *pItemInfo)
{
    int cmd;

    printf("--------------------------\n");
    printf("     %s%s\n", get_text(TEXT_STORE), get_text(TEXT_FUNCTION_TEST));
    printf("--------------------------\n");

    printf("%s(0-%s): ", 
            get_text(TEXT_SELECT_OPTION), get_text(TEXT_EXIT));
    scanf(" %d", &cmd); // 空格作用是忽略上次的回车

    if (cmd == 0)
    {
        cotMenu_Exit(false);
    }
}