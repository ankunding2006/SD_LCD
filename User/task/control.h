#ifndef __CONTROL_H
#define __CONTROL_H

#include "main.h"

// 函数声明
uint8_t line_following_task(void);
uint8_t open_loop_steering_control(int16_t steerTime, int16_t Rotate_Speed, int16_t Rotate_Speed_Base);
uint8_t open_loop_reversing_control(uint16_t reverseTime, int16_t Reverse_Speed, int16_t Turn_Speed);
void visual_reception_init(void);
uint8_t visual_process_command(void);
int8_t set_motor_speed(int16_t left_speed, int16_t right_speed, uint8_t acc);
void motor_speed_task_handler(void);
void control_uart_tx_cplt_callback(UART_HandleTypeDef *huart);

#endif /* __CONTROL_H */
