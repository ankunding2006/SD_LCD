#include "hmi_video.h"
#include "cot_menu.h"
#include "language.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

const MenuImage_t sgc_VideoImage = {
"vvvvvvvvvv",
"#"
};




void Hmi_VideoLoad(const cotMenuItemInfo_t *pItemInfo)
{
    printf("--------------------------\n");
    printf("     %s %s...\n", get_text(pItemInfo->uMenuDesc.textId), get_text(TEXT_LOADING));
    printf("--------------------------\n");
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
    CLEAR();
    MOVETO(0, 0);
}

void Hmi_VideoExit(const cotMenuItemInfo_t *pItemInfo)
{
    CLEAR();
    MOVETO(0, 0);

    printf("--------------------------\n");
    printf("     %s %s...\n", get_text(pItemInfo->uMenuDesc.textId), get_text(TEXT_EXIT));
    printf("--------------------------\n");
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
}

void Hmi_VideoTask(const cotMenuItemInfo_t *pItemInfo)
{
    int cmd;

    printf("--------------------------\n");
    printf("     %s%s\n", get_text(pItemInfo->uMenuDesc.textId), get_text(TEXT_FUNCTION_TEST));
    printf("--------------------------\n");

    printf("%s(0-%s): ", 
            get_text(TEXT_SELECT_OPTION), get_text(TEXT_EXIT));
    scanf(" %d", &cmd); // 空格作用是忽略上次的回车

    if (cmd == 0)
    {
        cotMenu_Exit(false);
    }
}
