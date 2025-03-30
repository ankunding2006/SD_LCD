
/* 菜单显示效果图可看: 

https://blog.csdn.net/qq_24130227/article/details/121167276 

*/

#include "cot_menu.h"
#include "mainhmi.h"
#include "language.h"
#include <stdio.h>
#include <string.h>

int isEnterMenu = 0;

void EnterMainMenu(const cotMenuItemInfo_t *pItemInfo)
{
    isEnterMenu = 1;
}

void ExitMainMenu(const cotMenuItemInfo_t *pItemInfo)
{
    isEnterMenu = 0;
}

int main(int argc, char **argv)
{
    int cmd = 0;
    int8_t musicMenuId, languageMenuId, moreSetMenuId;
    
    cotMainMenuCfg_t tMainMenu = COT_MENU_ITEM_BIND(TEXT_MAIN_MENU, EnterMainMenu, ExitMainMenu, Hmi_LoadMainHmi, Hmi_MainTask, NULL);

    cotMenu_Init(&tMainMenu);
    
    while (1)
    {
        CLEAR();
        MOVETO(0, 0);

        if (!isEnterMenu)
        {
            printf("%s(0-%s%s; 1-%s): ", get_text(TEXT_SELECT_OPTION), 
                    get_text(TEXT_ENTER), get_text(TEXT_MAIN_MENU), get_text(TEXT_EXIT));
            scanf(" %d", &cmd); // 空格作用是忽略上次的回车

            if (cmd == 0)
            {
                cotMenu_MainEnter();

                CLEAR();
                MOVETO(0, 0);
            }
            else if (cmd == 1)
            {
                break;
            }
        }

        cotMenu_Task();
    }

    cotMenu_DeInit();

    return 0;
}