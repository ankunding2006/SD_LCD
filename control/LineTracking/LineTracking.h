#ifndef __LINETRACKING_H
#define __LINETRACKING_H

#include "main.h"
#include "usart.h"

// 巡线数据包定义
#define FRAME_HEADER 0xFD
#define FRAME_FOOTER 0xFE
#define LINE_DATA_LENGTH 7  // 完整数据包长度

// UART2接收缓冲区
#define UART2_RX_BUFFER_SIZE 1
extern uint8_t g_uart2_rx_buffer[UART2_RX_BUFFER_SIZE];  // UART2接收缓冲区

// 路径选择枚举
typedef enum {
    PATH_LEFT = 0,
    PATH_RIGHT = 1,
    PATH_MIDDLE = 2
} PathChoice_t;

// 巡线数据结构
typedef struct {
    uint16_t lineOffset;   // 线偏移量 (0-360)
    uint16_t angleLeft;    // 左侧角度信息
    uint8_t crossFlag;     // 岔路标志 (0:无岔路 1:有岔路)
    uint8_t stopFlag;      // 停止标志 (0:不停 1-3:停止类型)
    uint8_t dataReady;     // 数据就绪标志
} LineTrackData_t;

// 函数声明
void LineTracking_Init(void);
void LineTracking_ProcessData(uint8_t rxData);
int LineTracking_CalculateTurn(void); 
void LineTracking_SetPathChoice(PathChoice_t choice);
PathChoice_t LineTracking_GetPathChoice(void);
LineTrackData_t* LineTracking_GetData(void);
void LineTracking_Task(void);

extern LineTrackData_t lineData;
extern PathChoice_t currentPathChoice;

#endif /* __LINETRACKING_H */
