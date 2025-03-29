#include "linetracking.h"
#include "control.h"
#include <stdio.h>

// 全局变量定义
LineTrackData_t lineData = {0};
PathChoice_t currentPathChoice = PATH_MIDDLE; // 默认选择中间路径
uint8_t rxBuffer[LINE_DATA_LENGTH] = {0};
uint8_t rxIndex = 0;
uint8_t isReceiving = 0;

// UART2独立接收缓冲区
uint8_t g_uart2_rx_buffer[UART2_RX_BUFFER_SIZE] = {0};


/**
  * @brief  初始化巡线功能
  * @param  无
  * @retval 无
  */
void LineTracking_Init(void)
{
    // 注：UART2已在main.c中初始化
    printf("LineTracking module initialized\r\n");
}

/**
  * @brief  处理接收到的巡线数据
  * @param  rxData: 接收到的字节数据
  * @retval 无
  */
void LineTracking_ProcessData(uint8_t rxData)
{
    // 帧头检测
    if(rxData == FRAME_HEADER && !isReceiving)
    {
        rxIndex = 0;
        isReceiving = 1;
        rxBuffer[rxIndex++] = rxData;
        return;
    }
    
    // 接收数据
    if(isReceiving)
    {
        rxBuffer[rxIndex++] = rxData;
        
        // 检查是否接收完整
        if(rxIndex >= LINE_DATA_LENGTH)
        {
            isReceiving = 0;
            
            // 验证帧尾
            if(rxBuffer[LINE_DATA_LENGTH-1] == FRAME_FOOTER)
            {
                // 解析数据包
                lineData.lineOffset = (rxBuffer[1] << 8) | rxBuffer[2];  // 高字节在前
                lineData.angleLeft = (rxBuffer[3] << 8) | rxBuffer[4];
                lineData.crossFlag = rxBuffer[5];
                lineData.stopFlag = rxBuffer[6];
                lineData.dataReady = 1;
                
                // 调试输出
                printf("Line Data: Offset=%d, Angle=%d, Cross=%d, Stop=%d\r\n", 
                       lineData.lineOffset, lineData.angleLeft, 
                       lineData.crossFlag, lineData.stopFlag);
            }
            rxIndex = 0;
        }
    }
}

/**
  * @brief  根据巡线数据计算转向值
  * @param  无
  * @retval 转向PWM值 (-1000 到 1000)
  */
int LineTracking_CalculateTurn(void)
{
    int turnValue = 0;
    
    if(!lineData.dataReady)
        return 0;
    
    // 基于屏幕中心(180)计算偏移量
    int centerOffset = (int)lineData.lineOffset - 180;
    
    // PID参数（可根据实际调整）
    float Kp = 10.0f;  // 比例系数
    float Kd = 5.0f;   // 微分系数
    
    static int lastOffset = 0;
    int differential = centerOffset - lastOffset;
    lastOffset = centerOffset;
    
    // 计算转向值
    turnValue = (int)(Kp * centerOffset + Kd * differential);
    
    // 限制转向值范围
    if(turnValue > 1000) turnValue = 1000;
    if(turnValue < -1000) turnValue = -1000;
    
    // 特殊情况处理
    if(lineData.crossFlag && currentPathChoice != PATH_MIDDLE)
    {
        // 在岔路口时根据选择路径调整转向值
        if(currentPathChoice == PATH_LEFT)
        {
            turnValue -= 500; // 向左偏移更多
        }
        else if(currentPathChoice == PATH_RIGHT)
        {
            turnValue += 500; // 向右偏移更多
        }
    }
    
    return turnValue;
}

/**
  * @brief  设置路径选择
  * @param  choice: 路径选择（左/右/中）
  * @retval 无
  */
void LineTracking_SetPathChoice(PathChoice_t choice)
{
    currentPathChoice = choice;
    printf("Path choice set to: %d\r\n", choice);
}

/**
  * @brief  获取当前路径选择
  * @param  无
  * @retval 当前路径选择
  */
PathChoice_t LineTracking_GetPathChoice(void)
{
    return currentPathChoice;
}

/**
  * @brief  获取巡线数据结构体指针
  * @param  无
  * @retval 巡线数据结构体指针
  */
LineTrackData_t* LineTracking_GetData(void)
{
    return &lineData;
}

/**
  * @brief  巡线主任务，需要在主循环中调用
  * @param  无
  * @retval 无
  */
void LineTracking_Task(void)
{
    // 检查停止标志
    if(lineData.dataReady && lineData.stopFlag)
    {
        // 如果检测到停止线，执行停车操作
        Flag_Stop = 1;
        printf("Stop line detected, Flag_Stop=%d\r\n", Flag_Stop);
    }
}
