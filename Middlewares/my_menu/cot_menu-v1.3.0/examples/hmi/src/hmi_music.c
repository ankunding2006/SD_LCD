#include "hmi_music.h"
#include "cot_menu.h"
#include "language.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif
static bool sg_isInit = false;

const MenuImage_t sgc_MusicImage = {
"mmmmmmmmmm",
"@"
};

void Hmi_MusicEnter(const cotMenuItemInfo_t *pItemInfo)
{

}

void Hmi_MusicExit(const cotMenuItemInfo_t *pItemInfo)
{

}

void Hmi_MusicLoad(const cotMenuItemInfo_t *pItemInfo)
{
    sg_isInit = true;
}

void Hmi_MusicTask(const cotMenuItemInfo_t *pItemInfo)
{
    int cmd = 0;

    if (sg_isInit)
    {
        sg_isInit = false;
        printf("--------------------------\n");
        printf("     %s%s\n", get_text(pItemInfo->uMenuDesc.textId), get_text(TEXT_FUNCTION_TEST));
        printf("--------------------------\n");
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
    }

    printf("%s(0-%s): ", 
            get_text(TEXT_SELECT_OPTION), get_text(TEXT_EXIT));
    scanf(" %d", &cmd); // 空格作用是忽略上次的回车

    if (cmd == 0)
    {
        cotMenu_Exit(false);
    }
}