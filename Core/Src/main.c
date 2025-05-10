/* USER CODE BEGIN Header */
/*
本项目为 STM32F407 系列 MCU 设计的自动行驶小车，旨在实现路径跟踪和精准停车功能。

题目功能需求概述：

设计一个基于 STM32F407 系列 MCU 控制的自动行驶小车，在指定场地完成以下路径任务：


场地及黑线分布详细描述

---

 1. 场地基础信息
- 尺寸：长方形，长220cm × 宽120cm，白色哑光喷绘布，水平铺设于地面。
- 黑线特征：
  - 仅有 两个对称的黑色半圆弧线，线宽约 1.8cm，半径 40cm。
  - 半圆弧的 四个端点（顶点） 定义为 A、B、C、D，分别位于场地四角或关键位置。

---

 2. 黑线布局与顶点位置
- 半圆弧1（右侧半圆）：
  - 连接 B点（右上顶点） 和 C点（右下顶点），形成右侧半圆弧黑线。
- 半圆弧2（左侧半圆）：
  - 连接 D点（左下顶点） 和 A点（左上顶点），形成左侧半圆弧黑线。
- 顶点位置示意：
  - A：左上角顶点（坐标示例：长边左端，宽边顶端）。
  - B：右上角顶点（长边右端，宽边顶端）。
  - C：右下角顶点（长边右端，宽边底端）。
  - D：左下角顶点（长边左端，宽边底端）。

---

 3. 黑线引导区域与无引导区域
- 有黑线引导的区域：
  - 右侧半圆弧（B→C）：从右上角 B 沿右侧半圆弧黑线行驶至右下角 C。
  - 左侧半圆弧（D→A）：从左下角 D 沿左侧半圆弧黑线行驶至左上角 A。
- 无黑线引导的区域：
  - 直线段 A→B：左上角 A 到右上角 B 的顶部横向直线，无黑线。
  - 直线段 C→D：右下角 C 到左下角 D 的底部横向直线，无黑线。
  - 其他直线路径：如任务中涉及的 B→C、D→A 等非弧线段，均无黑线。

---

 4. 路径任务与黑线关系
- 任务1（A→B）：
  - 直线行驶从 A 到 B，无黑线，需依赖编码器/陀螺仪自主导航。
- 任务2（A→B→C→D→A）：
  - B→C：沿右侧半圆弧黑线循迹。
  - D→A：沿左侧半圆弧黑线循迹。
  - 其他路径（A→B、C→D）无引导。
- 任务3（A→C→B→D→A）：
  - C→B：沿右侧半圆弧黑线逆方向循迹。
  - D→A：沿左侧半圆弧黑线循迹。
  - 其他路径（A→C、B→D）无引导。



 示意图简化理解
A (左上)———————————→ B (右上)

左侧半圆弧（D→A）  右侧半圆弧（B→C）

D (左下) ←——————————— C (右下)
- 箭头方向：表示半圆弧黑线走向（实际为半圆路径）。
- 横向直线段（A→B、C→D）无黑线，需自主导航。

---

 任务 1（20 分）
- 功能：
  小车从 A 点出发，沿直线行驶至 B 点 后停车，全程用时不超过 15 秒。
  - 触发条件：到达 B 点时需有声（如蜂鸣器）光（如 LED）提示。

---

 任务 2（20 分）
- 功能：
  小车从 A 点 出发，按以下路径行驶：
  1. 直线行驶至 B 点。
  2. 沿 半圆弧黑线 从 B 行驶至 C 点。
  3. 直线行驶至 D 点。
  4. 沿另一 半圆弧黑线 从 D 返回 A 点 停车。
  - 触发条件：每经过 A/B/C/D 点时需有声光提示。
  - 总时间：不超过 30 秒。

---

 任务 3（30 分）
- 功能：
  小车从 A 点 出发，按以下路径行驶：
  1. 直线行驶至 C 点。
  2. 沿 半圆弧黑线 从 C 行驶至 B 点。
  3. 直线行驶至 D 点。
  4. 沿另一 半圆弧黑线 从 D 返回 A 点 停车。
  - 触发条件：每经过 A/B/C/D 点时需有声光提示。
  - 总时间：不超过 40 秒。

---

 任务 4（30 分）
- 功能：
  按 任务 3 的路径 连续自动行驶 4 圈，总用时越短越好。

---

 核心挑战
1. 导航方式：
   - 直线段：无黑线引导，需通过 编码器 + 陀螺仪 实现航位推算，保持直线行驶。
   - 圆弧段：沿黑线循迹，依赖 红外传感器阵列 实时调整转向。

2. 精准停靠与触发：
   - 需在 A/B/C/D 顶点 精确停车或触发提示，要求投影覆盖顶点位置。
   - 可结合 编码器距离计算 或 黑线端点检测 触发声光信号。

3. 时间与稳定性：
   - 在高速行驶中维持控制精度，避免脱线或超时。
   - 需优化 PID 控制算法 和路径切换逻辑。

---

 技术实现要点
- 传感器：灰度传感器（循迹）、编码器（测距）、陀螺仪（防偏航）。
- 控制算法：
  - 直线段：PID 控制电机速度 + 陀螺仪纠偏。
  - 圆弧段：PID 调节差速转向，保持沿黑线行驶。
- 路径规划：根据任务动态切换导航模式（直线/循迹）。
- 触发逻辑：通过编码器累计距离或黑线端点检测触发声光提示。
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
#include "gray_detection.h"
#include <stdlib.h>
#include "encoder.h"
#include "control.h"
#include "cot_menu.h"
#include "lcd.h"
#include "my_menu.h"
#include "soft_i2c.h"
#include "nchd12.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/**************enum define****************/
enum currentPosition
{
  /*Line*/
  AB,
  BC,
  CD,
  DA,
  /* spesific point*/
  A,
  B,
  C,
  D
};
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
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  delay_init(168);     /* 延时初始*/
  usart_init(115200);  /* 串口初始化为115200 */
  usmart_dev.init(84); /* USMART初始*/
  lcd_init_dev(&lcd_desc, LCD_2_00_INCH, LCD_ROTATE_270);
  CarState_Init(); // 初始化汽车状态结构体
  grey_sensor_Init(); // 初始化灰度传感器

  lcd_print(&lcd_desc, 0, 10, "> X Pulse");
  lcd_print(&lcd_desc, 0, 30, "> STM32 lcd demo");
  lcd_print(&lcd_desc, 0, 50, "> LCD 2.0 inch 320x240");
  lcd_print(&lcd_desc, 0, 70, "> 2024/9/1");

  led_off();
  //!app_main();
  lcd_set_font(&lcd_desc, FONT_3216, YELLOW, BLACK);
  Menu_Init(); // 初始化菜单系统
#if GYROSCOPE_ON_DEFAULT == 1
  JY901_init(); // 初始化JY901传感器
#endif
  HAL_TIM_Base_Start_IT(&htim6);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    interface_Handler(); // 菜单界面处理函数
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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

#ifdef USE_FULL_ASSERT
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
