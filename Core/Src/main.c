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
#include "dma.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// fputc function for printf
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}
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
  MX_DMA_Init();
  MX_SDIO_SD_Init();
  MX_USART1_UART_Init();
  MX_FATFS_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  lcd_init_dev(&lcd_desc, LCD_2_00_INCH, LCD_ROTATE_270);

  lcd_print(&lcd_desc, 0, 10, "> X Pulse");
  lcd_print(&lcd_desc, 0, 30, "> STM32 lcd demo");
  lcd_print(&lcd_desc, 0, 50, "> LCD 2.0 inch 320x240");
  lcd_print(&lcd_desc, 0, 70, "> 2024/9/1");

  led_off();
  app_main();
  lcd_set_font(&lcd_desc, FONT_3216,YELLOW, BLACK);
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
// LED1 控制函数
void led1_toggle(void)
{
  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
}

void led1_on(void)
{
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
}

void led1_off(void)
{
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
}

// LED2 控制函数
void led2_toggle(void)
{
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
}

void led2_on(void)
{
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}

void led2_off(void)
{
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
}

// LED3 控制函数
void led3_toggle(void)
{
  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
}

void led3_on(void)
{
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
}

void led3_off(void)
{
  HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
}

// 全部LED控制函数
void all_leds_on(void)
{
  led1_on();
  led2_on();
  led3_on();
}

void all_leds_off(void)
{
  led1_off();
  led2_off();
  led3_off();
}

void all_leds_toggle(void)
{
  led1_toggle();
  led2_toggle();
  led3_toggle();
}

// 为了兼容原来的函数，保留这些函数但修改为使用LED1
void led_toggle(void)
{
  led1_toggle();
}

void led_on(void)
{
  led1_on();
}

void led_off(void)
{
  led1_off();
}

// 添加app_main函数的实现
void app_main(void)
{
  printf("App main started\r\n");
  
  // 初始化演示 - 循环点亮所有LED
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
  
  // 启动舵机PWM输出
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
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
