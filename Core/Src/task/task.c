#include "main.h"

/**
 * @brief  定时器周期溢出回调函数
 * @param  htim: 定时器句柄
 * @retval 无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM12)
  {
    // TODO: 定时器周期溢出回调函数
  }
}
