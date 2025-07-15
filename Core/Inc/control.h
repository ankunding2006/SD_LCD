#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

uint8_t line_following_task(void);
uint8_t open_loop_steering_control(int16_t angle, int16_t PWM_Value, int16_t PWM_Base);

#endif // CONTROL_H
