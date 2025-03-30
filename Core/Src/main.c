/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include "encoder.h" 
#include "control.h" 
// 你好的
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
u8 Way_Angle = 1;                                                                                                   // 获取角度的算法，1：四元数  2：卡尔曼  3：互补滤??
u8 Flag_front, Flag_back, Flag_Left, Flag_Right, Flag_velocity = 2;                                                 // 蓝牙遥控相关的变??
u8 Flag_Stop = 1, Flag_Show = 0;                                                                                    // 电机停止标志位和显示标志??  默认停止 显示打开
int Motor_Left, Motor_Right;                                                                                        // 电机PWM变量 应是Motor?? 向Moto致敬
int Temperature;                                                                                                    // 温度变量
int Voltage, Middle_angle;                                                                                          // 电池电压采样相关的变??
float Angle_Balance, Gyro_Balance, Gyro_Turn;                                                                       // 平衡倾角 平衡??螺仪 转向??螺仪
u8 LD_Successful_Receive_flag;                                                                                      // 雷达成功接收数据标志??
u8 Mode = 0;                                                                                                        // 模式选择，默认是普???的控制模式
u8 CCD_Zhongzhi, CCD_Yuzhi;                                                                                         // CCD中???和阈???
u16 ADV[128] = {0};                                                                                                 // 存放CCD的数据的数组
u16 determine;                                                                                                      // 雷达跟随模式的一个标志位
float Move_X, Move_Z;                                                                                               // 遥控控制的???度
u32 Distance;                                                                                                       // 超声波测??
u8 PID_Send;                                                                                                        // 调参相关变量
u8 Flag_follow = 0, Flag_avoid = 0;                                                                                 // 超声波跟随???超声波壁障标志??
float Acceleration_Z;                                                                                               // Z轴加速度??
volatile u8 delay_flag, delay_50;                                                                                   // 提供延时的变??
float Balance_Kp = 25500, Balance_Kd = 135, Velocity_Kp = 16000, Velocity_Ki = 120, Turn_Kp = 17000, Turn_Kd = 100; // PID参数（放??100倍）
u8 Sensor_Left = 0, Sensor_MiddleLeft = 0, Sensor_Middle = 0, Sensor_MiddleRight = 0, Sensor_Right = 0;             // 传感器状??
float Sensor_Kp = 640, Sensor_KI = 2.1, Sensor_Kd = 115;                                                           // 传感器的PID参数（放??100倍）
float Target_Velocity = 16;     
u16 ZoomRatio=1000;           
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint16_t line_buffer[320];
uint8_t angle = 0; // 添加舵机角度变量

lcd_io lcd_io_desc = {
    .spi = &hspi1,
    .rst = {LCD_RST_GPIO_Port, LCD_RST_Pin, 0},
    .bl = {LCD_BLK_GPIO_Port, LCD_BLK_Pin, 0},
    .cs = {LCD_CS_GPIO_Port, LCD_CS_Pin, 0},
    .dc = {LCD_DC_GPIO_Port, LCD_DC_Pin, 0},
    //.te  = { /* TE */ }
};

lcd lcd_desc = {
    .io = &lcd_io_desc,
    .line_buffer = line_buffer,
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Before_Main(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  MX_TIM6_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_TIM9_Init();
  /* USER CODE BEGIN 2 */
  delay_init(168);     /* 延时初始*/
  usart_init(115200);  /* 串口初始化为115200 */
  usmart_dev.init(84); /* USMART初始*/
  lcd_init_dev(&lcd_desc, LCD_2_00_INCH, LCD_ROTATE_270);
  
  // 初始化编码器
  Encoder_Init_TIM3();  // 打开左轮编码器?
  Encoder_Init_TIM5();  // 打开右轮编码器?

  // 设置默认控制参数
  Middle_angle = 0;    // 初始平衡角度设定
  Target_Velocity = 16; // 目标速度
  ZoomRatio = 1000;     // 转向缩放比例

  lcd_print(&lcd_desc, 0, 10, "> X Pulse");
  lcd_print(&lcd_desc, 0, 30, "> STM32 lcd demo");
  lcd_print(&lcd_desc, 0, 50, "> LCD 2.0 inch 320x240");
  lcd_print(&lcd_desc, 0, 70, "> 2024/9/1");

  led_off();
  app_main();
  lcd_set_font(&lcd_desc, FONT_3216, YELLOW, BLACK);
  LineTracking_Init();
  Before_Main();
  

  HAL_TIM_Base_Start_IT(&htim6);   
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

// 添加app_main函数
void Before_Main(void)
{
  printf("App main started\r\n");

  all_leds_off();
  HAL_Delay(500);
  led1_on();
  HAL_Delay(500);
  led1_off();
  led2_on();
  HAL_Delay(500);
  led2_off();
  led3_on();
  HAL_Delay(500);
  led3_off();
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
