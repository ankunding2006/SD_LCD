#ifndef __TEST_H
#define __TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "LED.h"

// 测试函数声明
void SteeringTest_CyclicRotation(void);
void Test_Handler(void);
void turnToAbsoluteAngle_TEST_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_H */
