#ifndef __TASK_H
#define __TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// 任务函数声明
u8 Task2_Handler(void);
// 可以添加其他任务函数声明，如Task1_Handler, Task3_Handler等

#ifdef __cplusplus
}
#endif

#endif /* __TASK_H */
