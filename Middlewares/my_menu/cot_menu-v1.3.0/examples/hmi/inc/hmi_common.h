#ifndef HMI_COMMON_H
#define HMI_COMMON_H

#include "cot_menu.h"

/* 自定义图标数据 */
typedef struct
{
    const char *pImageFrame;
    const char *pImage;
} MenuImage_t;

// 清除屏幕
#define CLEAR() printf("\033[2J")
// 定位光标
#define MOVETO(x,y) printf("\033[%d;%dH", (x), (y))

void Hmi_OnCommonFunction(const cotMenuItemInfo_t *pItemInfo);

#endif