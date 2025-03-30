#ifndef HMI_SET_H
#define HMI_SET_H

#include "hmi_common.h"

extern const MenuImage_t sgc_SettingImage;

void Hmi_SetEnter(const cotMenuItemInfo_t *pItemInfo);
void Hmi_SetExit(const cotMenuItemInfo_t *pItemInfo);
void Hmi_SetLoad(const cotMenuItemInfo_t *pItemInfo);
void Hmi_SetTask(const cotMenuItemInfo_t *pItemInfo);

#endif