#ifndef HMI_VIDEO_H
#define HMI_VIDEO_H

#include "hmi_common.h"

extern const MenuImage_t sgc_VideoImage;

void Hmi_VideoLoad(const cotMenuItemInfo_t *pItemInfo);
void Hmi_VideoExit(const cotMenuItemInfo_t *pItemInfo);
void Hmi_VideoTask(const cotMenuItemInfo_t *pItemInfo);

#endif