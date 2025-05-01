/* USER CODE BEGIN Header */
// 本文件定义了一些常用的数据类型和宏定义
//AUTO,DOWN,ENTER,MENU是5个按键,低电平有效
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "lcd.h"
#include <string.h>
#include "LED.h"
#include "usmart.h"
#include "delay.h"
#include "encoder.h"
#include "tim.h"
#include "wit_c_sdk.h" 
#include "usart.h"
#include "car_config.h"
#include "my_menu.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
extern lcd lcd_desc;
extern lcd_io lcd_io_desc;
extern uint16_t line_buffer[320];
extern u8 Way_Angle;                                                   
extern u8 Flag_front, Flag_back, Flag_Left, Flag_Right, Flag_velocity; 
extern u8 Flag_Stop, Flag_Show;                                        
extern int Motor_Left, Motor_Right;                                    
extern int Temperature;                                                
extern int Voltage, Middle_angle;                                      
extern u8 Mode, CCD_Zhongzhi, CCD_Yuzhi, Lidar_Detect;                 
extern u16 ADV[128];
extern u16 determine; 
extern float Move_X, Move_Z;
extern u8 LD_Successful_Receive_flag;                                            
extern float Angle_Balance, Gyro_Balance, Gyro_Turn;                            
extern u32 Distance;                                                             
extern u8 PID_Send;                                                              
extern u8 Flag_follow, Flag_avoid;                                               
extern float Acceleration_Z;                                                     
extern volatile u8 delay_flag, delay_50;                                         
extern float Balance_Kp, Balance_Kd, Velocity_Kp, Velocity_Ki, Turn_Kp, Turn_Kd; 
extern float Target_Velocity; 
extern volatile float fAcc[3], fGyro[3], fAngle[3];
extern int16_t iMag[3];
extern float Velocity_Left, Velocity_Right; // 左右轮速度，全局变量
extern volatile int Encoder_Left, Encoder_Right; // 左右编码器的脉冲计数
extern volatile int Balance_Pwm, Velocity_Pwm, Turn_Pwm;
extern float Task3_Rotation_Angle_1;     // 任务3中初始从A点对准C点需要顺时针旋转的角度(度)
extern float Task3_Rotation_Angle_2;     // 任务3中C点旋转需要顺时针旋转的角度(度)
extern float Task3_Rotation_Angle_3;     // 任务3中从B到D前需要逆时针旋转的角度(度)
extern int Task3_Steer_Time_C;           // C点openLoopSteering的转向时间参数(中断次数)
extern int Task3_Steer_PWM_C;            // C点openLoopSteering的PWM参数(速度值)
extern int Task3_Steer_Time_D;           // D点openLoopSteering的转向时间参数(中断次数)
extern int Task3_Steer_PWM_D;            // D点openLoopSteering的PWM参数(速度值)
extern int Task3_Move_Forward_Speed;     // 任务3中直线行驶的速度(速度值)
extern u16 forwardBase_PWM;              // 直线行走基础PWM值
extern u16 openLoopSteeringBase_PWM;     // 转向基础PWM值
extern u8 resetTask_flag;                // 是否要重启任务标志位   
extern u32 resetMode_start_time;         // 任务3重置模式开始时间
extern int Task3_line_tracking_Speed;    // 任务3中循迹行驶的速度(速度值)
extern float initialAngle_temp[5];       // 记录初始角度的数组,在不同的时刻记录初始角度,取平均值作为最终初始角度
extern float Task4_Rotation_Angle_1;    // 任务4中初始从A点对准C点需要顺时针旋转的角度(度)
extern float Task4_Rotation_Angle_3;    // 任务4中从B到D前需要逆时针旋转的角度(度)
extern float Task4_Rotation_Angle_4;    // 任务4中后3圈从B到D前需要逆时针旋转的角度(度)
extern int Task4_Steer_Time_C;          // C点openLoopSteering的转向时间参数(中断次数)
extern int Task4_Steer_PWM_C;           // C点openLoopSteering的PWM参数(速度值)
extern int Task4_Steer_Time_D;          // D点openLoopSteering的转向时间参数(中断次数)
extern int Task4_Steer_PWM_D;           // D点openLoopSteering的PWM参数(速度值)
extern int Task4_Cycle_Count;           // 任务4循环执行次数
extern int Task4_Interval;              // 任务间隔时间(毫秒)
extern int Task4_Move_Forward_Speed;    // 任务4中直线行驶的速度(速度值)
extern int Task4_Line_Tracking_Speed;   // 任务4中循迹行驶的速度(速度值)
extern int Task4_B_Point_Delay_Time;    // B点延时时间(毫秒)
extern u8 key_state;                    // 按键状态
extern u8 key_state_last;               // 上次按键状态
extern float initialAngle;              // 初始航向角
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

// 测试函数声明
void SteeringTest_CyclicRotation(void);
// 修改PID参数声明，将float改为u16（放大100倍后使用整数）
extern u16 Steering_Kp, Steering_Ki, Steering_Kd;
extern u16 Steering_Error_Threshold;
extern u16 Steering_Speed;
extern u8 Steering_Completed;
extern u16 Steering_Stable_Count;

// 直线行驶角度修正PID参数（放大100倍）
extern u16 Forward_Kp, Forward_Ki, Forward_Kd;
extern u16 Forward_Error_Threshold;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef const int32_t sc32;
typedef const int16_t sc16;
typedef const int8_t sc8;

typedef __IO int32_t vs32;
typedef __IO int16_t vs16;
typedef __IO int8_t vs8;

typedef __I int32_t vsc32;
typedef __I int16_t vsc16;
typedef __I int8_t vsc8;

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef const uint32_t uc32;
typedef const uint16_t uc16;
typedef const uint8_t uc8;

typedef __IO uint32_t vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t vu8;

typedef __I uint32_t vuc32;
typedef __I uint16_t vuc16;
typedef __I uint8_t vuc8;

// 位带操作,实现51类似的GPIO控制功能
// IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5) + (bitnum << 2))
#define MEM_ADDR(addr) *((volatile unsigned long *)(addr))
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))
// IO口地???映射
#define GPIOA_ODR_Addr (GPIOA_BASE + 12) // 0x4001080C
#define GPIOB_ODR_Addr (GPIOB_BASE + 12) // 0x40010C0C
#define GPIOC_ODR_Addr (GPIOC_BASE + 12) // 0x4001100C
#define GPIOD_ODR_Addr (GPIOD_BASE + 12) // 0x4001140C
#define GPIOE_ODR_Addr (GPIOE_BASE + 12) // 0x4001180C
#define GPIOF_ODR_Addr (GPIOF_BASE + 12) // 0x40011A0C
#define GPIOG_ODR_Addr (GPIOG_BASE + 12) // 0x40011E0C

#define GPIOA_IDR_Addr (GPIOA_BASE + 8) // 0x40010808
#define GPIOB_IDR_Addr (GPIOB_BASE + 8) // 0x40010C08
#define GPIOC_IDR_Addr (GPIOC_BASE + 8) // 0x40011008
#define GPIOD_IDR_Addr (GPIOD_BASE + 8) // 0x40011408
#define GPIOE_IDR_Addr (GPIOE_BASE + 8) // 0x40011808
#define GPIOF_IDR_Addr (GPIOF_BASE + 8) // 0x40011A08
#define GPIOG_IDR_Addr (GPIOG_BASE + 8) // 0x40011E08


#define PAout(n) BIT_ADDR(GPIOA_ODR_Addr, n) // 输出
#define PAin(n) BIT_ADDR(GPIOA_IDR_Addr, n)  // 输入

#define PBout(n) BIT_ADDR(GPIOB_ODR_Addr, n) // 输出
#define PBin(n) BIT_ADDR(GPIOB_IDR_Addr, n)  // 输入

#define PCout(n) BIT_ADDR(GPIOC_ODR_Addr, n) // 输出
#define PCin(n) BIT_ADDR(GPIOC_IDR_Addr, n)  // 输入

#define PDout(n) BIT_ADDR(GPIOD_ODR_Addr, n) // 输出
#define PDin(n) BIT_ADDR(GPIOD_IDR_Addr, n)  // 输入

#define PEout(n) BIT_ADDR(GPIOE_ODR_Addr, n) // 输出
#define PEin(n) BIT_ADDR(GPIOE_IDR_Addr, n)  // 输入

#define PFout(n) BIT_ADDR(GPIOF_ODR_Addr, n) // 输出
#define PFin(n) BIT_ADDR(GPIOF_IDR_Addr, n)  // 输入

#define PGout(n) BIT_ADDR(GPIOG_ODR_Addr, n) // 输出
#define PGin(n) BIT_ADDR(GPIOG_IDR_Addr, n)  // 输入
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void led_toggle(void);
void led_off(void);
void led_on(void);
void led1_toggle(void);
void led1_on(void);
void led1_off(void);
void led2_toggle(void);
void led2_on(void);
void led2_off(void);
void led3_toggle(void);
void led3_on(void);
void led3_off(void);
void all_leds_on(void);
void all_leds_off(void);
void all_leds_toggle(void);
void app_main(void);
void app_fatfs(void);
int fputc(int ch, FILE *f);
void delay_init(uint16_t sysclk);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);
void usart_init(uint32_t bound);
int click(void);
void JY901_init(void);
void print_angle_Handle(void);
void Before_Main(void);
void angleSetWithKey_Handler(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED1_Pin GPIO_PIN_0
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_1
#define LED2_GPIO_Port GPIOC
#define LED3_Pin GPIO_PIN_2
#define LED3_GPIO_Port GPIOC
#define LCD_SCK_Pin GPIO_PIN_5
#define LCD_SCK_GPIO_Port GPIOA
#define LCD_DC_Pin GPIO_PIN_6
#define LCD_DC_GPIO_Port GPIOA
#define LCD_SDA_Pin GPIO_PIN_7
#define LCD_SDA_GPIO_Port GPIOA
#define BIN2_Pin GPIO_PIN_13
#define BIN2_GPIO_Port GPIOE
#define BIN1_Pin GPIO_PIN_14
#define BIN1_GPIO_Port GPIOE
#define AIN2_Pin GPIO_PIN_15
#define AIN2_GPIO_Port GPIOE
#define AIN1_Pin GPIO_PIN_10
#define AIN1_GPIO_Port GPIOB
#define AUTO_Pin GPIO_PIN_11
#define AUTO_GPIO_Port GPIOB
#define MENU_Pin GPIO_PIN_13
#define MENU_GPIO_Port GPIOB
#define UP_Pin GPIO_PIN_15
#define UP_GPIO_Port GPIOB
#define ENTER_Pin GPIO_PIN_9
#define ENTER_GPIO_Port GPIOD
#define GND_Pin GPIO_PIN_11
#define GND_GPIO_Port GPIOD
#define DOWN_Pin GPIO_PIN_13
#define DOWN_GPIO_Port GPIOD
#define I2C_CLK_Pin GPIO_PIN_6
#define I2C_CLK_GPIO_Port GPIOD
#define I2C_Data_Pin GPIO_PIN_7
#define I2C_Data_GPIO_Port GPIOD
#define LCD_RST_Pin GPIO_PIN_3
#define LCD_RST_GPIO_Port GPIOB
#define LCD_BLK_Pin GPIO_PIN_4
#define LCD_BLK_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_5
#define LCD_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
