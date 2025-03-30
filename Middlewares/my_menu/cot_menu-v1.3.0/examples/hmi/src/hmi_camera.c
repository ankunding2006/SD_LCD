#include "hmi_camera.h"
#include "cot_menu.h"
#include "language.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

const MenuImage_t sgc_CameraImage = {
"**********",
"&"
};


/* 摄像机菜单 */
cotMenuList_t sg_CameraMenuTable[] = 
{
    COT_MENU_ITEM_BIND("TEXT_PHOTO", NULL, NULL, NULL, Hmi_OnCommonFunction, NULL),
    COT_MENU_ITEM_BIND(TEXT_PHOTOGRAPHY, NULL, NULL, NULL, Hmi_OnCommonFunction, NULL),
};

/* 摄像机菜单显示效果 */
static void ShowCameraMenu(cotMenuShow_t *ptShowInfo)
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
            printf("\e[0;30;47m %d. %-34s\e[0m\n", tmpselect + 1, get_text(ptShowInfo->uItemsListDesc[tmpselect].textId));
        }
        else
        {
            printf("\e[7;30;47m %d. %-34s\e[0m\n", tmpselect + 1, get_text(ptShowInfo->uItemsListDesc[tmpselect].textId));
        }
    }
}

void Hmi_CameraEnter(const cotMenuItemInfo_t *pItemInfo)
{
    cotMenu_Bind(sg_CameraMenuTable, COT_GET_MENU_NUM(sg_CameraMenuTable), ShowCameraMenu);
}

void Hmi_CameraLoad(const cotMenuItemInfo_t *pItemInfo)
{
    CLEAR();
    MOVETO(0, 0);
    printf("---%s-----\n", get_text(TEXT_LOADING));
}

void Hmi_CameraExit(const cotMenuItemInfo_t *pItemInfo)
{
    printf("--------------------------\n");
    printf("     %s%s...\n", get_text(TEXT_VIDEO), get_text(TEXT_STOPPING_PLAYBACK));
    printf("--------------------------\n");
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
}

void Hmi_CameraTask(const cotMenuItemInfo_t *pItemInfo)
{
    int cmd;

    printf("%s(0-%s; 1-%s%s; 2-%s; 3-%s; 4-%s): ", 
            get_text(TEXT_SELECT_OPTION), get_text(TEXT_RETURN), get_text(TEXT_RETURN), get_text(TEXT_MAIN_MENU),
            get_text(TEXT_ENTER), get_text(TEXT_NEXT), get_text(TEXT_PREVIOUS));
    scanf(" %d", &cmd); // 空格作用是忽略上次的回车
 
    switch (cmd)
    {
    case 0:
        cotMenu_Exit(true);
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