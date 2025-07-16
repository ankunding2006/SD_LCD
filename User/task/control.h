#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

void set_motor_speed(int16_t left_speed, int16_t right_speed, uint8_t acc);
uint8_t line_following_task(void);
uint8_t open_loop_steering_control(int16_t angle, int16_t PWM_Value, int16_t PWM_Base);

// 嫌弃全局变量可以把extern放到需要的文件的.c里面
extern uint8_t usart2_rx_buffer;
extern uint8_t rx_nums;
void visual_reception_init(void);
uint8_t visual_process_command(void);

#endif // CONTROL_H
