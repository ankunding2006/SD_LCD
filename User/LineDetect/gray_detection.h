#ifndef __GRAY_DETECTION_H
#define __GRAY_DETECTION_H
#include "main.h"
#include <stdint.h>

#define u16 uint16_t 
#define u8 uint8_t 
#define u32 uint32_t 

#define turn_PWM_Limit 1000

typedef struct
{
    uint8_t bit1 : 1;
    uint8_t bit2 : 1;
    uint8_t bit3 : 1;
    uint8_t bit4 : 1;
    uint8_t bit5 : 1;
    uint8_t bit6 : 1;
    uint8_t bit7 : 1;
    uint8_t bit8 : 1;
    uint8_t bit9 : 1;
    uint8_t bit10 : 1;
    uint8_t bit11 : 1;
    uint8_t bit12 : 1;
    uint8_t bit13 : 1;
    uint8_t bit14 : 1;
    uint8_t bit15 : 1;
    uint8_t bit16 : 1;
} gray_flags;

typedef union
{
    gray_flags gray;
    uint16_t state;
} _gray_state;

void gpio_input_init(void);
void gpio_input_check_channel_12_linewidth_10mm(void);
void gpio_input_check_channel_12_linewidth_20mm(void);
void grey_sensor_Read(void);
void grey_sensor_Init(void);
void grey_sensorData_print(void);
uint16_t pca9555_read_bit12(uint8_t slave_num);
uint8_t i2c_CheckDevice(uint8_t _Address);
extern int IsCertainGraySenorsAcrived(uint8_t start_graySensor, uint8_t end_graySensor);
int Calculate_Turn_Value(void);

extern float gray_status[2], gray_status_backup[2][20];
extern uint32_t gray_status_worse;
extern _gray_state gray_state;
extern u16 scaleFactor;

#endif
