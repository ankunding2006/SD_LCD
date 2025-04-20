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
		#ifndef TEST_MODE // 测试模式
		lineTracking_Handler();  
		#else
			Test_Handler(); // 测试模式下的处理函数
		#endif
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

/**
 * @brief Get the Heading Angle object
 * @note 返回值为当前航向角度,且分正负,单位为度,逆时旋转小车(左转)
 * 航向角增大,顺时针旋转小车(右转)航向角减小 
 * 当车头指向正西方向时候返回值为0度,正北方向为-90度,
 * 正南方向为90度,使用这个函数时需要注意在经过正东方向时
 * 航向角会发生从+180到-180的变化,所以在使用时需要注意这个问题
 * @param None
 * @return float 
 */
float getHeadingAngle(void)
{
	return fAngle[2]; // 返回当前航向角度
}


/**
 * @brief 这个函数用来控制小车进行循迹
 * @note 这个函数是小车循迹的主函数，主要是通过灰度传感器来控制小车的行驶方向
 * 这个函数主要来控制小车走过AD,BC这两条曲线
 * @return 如果完成了循迹(即所有传感器都返回白色)就返回1,否则返回0 
 */
int lineTracking_Handler(void)
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
	Turn_Pwm = Calculate_Turn_Pwm(); // 计算转向PWM值,如果所有传感器都返回白色，返回1
	if (Turn_Pwm==INT16_MIN)
	{
		Set_Pwm(0, 0); // 如果没有传感器检测到黑线，停止电机
		return 1; // 返回1，表示完成循迹
	}
	#endif

	// 使用计算出的实际速度，不修改Target_Velocity
	Motor_Left = actual_velocity - Turn_Pwm;
	Motor_Right = actual_velocity + Turn_Pwm;
	if (Flag_Stop == 1) 
		Set_Pwm(0, 0);
	else
		Set_Pwm(Motor_Left, Motor_Right); // 赋值给PWM寄存器
	return 0; // 返回0，表示没有完成循迹,正在循迹中
}

/**
 * @brief Local steering control function
 * @note This function is used to control the steering of the vehicle
 * 通过pid的方式使小车转过指定的角度
 * @param angle 小车目标应该转过的角度,逆时针为正,顺时针为负
 * @return u8 1:完成 0:小车正在转向中
 */
u8 localSteeringControl_Handler(float angle)
{
    static float targetAngle = 0;      // 目标角度
    static float lastError = 0;        // 上一次误差
    static float integral = 0;         // 积分项
    static float startAngle = 0;       // 开始角度
    static u8 isInitialized = 0;       // 初始化标志
    
    float currentAngle = getHeadingAngle();   // 获取当前角度
    float error, derivative, output;
    
    // 初始化，记录起始角度并计算目标角度（相对角度转换为绝对角度）
    if(isInitialized == 0) {
        startAngle = currentAngle;
        targetAngle = startAngle + angle;  // 计算目标绝对角度
        
        // 规范化目标角度到±180度范围内
        while(targetAngle > 180.0f) {
            targetAngle -= 360.0f;
        }
        while(targetAngle < -180.0f) {
            targetAngle += 360.0f;
        }
        
        printf("开始旋转: 起始角度=%.2f, 目标角度=%.2f, 相对角度=%.2f\r\n", 
               startAngle, targetAngle, angle);
        
        lastError = 0;
        integral = 0;
        Steering_Stable_Count = 0;
        Steering_Completed = 0;
        isInitialized = 1;
        
        return 0;  // 刚初始化，返回未完成
    }
    
    // 计算当前误差
    error = targetAngle - currentAngle;
    
    // 处理误差过大的情况（过±180度），确保选择最短路径
    if(error > 180.0f) {
        error -= 360.0f;
    } else if(error < -180.0f) {
        error += 360.0f;
    }

    // 计算积分项
    integral += error;
    
    // 积分限幅
    if(integral > STEERING_I_LIMIT) {
        integral = STEERING_I_LIMIT;
    } else if(integral < -STEERING_I_LIMIT) {
        integral = -STEERING_I_LIMIT;
    }
    
    // 如果误差很小，逐渐减小积分项，防止过冲
    if(fabs(error) < (float)Steering_Error_Threshold/100.0f) {
        integral *= 0.9f;
    }
    
    // 计算微分项
    derivative = error - lastError;
    lastError = error;
    
    // 计算PID输出 - 使用放大100倍后的参数值，并转换回float
    output = ((float)Steering_Kp/100.0f) * error + ((float)Steering_Ki/100.0f) * integral + ((float)Steering_Kd/100.0f) * derivative;
    
    // 输出限幅
    if(output > STEERING_MAX_OUTPUT) {
        output = STEERING_MAX_OUTPUT;
    } else if(output < STEERING_MIN_OUTPUT) {
        output = STEERING_MIN_OUTPUT;
    }
    
    // 设置电机速度
    Motor_Left = -(PWM_Base + output);
    Motor_Right = PWM_Base + output;
    
    // 误差在阈值范围内且变化率小，计数稳定时间
    if(fabs(error) < (float)Steering_Error_Threshold/100.0f && fabs(derivative) < 0.5f) {
        Steering_Stable_Count++;
        printf("误差: %.2f, 当前角度: %.2f, 目标角度: %.2f\r\n", error, currentAngle, targetAngle);
        // 如果稳定计数达到设定时间，认为转向完成
        if(Steering_Stable_Count >= STEERING_STABLE_TIME) {
            // 重置状态，为下一次转向做准备
            isInitialized = 0;
            Set_Pwm(0, 0);  // 停止电机
            Steering_Completed = 1;
            printf("转向完成！最终角度: %.2f\r\n", currentAngle);
            return 1;  // 转向完成
        }
    } else {
        // 不稳定，重置稳定计数
        Steering_Stable_Count = 0;
    }
    
    // 执行电机控制
    if(Flag_Stop == 1) {
        Set_Pwm(0, 0);  // 停止标志为1时停止电机
    } else {
        Set_Pwm(Motor_Left, Motor_Right);  // 设置电机PWM
    }
    
    return 0;  // 转向未完成
}

/**
 * @brief This function is used to control the vehicle to move in a straight line
 * @note This function adjusts the vehicle's speed and direction to maintain a straight path
 * @param speed 目标速度
 * @return u8 1:完成直线移动(即有传感器检测到了黑线) 0:小车正在直行中(没有传感器检测黑线)
 */ 
int moveForward_Handler(void)
{
	Encoder_Left = Read_Encoder(3);							// 读取左轮编码器的值，前进为正，后退为负
	Encoder_Right = Read_Encoder(5);						// 修改为TIM5，前进为正，后退为负
															// 左轮A相接TIM2_CH1,右轮A相接TIM4_CH2,故这里两个编码器的极性相同
	Get_Velocity_Form_Encoder(Encoder_Left, Encoder_Right); // 编码器读数转速度（mm/s）                                          				
	int actual_velocity = Velocity(Encoder_Left, Encoder_Right); // 获取速度控制的PWM,速度控制
	Velocity(Encoder_Left, Encoder_Right);

	grey_sensor_Read(); // 读取灰度传感器数据 
	Turn_Pwm = Calculate_Turn_Pwm(); // 此处并非计算转向PWM值,而是查看当前小车是否有传感器检测到黑线,如果有传感器检测到黑线,则返回1,否则返回0
	if (Turn_Pwm == INT16_MIN)
	{
		Set_Pwm(0, 0); // 如果没有传感器检测到黑线，停止电机
		return 1; // 返回1，表示完成直线移动
	}
	// 使用计算出的实际速度，不修改Target_Velocity
	Motor_Left = actual_velocity;
	Motor_Right = actual_velocity;
	if (Flag_Stop == 1) 
		Set_Pwm(0, 0);
	else
		Set_Pwm(Motor_Left, Motor_Right); // 赋值给PWM寄存器
	return 0; // 返回0，表示没有完成走直线,正在走直线中
}


/**
 * @brief 转向测试函数 - 单方向循环旋转测试
 * @note 控制小车逆时针转60度，等待3秒，再次逆时针转60度，等待3秒，如此循环
 * @return None
 */
void SteeringTest_CyclicRotation(void)
{
    static uint8_t test_state = 0;  // 测试状态：0-初始化 1-逆时针旋转 2-等待3秒
    static uint32_t wait_time = 0;  // 等待时间计数器
    
    // 根据当前状态执行不同操作
    switch(test_state)
    {
        case 0:  // 初始化状态
            printf("开始转向测试：逆时针60度 -> 等待3秒 -> 循环\r\n");
            test_state = 1;  // 进入逆时针旋转状态
            led1_on();       // 点亮LED1作为逆时针旋转指示
            led2_off();
            led3_off();
            break;
            
        case 1:  // 逆时针旋转60度
            if(localSteeringControl_Handler(60.0f))  // 如果转向完成
            {
                printf("逆时针旋转60度完成，等待3秒...\r\n");
                test_state = 2;   // 进入等待状态
                wait_time = 0;    // 清零等待时间计数器
                led1_off();       // 关闭LED1
                led3_on();        // 点亮LED3作为等待指示
            }
            break;
            
        case 2:  // 等待3秒
            wait_time++;
            if(wait_time >= 600)  // 5ms中断，600次大约3秒
            {
                test_state = 1;   // 返回逆时针旋转状态，形成循环
                printf("等待结束，再次开始逆时针旋转60度...\r\n");
                led3_off();       // 关闭LED3
                led1_on();        // 点亮LED1作为逆时针旋转指示
            }
            break;
            
        default:
            test_state = 0;  // 异常情况，重置状态
            break;
    }
}


void Test_Handler(void)
{
	SteeringTest_CyclicRotation(); // 调用转向测试函数
}

/**
 * @brief 设置转向控制比例系数
 * @param kp 新的比例系数(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Kp(u16 kp)
{
    Steering_Kp = kp;
    printf("设置转向控制比例系数: Kp = %.3f\r\n", (float)kp/100.0f);
}

/**
 * @brief 设置转向控制积分系数
 * @param ki 新的积分系数(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Ki(u16 ki)
{
    Steering_Ki = ki;
    printf("设置转向控制积分系数: Ki = %.3f\r\n", (float)ki/100.0f);
}

/**
 * @brief 设置转向控制微分系数
 * @param kd 新的微分系数(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Kd(u16 kd)
{
    Steering_Kd = kd;
    printf("设置转向控制微分系数: Kd = %.3f\r\n", (float)kd/100.0f);
}

/**
 * @brief 设置转向控制误差阈值
 * @param threshold 新的误差阈值(度)(放大100倍的整数)
 * @return 无
 */
void Set_Steering_Error_Threshold(u16 threshold)
{
    Steering_Error_Threshold = threshold;
    printf("设置转向控制误差阈值: %.3f度\r\n", (float)threshold/100.0f);
}

/**
 * @brief 一次性设置所有转向控制参数
 * @param kp 比例系数(放大100倍的整数)
 * @param ki 积分系数(放大100倍的整数)
 * @param kd 微分系数(放大100倍的整数)
 * @param threshold 误差阈值(度)(放大100倍的整数)
 * @return 无
 */
void Set_All_Steering_Params(u16 kp, u16 ki, u16 kd, u16 threshold)
{
    Steering_Kp = kp;
    Steering_Ki = ki;
    Steering_Kd = kd;
    Steering_Error_Threshold = threshold;
    printf("设置所有转向控制参数:\r\n");
    printf("Kp = %.3f, Ki = %.3f, Kd = %.3f\r\n", (float)kp/100.0f, (float)ki/100.0f, (float)kd/100.0f);
    printf("误差阈值 = %.3f度\r\n", (float)threshold/100.0f);
}

/**
 * @brief 获取转向控制比例系数
 * @return 当前比例系数
 */
float Get_Steering_Kp(void)
{
    float kp = (float)Steering_Kp/100.0f;
    printf("当前转向控制比例系数: Kp = %.3f\r\n", kp);
    return kp;
}

/**
 * @brief 获取转向控制积分系数
 * @return 当前积分系数
 */
float Get_Steering_Ki(void)
{
    float ki = (float)Steering_Ki/100.0f;
    printf("当前转向控制积分系数: Ki = %.3f\r\n", ki);
    return ki;
}

/**
 * @brief 获取转向控制微分系数
 * @return 当前微分系数
 */
float Get_Steering_Kd(void)
{
    float kd = (float)Steering_Kd/100.0f;
    printf("当前转向控制微分系数: Kd = %.3f\r\n", kd);
    return kd;
}

/**
 * @brief 获取转向控制误差阈值
 * @return 当前误差阈值(度)
 */
float Get_Steering_Error_Threshold(void)
{
    float threshold = (float)Steering_Error_Threshold/100.0f;
    printf("当前转向控制误差阈值: %.3f度\r\n", threshold);
    return threshold;
}
