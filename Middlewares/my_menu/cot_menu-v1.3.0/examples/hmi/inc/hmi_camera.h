#ifndef HMI_CAMERA_H
#define HMI_CAMERA_H

#include "hmi_common.h"

extern const MenuImage_t sgc_CameraImage;

void Hmi_CameraEnter(const cotMenuItemInfo_t *pItemInfo);
void Hmi_CameraExit(const cotMenuItemInfo_t *pItemInfo);
void Hmi_CameraLoad(const cotMenuItemInfo_t *pItemInfo);
void Hmi_CameraTask(const cotMenuItemInfo_t *pItemInfo);

#endif