/***********************************************
这原来是一个平衡车的控制代码，现在改成了一个小车的控制代码
并且之前这个代码是在stm32f103系列的开发版运行的，现在改成
了stm32f407系列的开发板运行

现在暂时没有使用mpu6050，并且也没有使用蓝牙模块，所以这两个
模块的代码都没有用到,但是这两个模块的代码还是保留在这个文件
中，以后可能会用到.

在现阶段我只想完成基本的小车控制，所以我只需要使用到编码器和
电机驱动模块，所以我只需要使用到这两个模块的代码，其他的模块
的代码都不需要使用，所以我会把这些模块的代码删除掉，只保留编码
器和电机驱动模块的代码
***********************************************/
#include "control.h"

//#define NOLINEDETECT // 调试模式，不使用灰度传感器

// 在文件开头添加全局变量定义
float Velocity_Left, Velocity_Right; // 左右轮速度，全局变量

/**************************************************************************
函数功能：Control function
入口参数：none
Output  : none
函数功能：所有的控制代码都在这里面
		 5ms外部中断由定时器TIM6产生，严格保证采样和数据处理的时间同步
入口参数：无
		 严格保证采样和数据处理的时间同步
入口参数：无
返回  值：无
**************************************************************************/

volatile int Encoder_Left, Encoder_Right; // 左右编码器的脉冲计数
volatile int Balance_Pwm, Velocity_Pwm, Turn_Pwm=0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6)
	{
		Encoder_Left = Read_Encoder(3);							// 读取左轮编码器的值，前进为正，后退为负
		Encoder_Right = Read_Encoder(5);						// 修改为TIM5，前进为正，后退为负
																// 左轮A相接TIM2_CH1,右轮A相接TIM4_CH2,故这里两个编码器的极性相同
		Get_Velocity_Form_Encoder(Encoder_Left, Encoder_Right); // 编码器读数转速度（mm/s）                                          				
		int actual_velocity = Velocity(Encoder_Left, Encoder_Right); // 获取速度控制的PWM,速度控制
	    Velocity(Encoder_Left, Encoder_Right);
 
		grey_sensor_Read(); // 读取灰度传感器数据
		#ifdef NOLINEDETECT // 调试模式，不使用灰度传感器
		Turn_Pwm = 0; // 调试模式，不使用灰度传感器
		#else
		Turn_Pwm = Calculate_Turn_Pwm(); // 计算转向PWM值
		#endif

		// 使用计算出的实际速度，不修改Target_Velocity
		Motor_Left = actual_velocity - Turn_Pwm;
		Motor_Right = actual_velocity + Turn_Pwm;
		if (Flag_Stop == 1) 
			Set_Pwm(0, 0);
		else
			Set_Pwm(Motor_Left, Motor_Right); // 赋值给PWM寄存器
	}
}


/**************************************************************************
Function: Speed PI control
Input   : encoder_left：Left wheel encoder reading；encoder_right：Right wheel encoder reading
Output  : Speed control PWM
函数功能：速度控制PWM
入口参数：encoder_left：左轮编码器读数；encoder_right：右轮编码器读数
返回  值：速度控制PWM
**************************************************************************/
// 修改前进后退速度，请修改Target_Velocity，比如，改成60就比较慢了
int Velocity(int encoder_left, int encoder_right)
{
	volatile static float velocity, Encoder_Least, Encoder_bias, Movement = 0;
	volatile static float Encoder_Integral = 0;

	//================速度PI控制器=====================//
	Encoder_Least = Target_Velocity * 2 - (encoder_left + encoder_right); // 获取最新速度偏差=目标速度-测量速度（左右编码器之和）
	Encoder_bias *= 0.86f;												  // 添加f后缀，指定为单精度浮点数
	Encoder_bias += Encoder_Least * 0.14f;								  // 添加f后缀，指定为单精度浮点数
	Encoder_Integral += Encoder_bias;									  // 积分出位移 积分时间：10ms
	if (Encoder_Integral > 4000)
		Encoder_Integral = 4000; // 积分限幅
	if (Encoder_Integral < -4000)
		Encoder_Integral = -4000;														 // 积分限幅
	velocity = Encoder_bias * Velocity_Kp / 100 + Encoder_Integral * Velocity_Ki / 100; // 速度控制
	if (Flag_Stop == 1)
		Encoder_Integral = 0; // 电机关闭后清除积分
	Velocity_Pwm = velocity;
	return velocity;
}


/**************************************************************************
函数功能：Assign to PWM register
Input   : motor_left：Left wheel PWM；motor_right：Right wheel PWM
Output  : none
函数功能：赋值给PWM寄存器
入口参数：左轮PWM、右轮PWM
返回  值：无
**************************************************************************/
void Set_Pwm(int motor_left, int motor_right)
{
	if (motor_left < 0) {
		HAL_GPIO_WritePin(AIN1_GPIO_PORT, AIN1_GPIO_PIN, GPIO_PIN_RESET); // AIN1 = 0
		HAL_GPIO_WritePin(AIN2_GPIO_PORT, AIN2_GPIO_PIN, GPIO_PIN_SET);   // AIN2 = 1
	} else {
		HAL_GPIO_WritePin(AIN1_GPIO_PORT, AIN1_GPIO_PIN, GPIO_PIN_SET);   // AIN1 = 1
		HAL_GPIO_WritePin(AIN2_GPIO_PORT, AIN2_GPIO_PIN, GPIO_PIN_RESET); // AIN2 = 0
	}
	PWMA = myabs(motor_left);
	if (motor_right < 0) {
		HAL_GPIO_WritePin(BIN1_GPIO_PORT, BIN1_GPIO_PIN, GPIO_PIN_RESET); // BIN1 = 0
		HAL_GPIO_WritePin(BIN2_GPIO_PORT, BIN2_GPIO_PIN, GPIO_PIN_SET);   // BIN2 = 1
	} else {
		HAL_GPIO_WritePin(BIN1_GPIO_PORT, BIN1_GPIO_PIN, GPIO_PIN_SET);   // BIN1 = 1
		HAL_GPIO_WritePin(BIN2_GPIO_PORT, BIN2_GPIO_PIN, GPIO_PIN_RESET); // BIN2 = 0
	}
	PWMB = myabs(motor_right);
}
/**************************************************************************
Function: PWM limiting range
Input   : IN：Input  max：Maximum value  min：Minimum value
Output  : Output
函数功能：限制PWM赋值
入口参数：IN：输入参数  max：限幅最大值  min：限幅最小值
返回  值：限幅后的值
**************************************************************************/
int PWM_Limit(int IN, int max, int min)
{
	int OUT = IN;
	if (OUT > max)
		OUT = max;
	if (OUT < min)
		OUT = min;
	return OUT;
}


/**************************************************************************
Function: Absolute value function
Input   : a：Number to be converted
Output  : unsigned int
函数功能：绝对值函数
入口参数：a：需要计算绝对值的数
返回  值：无符号整型
**************************************************************************/
int myabs(int a)
{
	int temp;
	if (a < 0)
		temp = -a;
	else
		temp = a;
	return temp;
}


/**************************************************************************
Function: Encoder reading is converted to speed (mm/s)
Input   : none
Output  : none
函数功能：编码器读数转换为速度（mm/s）
入口参数：无
返回  值：无
**************************************************************************/
void Get_Velocity_Form_Encoder(int encoder_left, int encoder_right)
{
	float Rotation_Speed_L, Rotation_Speed_R; // 电机转速  转速=编码器读数（5ms每次）*读取频率/倍频数/减速比/编码器精度
	Rotation_Speed_L = encoder_left * Control_Frequency / EncoderMultiples / Reduction_Ratio / Encoder_precision;
	Velocity_Left = Rotation_Speed_L * PI * Diameter_67; // 添加f后缀，指定为单精度浮点数
	Rotation_Speed_R = encoder_right * Control_Frequency / EncoderMultiples / Reduction_Ratio / Encoder_precision;
	Velocity_Right = Rotation_Speed_R * PI * Diameter_67; // 添加f后缀，指定为单精度浮点数
}


/**
 * @brief Flag_Stop取反
 * @retval None
 */
void toggle_Flag_Stop(void)
{
    Flag_Stop = !Flag_Stop;
}

/**
 * @brief 关闭TIM6中断或开启TIM6中断(反转TIM6中断)
 * @retval None
 */
void HAL_TIM6_toggle_IT(void)
{
	if (HAL_TIM_Base_GetState(&htim6) == HAL_TIM_STATE_READY) {
		HAL_TIM_Base_Start_IT(&htim6); // 开启TIM6中断
	} else {
		HAL_TIM_Base_Stop_IT(&htim6); // 关闭TIM6中断
	}
}


/**
 * @brief 速度控制函数
 * @note 速度控制函数，修改Target_Velocity的值来改变速度
 * @param Target_Velocity 目标速度
 * @return None
 * 
 */
void Set_Target_Velocity(int Control_Target_Velocity)
{
	// 设置目标速度
	if (Control_Target_Velocity > 100) Control_Target_Velocity = 100; // 限制最大速度为100
	if (Control_Target_Velocity < -100) Control_Target_Velocity = -100; // 限制最小速度为-100
    Target_Velocity = Control_Target_Velocity;
}
