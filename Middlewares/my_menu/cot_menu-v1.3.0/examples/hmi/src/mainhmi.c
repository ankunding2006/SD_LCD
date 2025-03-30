#include "mainhmi.h"
#include "hmi_camera.h"
#include "hmi_music.h"
#include "hmi_set.h"
#include "hmi_video.h"
#include "hmi_common.h"
#include "cot_menu.h"
#include "language.h"
#include <stdio.h>
#include <string.h>



/* 主菜单 */
cotMenuList_t sg_MainMenuTable[] = 
{
    COT_MENU_ITEM_BIND(TEXT_MUSIC,  Hmi_MusicEnter, Hmi_MusicExit, Hmi_MusicLoad, Hmi_MusicTask, (MenuImage_t *)&sgc_MusicImage),
    
    COT_MENU_ITEM_BIND(TEXT_VIDEO,  NULL, Hmi_VideoExit, Hmi_VideoLoad, Hmi_VideoTask, (MenuImage_t *)&sgc_VideoImage),
    
    COT_MENU_ITEM_BIND(TEXT_CAMERA,  Hmi_CameraEnter, Hmi_CameraExit, Hmi_CameraLoad, Hmi_CameraTask, (MenuImage_t *)&sgc_CameraImage),
    
    COT_MENU_ITEM_BIND(TEXT_SETTING,  Hmi_SetEnter, Hmi_SetExit, Hmi_SetLoad,   Hmi_SetTask, (MenuImage_t *)&sgc_SettingImage),
};



/* 主菜单显示效果 */
static void ShowMainMenu(cotMenuShow_t *ptShowInfo)
{
    uint8_t showNum = 3;
    MenuImage_t *pMenuImage;
    menusize_t  tmpselect;

    cotMenu_LimitShowListNum(ptShowInfo, &showNum);

    printf("\e[0;30;47m ------------- %s ------------- \e[0m\n", get_text(ptShowInfo->uMenuDesc.textId));

    for (int i = 0; i < showNum; i++)
    {
        tmpselect = i + ptShowInfo->showBaseItem;
        pMenuImage = (MenuImage_t *)ptShowInfo->pItemsListExtendData[tmpselect];

        if (tmpselect == ptShowInfo->selectItem)
        {
            printf("\e[0;30;47m %-10s \e[0m", pMenuImage->pImageFrame);
        }
        else
        {
            printf("\e[7;30;47m %-10s \e[0m", pMenuImage->pImageFrame);
        }
    }

    printf("\n");

    for (int i = 0; i < showNum; i++)
    {
        tmpselect = i + ptShowInfo->showBaseItem;
        pMenuImage = (MenuImage_t *)ptShowInfo->pItemsListExtendData[tmpselect];

        if (tmpselect == ptShowInfo->selectItem)
        {
            printf("\e[0;30;47m %-s%-8s%-s \e[0m", pMenuImage->pImage, get_text(ptShowInfo->uItemsListDesc[tmpselect].textId), pMenuImage->pImage);
        }
        else
        {
            printf("\e[7;30;47m %-s%-8s%-s \e[0m", pMenuImage->pImage, get_text(ptShowInfo->uItemsListDesc[tmpselect].textId), pMenuImage->pImage);
        }
    }

    printf("\n");

    for (int i = 0; i < showNum; i++)
    {
        tmpselect = i + ptShowInfo->showBaseItem;
        pMenuImage = (MenuImage_t *)ptShowInfo->pItemsListExtendData[tmpselect];

        if (tmpselect == ptShowInfo->selectItem)
        {
            printf("\e[0;30;47m %-10s \e[0m", pMenuImage->pImageFrame);
        }
        else
        {
            printf("\e[7;30;47m %-10s \e[0m", pMenuImage->pImageFrame);
        }
    }

    printf("\n");
}

void Hmi_LoadMainHmi(const cotMenuItemInfo_t *pItemInfo)
{
    cotMenu_Bind(sg_MainMenuTable, COT_GET_MENU_NUM(sg_MainMenuTable), ShowMainMenu);
}

void Hmi_MainTask(const cotMenuItemInfo_t *pItemInfo)
{
    int cmd;

    printf("%s(0-%s; 2-%s; 3-%s; 4-%s; 5-%s(%s); 6-%s(%s); 7-%s(%s)): ", 
            get_text(TEXT_SELECT_OPTION), get_text(TEXT_EXIT_MAIN_MENU),
            get_text(TEXT_ENTER), get_text(TEXT_NEXT), get_text(TEXT_PREVIOUS),
            get_text(TEXT_MUSIC), get_text(TEXT_SHORTCUT), 
            get_text(TEXT_LANGUAGE), get_text(TEXT_SHORTCUT), 
            get_text(TEXT_MORE_SETTING), get_text(TEXT_SHORTCUT));
    scanf(" %d", &cmd); // 空格作用是忽略上次的回车
 
    switch (cmd)
    {
    case 0:
        cotMenu_MainExit();
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

    case 5:
        cotMenu_ShortcutEnter(true, 1, 0);
        break;

    case 6:
        cotMenu_ShortcutEnter(true, 2, 3, 0);
        break;

    case 7:
        cotMenu_ShortcutEnter(true, 2, 3, 4);
        break;
        
    default:
        break;    
    }
            
}