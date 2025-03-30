#ifndef HMI_MUSIC_H
#define HMI_MUSIC_H

#include "hmi_common.h"


extern const MenuImage_t sgc_MusicImage;

void Hmi_MusicEnter(const cotMenuItemInfo_t *pItemInfo);
void Hmi_MusicExit(const cotMenuItemInfo_t *pItemInfo);
void Hmi_MusicLoad(const cotMenuItemInfo_t *pItemInfo);
void Hmi_MusicTask(const cotMenuItemInfo_t *pItemInfo);

#endif