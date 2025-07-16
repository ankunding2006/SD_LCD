#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

uint8_t line_following_task(void);
uint8_t open_loop_steering_control(int16_t angle, int16_t PWM_Value, int16_t PWM_Base);
void set_motor_speed(int16_t left_speed, int16_t right_speed, uint8_t acc);

#endif // CONTROL_H
