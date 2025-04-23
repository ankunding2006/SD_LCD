#ifndef __TASK_H
#define __TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "control.h"
#include "LED.h"
#include <stdio.h>

// 任务函数声明
u8 Task1_Handler(void);
u8 Task2_Handler(void);
u8 Task3_Handler(void);
u8 Task4_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __TASK_H */
