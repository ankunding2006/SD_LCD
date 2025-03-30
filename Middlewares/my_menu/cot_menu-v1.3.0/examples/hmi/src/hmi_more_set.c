#include "hmi_more_set.h"
#include "cot_menu.h"
#include "language.h"
#include <stdio.h>
#include <string.h>


/* 更多设置 */
cotMenuList_t sg_MoreSetMenuTable[] = 
{
    COT_MENU_ITEM_BIND(TEXT_UPGRADE, NULL, NULL, NULL, Hmi_OnCommonFunction, NULL),
    COT_MENU_ITEM_BIND(TEXT_ABOUT,   NULL, NULL, NULL, Hmi_OnCommonFunction, NULL),
};

/* 更多设置菜单显示效果: 右侧弹出菜单效果 */
static void ShowMoreSetMenu(cotMenuShow_t *ptShowInfo)
{
    uint8_t showNum = 3;
    uint8_t showsubNum = 3;
    menusize_t  tmpselect;

    cotMenuShow_t tParentMenuShowInfo;

    if (cotMenu_QueryParentMenu(&tParentMenuShowInfo, 1) != 0)
    {
        return;
    }

    cotMenu_LimitShowListNum(&tParentMenuShowInfo, &showNum);
    cotMenu_LimitShowListNum(ptShowInfo, &showsubNum);

    printf("\e[0;30;46m ------------- %s ------------- \e[0m\n", get_text(tParentMenuShowInfo.uMenuDesc.textId));

    for (int i = 0; i < showNum; i++)
    {
        tmpselect = i + tParentMenuShowInfo.showBaseItem;

        if (tmpselect == tParentMenuShowInfo.selectItem)
        {
            printf("\e[0;30;47m %d. %-16s\e[0m |", tmpselect + 1, get_text(tParentMenuShowInfo.uItemsListDesc[tmpselect].textId));
        }
        else
        {
            printf("\e[7;30;47m %d. %-16s\e[0m |", tmpselect + 1, get_text(tParentMenuShowInfo.uItemsListDesc[tmpselect].textId));
        }

        if (i < showsubNum)
        {
            tmpselect = i + ptShowInfo->showBaseItem;

            if (tmpselect == ptShowInfo->selectItem)
            {
                printf(" \e[0;30;47m %-14s\e[0m", get_text(ptShowInfo->uItemsListDesc[tmpselect].textId));
            }
            else
            {
                printf(" \e[7;30;47m %-14s\e[0m", get_text(ptShowInfo->uItemsListDesc[tmpselect].textId));
            }
        }

        printf("\n");
    }   
}

void Hmi_MoreSetEnter(const cotMenuItemInfo_t *pItemInfo)
{
    cotMenu_Bind(sg_MoreSetMenuTable, COT_GET_MENU_NUM(sg_MoreSetMenuTable), ShowMoreSetMenu);
}

void Hmi_MoreSetLoad(const cotMenuItemInfo_t *pItemInfo)
{

}

void Hmi_MoreSetExit(const cotMenuItemInfo_t *pItemInfo)
{

}

void Hmi_MoreSetTask(const cotMenuItemInfo_t *pItemInfo)
{
    int cmd;

    printf("%s(0-%s; 1-%s%s; 2-%s; 3-%s; 4-%s): ", 
            get_text(TEXT_SELECT_OPTION), get_text(TEXT_RETURN), get_text(TEXT_RETURN), get_text(TEXT_MAIN_MENU),
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

