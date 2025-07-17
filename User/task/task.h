#ifndef TASK_H
#define TASK_H

#define CAR_BASE_SPEED 100
#define CAR_MAX_SPEED 400

uint8_t Car_To_Crossing(uint8_t crossing_num);
void Car_To_Crossing_Reset(void);
uint8_t Car_To_Room(uint8_t room_num);
uint8_t Car_To_Room_1_2(uint8_t room_num);
uint8_t Car_To_Room_3_4(uint8_t room_num);
uint8_t Car_To_Room_5_8(uint8_t room_num);

#endif // TASK_H
