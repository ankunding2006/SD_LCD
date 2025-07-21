#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

// 全局变量声明
extern volatile uint16_t room_num;    // 病房布局/编号
extern volatile uint16_t room_target; // 目标病房
extern volatile uint8_t usart2_rx_buffer;

// 函数声明
int8_t
set_motor_speed(int16_t left_speed, int16_t right_speed, uint8_t acc);
void motor_speed_task_handler(void);
uint8_t line_following_task(void);
uint8_t open_loop_steering_control(int16_t steerTime, int16_t Rotate_Speed, int16_t Rotate_Speed_Base);
void visual_reception_init(void);
void visual_process_command(void);
void uart_rx_handler(uint8_t rx_data);

#endif // CONTROL_H
