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
#include "Test.h"
#include "Task.h"
#include "setParma.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// 在文件开头添加全局变量定义
float Velocity_Left, Velocity_Right; // 左右轮速度，全局变量
volatile int Encoder_Left, Encoder_Right; // 左右编码器的脉冲计数
volatile int Balance_Pwm, Velocity_Pwm, Turn_Pwm=0;

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
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6)
	{
		#if TEST_MODE==1 // 单项测试模式
			Test_Handler();
        #elif Normal_Mode==1 // 正常模式
            normal_Handler(); // 正常模式
        #elif TASK1==1 // 测试模式1
            Task1_Handler(); // 测试模式1
        #elif TASK2==1 // 测试模式2
            Task2_Handler(); // 测试模式2
        #elif TASK3==1 // 测试模式3
            Task3_Handler(); // 测试模式3
        #elif TASK4==1 // 测试模式4
            Task4_Handler(); // 测试模式4
		#endif
	}
    if (htim->Instance == TIM7)
    {
        JY901_Handler(); // 处理JY901数据 
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
    static u8 init=0; // 初始化标志
    static u32 startTime = 0; // 开始时间
    static u16 prev_turn_pwm = 0; // 上一次的转向PWM值
    if(init==0) // 如果没有初始化
    {
        init=1; // 设置初始化标志
        startTime = HAL_GetTick(); // 记录开始时间
        return 0; // 返回0，表示没有完成循迹,正在循迹中
    }
	Encoder_Left = Read_Encoder(3);							// 读取左轮编码器的值，前进为正，后退为负
	Encoder_Right = Read_Encoder(5);						// 修改为TIM5，前进为正，后退为负
															// 左轮A相接TIM2_CH1,右轮A相接TIM4_CH2,故这里两个编码器的脉冲极性相同
	Get_Velocity_Form_Encoder(Encoder_Left, Encoder_Right); // 编码器读数转速度（mm/s）                                          				
	int actual_velocity = Velocity(Encoder_Left, Encoder_Right); // 获取速度控制的PWM,速度控制

	grey_sensor_Read(); // 读取灰度传感器数据
	#if NOLINEDETECT==1 // 调试模式，不使用灰度传感器
	Turn_Pwm = 0; // 调试模式，不使用灰度传感器
	#else
	Turn_Pwm = Calculate_Turn_Pwm(); // 计算转向PWM值,如果所有传感器都返回白色，返回1

	if (Turn_Pwm==INT16_MIN)
	{
        if(HAL_GetTick() - startTime <= LINE_TRACKING_DEAD_TIME) 
        {
            Motor_Left = actual_velocity - prev_turn_pwm;
            Motor_Right = actual_velocity + prev_turn_pwm;
            if (Flag_Stop == 1) 
                Set_Pwm(0, 0);
            else
                Set_Pwm(Motor_Left, Motor_Right); // 赋值给PWM寄存器
            return 0; // 返回0，表示没有完成循迹,正在循迹中
        }
        else
        {
            Set_Pwm(0, 0); // 如果没有传感器检测到黑线，停止电机
            init = 0; // 重置初始化标志
            startTime = 0; // 重置开始时间
            return 1; // 返回1，表示完成循迹
        }
	}
	#endif

	// 使用计算出的实际速度，不修改Target_Velocity
	Motor_Left = actual_velocity - Turn_Pwm;
	Motor_Right = actual_velocity + Turn_Pwm;
	if (Flag_Stop == 1) 
		Set_Pwm(0, 0);
	else
		Set_Pwm(Motor_Left, Motor_Right); // 赋值给PWM寄存器
        prev_turn_pwm = Turn_Pwm; // 更新上一次的转向PWM值
        
	return 0; // 返回0，表示没有完成循迹,正在循迹中
}


/**
 * @brief Open loop steering control function
 * 
 * @param angle 小车旋转的时间,逆时针旋转为正,顺时针旋转为负
 * @return u8 1:完成 0:小车正在转向中
 */
u8 openLoopSteering_Handler(int SteerTime,int PWM_Value)
{
    static u8 isInitialized = 0; // 初始化标志
    static u32 startTime = 0; // 开始时间
    
    if(isInitialized == 0) {
        startTime = HAL_GetTick(); // 记录开始时间
        isInitialized = 1; // 设置初始化标志
        return 0; // 返回未完成
    }
    
    if(HAL_GetTick() - startTime >= abs(SteerTime)) {
        Set_Pwm(0, 0); // 停止电机
        isInitialized = 0; // 重置初始化标志
        return 1; // 返回完成
    } else {
        if(SteerTime > 0) {
            // 逆时针转向
            Motor_Left = openLoopSteeringBase_PWM - PWM_Value; // 左轮反转
            Motor_Right = openLoopSteeringBase_PWM + PWM_Value; // 右轮正转
        } else {
            // 顺时针转向
            Motor_Left = openLoopSteeringBase_PWM + PWM_Value; // 左轮正转
            Motor_Right = openLoopSteeringBase_PWM - PWM_Value; // 右轮反转
        }
        if(Flag_Stop==1) // 如果停止标志为1，停止电机
            Set_Pwm(0, 0); // 停止电机
        else
            Set_Pwm(Motor_Left, Motor_Right); // 赋值给PWM寄存器
        return 0; // 返回未完成
    }
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
    output = ((float)Steering_Kp/100.0f) * error + ((float)Steering_Ki/1000.0f) * integral + ((float)Steering_Kd/100.0f) * derivative;
    
    // 输出限幅
    if(output > STEERING_MAX_OUTPUT) {
        output = STEERING_MAX_OUTPUT;
    } else if(output < STEERING_MIN_OUTPUT) {
        output = STEERING_MIN_OUTPUT;
    }
    
    if (output >= 0) {
        // 逆时针转向 (output 为正或零)
        // 左轮反转，右轮正转
        Motor_Left = -(PWM_Base + output);
        Motor_Right = PWM_Base + output;
    } else {
        // 顺时针转向 (output 为负)
        // 左轮正转，右轮反转，注意 output 本身为负数
        Motor_Left = PWM_Base - output;  // -output 为正值，增加左轮正转速度
        Motor_Right = -(PWM_Base - output); // -output 为正值，增加右轮反转速度的绝对值
    }
    
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
 * using gyroscope angle feedback for direction correction
 * @param void
 * @return u8 1:完成直线移动(即有传感器检测到了黑线) 0:小车正在直行中(没有传感器检测黑线)
 */ 
int moveForward_Handler(void)
{
    // 状态机状态枚举
    typedef enum {
        INIT,              // 初始化状态
        FORWARD_MOVING     // 正常直线行驶状态
    } MoveForwardState;
    
    // 静态变量
    static MoveForwardState state = INIT;
    static float integral = 0.0f;          // 积分项
    static float lastError = 0.0f;         // 上一次误差
    static uint8_t debug_counter = 0;      // 调试计数器
    static float referenceAngle=0;
    float currentAngle, error, derivative, angleCorrection;
    bool can_print_debug = false;
    if (++debug_counter >= DEBUG_PRINT_COUNT) {
        debug_counter = 0;
        can_print_debug = true;
    }
    // 读取灰度传感器数据
    grey_sensor_Read();
    // 检查是否有传感器检测到黑线
    Turn_Pwm = Calculate_Turn_Pwm();
    if (Turn_Pwm != INT16_MIN)
    {
        Set_Pwm(0, 0); // 如果有传感器检测到黑线，停止电机
        // 重置所有状态变量，为下次调用准备
        state = INIT;
        integral = 0.0f;
        lastError = 0.0f;
        if(can_print_debug==true) {
            printf("传感器检测到黑线，停止电机\r\n");
        }
        return 1; // 返回1，表示完成直线移动
    }
    
    // 状态机处理
    switch (state) {
        case INIT:
            // 初始化状态 - 重置积分和误差
            referenceAngle=getHeadingAngle();
            // 边界条件处理 - 确保参考角度在±180度范围内
            while(referenceAngle > 180.0f) {
                referenceAngle -= 360.0f;
            }
            while(referenceAngle < -180.0f) {
                referenceAngle += 360.0f;
            }
            printf("开始按指定角度 %.2f 度直线行驶\r\n", referenceAngle);
            integral = 0.0f;
            lastError = 0.0f;
            state = FORWARD_MOVING;
            Set_Pwm(FORWARDBASE_PWM, FORWARDBASE_PWM);
            return 0;
            
        case FORWARD_MOVING:
            // 正常的直线行走阶段
            // 获取当前角度并计算误差
            currentAngle = getHeadingAngle();
            error = currentAngle - referenceAngle;
            
            // 处理误差过大的情况（过±180度），确保选择最短路径
            if (error > 180.0f) {
                error -= 360.0f;
            } else if (error < -180.0f) {
                error += 360.0f;
            }
            
            // 计算积分项
            integral += error;
            
            // 积分限幅
            if (integral > FORWARD_I_LIMIT) {
                integral = FORWARD_I_LIMIT;
            } else if (integral < -FORWARD_I_LIMIT) {
                integral = -FORWARD_I_LIMIT;
            }
            
            // 如果误差很小，逐渐减小积分项，防止过冲
            if (fabs(error) < (float)Forward_Error_Threshold/100.0f) {
                integral *= 0.95f;
            }
            
            // 计算微分项
            derivative = error - lastError;
            lastError = error;
            
            // 计算PID输出 - 使用放大100倍后的参数值，并转换回float
            angleCorrection = ((float)Forward_Kp/100.0f) * error + 
							  ((float)Forward_Ki/100.0f) * integral + 
							  ((float)Forward_Kd/100.0f) * derivative;
            
            // 输出限幅
            if (angleCorrection > STEERING_MAX_OUTPUT) {
                angleCorrection = STEERING_MAX_OUTPUT;
            } else if (angleCorrection < STEERING_MIN_OUTPUT) {
                angleCorrection = STEERING_MIN_OUTPUT;
            }
            
            // 基于角度纠正计算左右轮差速
            Motor_Left = forwardBase_PWM + angleCorrection;
            Motor_Right = forwardBase_PWM - angleCorrection;
            
            // 打印调试信息（降低频率，避免刷屏）
            if (can_print_debug==true) { // 每隔一定次数打印一次
                printf("直线修正: 当前角度=%.2f, 参考角度=%.2f, 误差=%.2f, 修正值=%.2f\r\n", 
					   currentAngle, referenceAngle, error, angleCorrection);
            }
            break;
    }


    
    // 执行电机控制
    if (Flag_Stop == 1) 
        Set_Pwm(0, 0); // 停止标志为1时停止电机
    else
        Set_Pwm(Motor_Left, Motor_Right); // 赋值给PWM寄存器
    
    return 0; // 返回0，表示没有完成直线移动，正在直行中
}



/**
 * @brief 指定角度直线行驶函数
 * @note 此函数可以接受一个参考角度作为参数，使小车按照指定的角度直线行驶
 * @param referenceAngle 小车行驶的参考角度(度)
 * @return int 1:完成直线行驶(检测到黑线) 0:小车正在直行中
 */
int moveForwardWithAngle_Handler(float referenceAngle)
{
    // 状态机状态枚举
    typedef enum {
        INIT,              // 初始化状态
        FORWARD_MOVING     // 正常直线行驶状态
    } MoveForwardState;
    
    // 静态变量
    static MoveForwardState state = INIT;
    static float integral = 0.0f;          // 积分项
    static float lastError = 0.0f;         // 上一次误差
    static uint8_t debug_counter = 0;      // 调试计数器
    float currentAngle, error, derivative, angleCorrection;

    bool can_print_debug = false;
    if (++debug_counter >= DEBUG_PRINT_COUNT) {
        debug_counter = 0;
        can_print_debug = true;
    }

    // 边界条件处理 - 确保参考角度在±180度范围内
    while(referenceAngle > 180.0f) {
        referenceAngle -= 360.0f;
    }
    while(referenceAngle < -180.0f) {
        referenceAngle += 360.0f;
    }

    // 读取灰度传感器数据
    grey_sensor_Read();
    // 检查是否有传感器检测到黑线
    Turn_Pwm = Calculate_Turn_Pwm();
    if (Turn_Pwm != INT16_MIN)
    {
        Set_Pwm(0, 0); // 如果有传感器检测到黑线，停止电机
        // 重置所有状态变量，为下次调用准备
        state = INIT;
        integral = 0.0f;
        lastError = 0.0f;
        if(can_print_debug==true) {
            printf("传感器检测到黑线，停止电机\r\n");
        }
        return 1; // 返回1，表示完成直线移动
    }
    
    // 状态机处理
    switch (state) {
        case INIT:
            // 初始化状态 - 重置积分和误差
            printf("开始按指定角度 %.2f 度直线行驶\r\n", referenceAngle);
            integral = 0.0f;
            lastError = 0.0f;
            state = FORWARD_MOVING;
            Set_Pwm(FORWARDBASE_PWM, FORWARDBASE_PWM);
            return 0;
            
        case FORWARD_MOVING:
            // 正常的直线行走阶段
            // 获取当前角度并计算误差
            currentAngle = getHeadingAngle();
            error = currentAngle - referenceAngle;
            
            // 处理误差过大的情况（过±180度），确保选择最短路径
            if (error > 180.0f) {
                error -= 360.0f;
            } else if (error < -180.0f) {
                error += 360.0f;
            }
            
            // 计算积分项
            integral += error;
            
            // 积分限幅
            if (integral > FORWARD_I_LIMIT) {
                integral = FORWARD_I_LIMIT;
            } else if (integral < -FORWARD_I_LIMIT) {
                integral = -FORWARD_I_LIMIT;
            }
            
            // 如果误差很小，逐渐减小积分项，防止过冲
            if (fabs(error) < (float)Forward_Error_Threshold/100.0f) {
                integral *= 0.95f;
            }
            
            // 计算微分项
            derivative = error - lastError;
            lastError = error;
            
            // 计算PID输出 - 使用放大100倍后的参数值，并转换回float
            angleCorrection = ((float)Forward_Kp/100.0f) * error + 
							  ((float)Forward_Ki/100.0f) * integral + 
							  ((float)Forward_Kd/100.0f) * derivative;
            
            // 输出限幅
            if (angleCorrection > STEERING_MAX_OUTPUT) {
                angleCorrection = STEERING_MAX_OUTPUT;
            } else if (angleCorrection < STEERING_MIN_OUTPUT) {
                angleCorrection = STEERING_MIN_OUTPUT;
            }
            
            // 基于角度纠正计算左右轮差速
            Motor_Left = forwardBase_PWM + angleCorrection;
            Motor_Right = forwardBase_PWM - angleCorrection;
            
            // 打印调试信息（降低频率，避免刷屏）
            if ( can_print_debug==true ) { // 每隔一定次数打印一次
                printf("直线修正: 当前角度=%.2f, 参考角度=%.2f, 误差=%.2f, 修正值=%.2f\r\n", 
					   currentAngle, referenceAngle, error, angleCorrection);
                }
            break;
    }


    
    // 执行电机控制
    if (Flag_Stop == 1) 
        Set_Pwm(0, 0); // 停止标志为1时停止电机
    else
        Set_Pwm(Motor_Left, Motor_Right); // 赋值给PWM寄存器
    
    return 0; // 返回0，表示没有完成直线移动，正在直行中
}



/**
 * @brief 控制小车转向到指定的绝对角度
 * @note 这个函数控制小车转向到一个指定的绝对航向角度，不是相对转动角度
 * 当车头指向正西方向时候返回值为0度,正北方向为-90度,正南方向为90度
 * @param targetAbsoluteAngle 目标绝对角度，范围[-180, 180]度
 * @return u8 1:转向完成 0:小车正在转向中
 */
u8 turnToAbsoluteAngle(float targetAbsoluteAngle)
{
    static float lastError = 0;        // 上一次误差
    static float integral = 0;         // 积分项
    static u8 isInitialized = 0;       // 初始化标志
    static uint8_t debug_counter = 0;      // 调试计数器
    static float angleDifference = 0;     // 目标角度与起始角度的差值
    float currentAngle = getHeadingAngle();   // 获取当前角度
    float error, derivative, output;
    bool can_print_debug = false;
    if (++debug_counter >= DEBUG_PRINT_COUNT) {
        debug_counter = 0;
        can_print_debug = true;
    }
    
    // 确保目标角度在[-180, 180]范围内
    while(targetAbsoluteAngle > 180.0f) {
        targetAbsoluteAngle -= 360.0f;
    }
    while(targetAbsoluteAngle < -180.0f) {
        targetAbsoluteAngle += 360.0f;
    }
    
    // 初始化，重置控制变量
    if(isInitialized == 0) {
        printf("开始旋转到绝对角度: 当前角度=%.2f, 目标角度=%.2f\r\n", 
               currentAngle, targetAbsoluteAngle);
        
        angleDifference = targetAbsoluteAngle - currentAngle;
        // 处理角度差值，确保在[-180, 180]范围内
        if(angleDifference > 180.0f) {
            angleDifference -= 360.0f;
        } else if(angleDifference < -180.0f) {
            angleDifference += 360.0f;
        }
        lastError = 0;
        integral = 0;
        Steering_Stable_Count = 0;
        Steering_Completed = 0;
        isInitialized = 1;
        
        return 0;  // 刚初始化，返回未完成
    }
    
    // 计算当前误差
    error = targetAbsoluteAngle - currentAngle;
    
    // 关键处理: 处理误差过大的情况（过±180度），确保选择最短路径
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
    if(fabs(error) < (float)Steering_Error_Threshold/100.0f * 0.85f) {
        integral *= 0.9f;
    }
    
    // 计算微分项
    derivative = error - lastError;
    lastError = error;
    
    // 计算PID输出 - 使用放大100倍后的参数值，并转换回float
    output = ((float)Steering_Kp/100.0f) * error + 
             ((float)Steering_Ki/100.0f) * integral + 
             ((float)Steering_Kd/100.0f) * derivative;
    
    // 输出限幅
    if(output > STEERING_MAX_OUTPUT) {
        output = STEERING_MAX_OUTPUT;
    } else if(output < STEERING_MIN_OUTPUT) {
        output = STEERING_MIN_OUTPUT;
    }
    
    if (output >= 0) {
        // 逆时针转向 (output 为正或零)
        // 左轮反转，右轮正转
        Motor_Left = -(PWM_Base + output);
        Motor_Right = PWM_Base + output;
    } else {
        // 顺时针转向 (output 为负)
        // 左轮正转，右轮反转，注意 output 本身为负数
        Motor_Left = PWM_Base - output;  // -output 为正值，增加左轮正转速度
        Motor_Right = -(PWM_Base - output); // -output 为正值，增加右轮反转速度的绝对值
    }
    
    // 误差在阈值范围内且变化率小，计数稳定时间
    if(fabs(error) < (float)Steering_Error_Threshold/100.0f && fabs(derivative) < 1.2f ) {
        Steering_Stable_Count++;
        
        // 调试输出
        if(Steering_Stable_Count % 20 == 0) {  // 减少打印频率
            printf("误差: %.2f, 当前角度: %.2f, 目标角度: %.2f\r\n", 
                   error, currentAngle, targetAbsoluteAngle);
        }
        
        // 如果稳定计数达到设定时间，认为转向完成
        if(Steering_Stable_Count >= STEERING_STABLE_TIME) {
            // 重置状态，为下一次转向做准备
            isInitialized = 0;
            Set_Pwm(0, 0);  // 停止电机
            Steering_Completed = 1;
            printf("转向到绝对角度完成！最终角度: %.2f\r\n", currentAngle);
            return 1;  // 转向完成
        }
    } else {
        // 不稳定，重置稳定计数
        Steering_Stable_Count = 0;
    }

    if(can_print_debug==true) { 
        // 打印调试信息
        printf("PID调试: 当前角度=%.2f, 目标角度=%.2f, 误差=%.2f, 修正值=%.2f\r\n", 
               currentAngle, targetAbsoluteAngle, error, output);
    }
    if(angleDifference > 0) {
        // 逆时针转向
        Motor_Left = 0; // 左轮停止
    } else {
        // 顺时针转向
        Motor_Right = 0; // 右轮停止
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
 * @brief normal_Handler函数
 * @note 普通模式下的处理函数
 * @return None
 */
void normal_Handler(void)
{
    // 普通模式的处理代码
}


/**
 * @brief angleSetWithKey_Handler函数
 * @note 通过按键设置角度的处理函数
 * @return None
 */
void angleSetWithKey_Handler(void)
{
    typedef enum
    {
        INITIAL,              // 初始状态
        SET_INITIAL_ANGLE,    // 设置初始角度
        SET_TASK_ROTATION_ANGLE_1, // 设置任务旋转角度1
        SET_TASK_ROTATION_ANGLE_3, // 设置任务旋转角度3
        SET_TASK_ROTATION_ANGLE_4  // 设置任务旋转角度4
    } AngleSetState;
    
    static AngleSetState state = INITIAL; // 初始化状态机
    static uint32_t key_press_start_time = 0; // 按键按下开始时间
    static uint8_t is_key_pressed = 0; // 按键是否被按下
    static uint8_t long_press_handled = 0; // 长按是否已处理标志
    
    // 获取按键状态
    key_state = Get_Key_Value(); 
    
    // 按键状态处理
    if (key_state == KEY_ENTER) // 检测到按下ENTER键
    {
        if (is_key_pressed == 0) // 第一次检测到按下
        {
            key_press_start_time = HAL_GetTick(); // 记录按下开始时间
            is_key_pressed = 1; // 标记按键已按下
            long_press_handled = 0; // 重置长按处理标志
            printf("按键按下，开始计时\r\n"); // 调试信息
        }
        else // 按键持续按下
        {
            uint32_t press_duration = HAL_GetTick() - key_press_start_time;
            
            // 调试输出：每500ms打印一次按键持续时间
            if (press_duration % 500 == 0) {
                printf("按键持续按下时间: %lu ms\r\n", press_duration);
            }
            
            if (press_duration >= LONG_PRESS_THRESHOLD && long_press_handled == 0)
            {
                // 满足长按条件，直接启动小车
                printf("检测到长按（%lums），启动小车！\r\n", press_duration);
                state = INITIAL; // 重置状态机
                // 启动小车
                if (Flag_Stop == 1) { // 只有在停止状态下才启动
                    HAL_TIM6_toggle_IT(); // 切换定时器6中断
                    resetTask(); // 重置任务
                    state = INITIAL; // 重置状态机
                    toggle_Flag_Stop(); // 切换停止标志
                    all_leds_off();
                    manual_mode = 1; // 手动设置了角度
                }
                // 标记长按已处理
                long_press_handled = 1;
            }
        }
    }
    else // 按键松开
    {
        if (is_key_pressed) // 之前按键是按下的，现在松开了
        {
            uint32_t press_duration = HAL_GetTick() - key_press_start_time;
            printf("按键释放，按下持续时间: %lu ms\r\n", press_duration);
            
            is_key_pressed = 0; // 重置按键按下标记
            
            // 只有短按才处理状态机逻辑
            if (press_duration < LONG_PRESS_THRESHOLD)
            {
                float reverseInitialAngle = initialAngle + 180; // 计算反向初始角度
                // 规范化反向初始角度到[-180, 180]范围内
                while (reverseInitialAngle > 180.0f) {
                    reverseInitialAngle -= 360.0f;
                }
                while (reverseInitialAngle < -180.0f) {
                    reverseInitialAngle += 360.0f;
                }
                
                // 状态机处理逻辑
                switch (state)
                {
                    case INITIAL:
                        state = SET_INITIAL_ANGLE; // 设置初始角度
                        Flag_Stop = 1; // 设置停止标志
                        HAL_TIM6_toggle_IT(); // 切换定时器6中断
                        Set_Pwm(0, 0); // 停止电机
                        led1_on(); // 打开LED1
                        break;
                        
                    case SET_INITIAL_ANGLE:
                        state = SET_TASK_ROTATION_ANGLE_1; // 设置任务旋转角度1
                        initialAngle = getHeadingAngle(); // 获取当前航向角度
                        printf("设置初始角度: %.2f\r\n", initialAngle); // 打印初始角度
                        led2_on(); // 打开LED2
                        break;
                        
                    case SET_TASK_ROTATION_ANGLE_1:
                        Task4_Rotation_Angle_1 = Task3_Rotation_Angle_1 = initialAngle - getHeadingAngle(); // 计算任务旋转角度1
                        // 规范化任务旋转角度1到[-180, 180]范围内
                        while (Task3_Rotation_Angle_1 > 180.0f) {
                            Task3_Rotation_Angle_1 -= 360.0f;
                        }
                        while (Task3_Rotation_Angle_1 < -180.0f) {
                            Task3_Rotation_Angle_1 += 360.0f;
                        }
                        state = SET_TASK_ROTATION_ANGLE_4; // 设置任务旋转角度4
                        printf("设置任务旋转角度1: %.2f\r\n", Task3_Rotation_Angle_1); // 打印任务旋转角度1
                        led3_on(); // 打开LED3
                        break;
                        
                    case SET_TASK_ROTATION_ANGLE_4:
                        state = SET_TASK_ROTATION_ANGLE_3; // 设置任务旋转角度4
                        Task4_Rotation_Angle_3 = initialAngle - getHeadingAngle(); // 计算任务旋转角度4
                        while (Task4_Rotation_Angle_3 > 180.0f) {
                            Task4_Rotation_Angle_3 -= 360.0f;
                        }
                        while (Task4_Rotation_Angle_3 < -180.0f) {
                            Task4_Rotation_Angle_3 += 360.0f;
                        }


                        led1_off(); // 关闭LED1
                        printf("设置任务旋转角度2: %.2f\r\n", Task3_Rotation_Angle_2); // 打印任务旋转角度3
                        break;
                        
                    case SET_TASK_ROTATION_ANGLE_3:
                        state = INITIAL; // 返回初始状态
                        Task4_Rotation_Angle_2 = Task3_Rotation_Angle_2 = getHeadingAngle() - reverseInitialAngle; // 计算任务旋转角度3
                        // 规范化任务旋转角度3到[-180, 180]范围内
                        while (Task3_Rotation_Angle_2 > 180.0f) {
                            Task3_Rotation_Angle_2 -= 360.0f;
                        }
                        while (Task3_Rotation_Angle_2 < -180.0f) {
                            Task3_Rotation_Angle_2 += 360.0f;
                        }

                        led1_off(); // 关闭LED1
                        led2_off(); // 关闭LED2
                        led3_off(); // 关闭LED3
                        manual_mode = 1; // 手动设置了角度
                        printf("设置任务旋转角度3: %.2f\r\n", Task4_Rotation_Angle_3); // 打印任务旋转角度3
                        printf("初始角度: %.2f , 任务旋转角度1: %.2f, 任务旋转角度2: %.2f, 任务旋转角度3: %.2f\r\n", 
                               initialAngle, Task3_Rotation_Angle_1, Task3_Rotation_Angle_2, Task4_Rotation_Angle_3);
                        HAL_TIM6_toggle_IT(); // 切换定时器6中断
                        resetTask(); // 重置任务
                        toggle_Flag_Stop(); // 切换停止标志
                        break;
                }
            }
        }
    }
}

/**
 * @brief 指定角度直线行驶函数,直线行驶直到灰度传感器的某半部分检测到黑线
 * @note 此函数可以接受一个参考角度作为参数，使小车按照指定的角度直线行驶
 * @param referenceAngle 小车行驶的参考角度(度)
 * @param start_graySensor 起始灰度传感器编号(从1开始)
 * @param end_graySensor 结束灰度传感器编号
 * @return int 1:完成直线行驶(检测到黑线) 0:小车正在直行中
 */
int moveForwardWithAngle_UntileSomeGraySencorActived_Handler(float referenceAngle,u8 start_graySensor,u8 end_graySensor)
{
    // 状态机状态枚举
    typedef enum {
        INIT,              // 初始化状态
        FORWARD_MOVING     // 正常直线行驶状态
    } MoveForwardState;
    
    // 静态变量
    static MoveForwardState state = INIT;
    static float integral = 0.0f;          // 积分项
    static float lastError = 0.0f;         // 上一次误差
    static uint8_t debug_counter = 0;      // 调试计数器
    float currentAngle, error, derivative, angleCorrection;

    bool can_print_debug = false;
    if (++debug_counter >= DEBUG_PRINT_COUNT) {
        debug_counter = 0;
        can_print_debug = true;
    }

    // 边界条件处理 - 确保参考角度在±180度范围内
    while(referenceAngle > 180.0f) {
        referenceAngle -= 360.0f;
    }
    while(referenceAngle < -180.0f) {
        referenceAngle += 360.0f;
    }

    // 读取灰度传感器数据
    grey_sensor_Read();
    // 检查是否有传感器检测到黑线
    Turn_Pwm = IsCertainGraySenorsAcrived(start_graySensor-1,end_graySensor-1);
    if (Turn_Pwm != INT16_MIN)
    {
        Set_Pwm(0, 0); // 如果有传感器检测到黑线，停止电机
        // 重置所有状态变量，为下次调用准备
        state = INIT;
        integral = 0.0f;
        lastError = 0.0f;
        if(can_print_debug==true) {
            printf("传感器检测到黑线，停止电机\r\n");
        }
        return 1; // 返回1，表示完成直线移动
    }
    
    // 状态机处理
    switch (state) {
        case INIT:
            // 初始化状态 - 重置积分和误差
            printf("开始按指定角度 %.2f 度直线行驶\r\n", referenceAngle);
            integral = 0.0f;
            lastError = 0.0f;
            state = FORWARD_MOVING;
            Set_Pwm(FORWARDBASE_PWM, FORWARDBASE_PWM);
            return 0;
            
        case FORWARD_MOVING:
            // 正常的直线行走阶段
            // 获取当前角度并计算误差
            currentAngle = getHeadingAngle();
            error = currentAngle - referenceAngle;
            
            // 处理误差过大的情况（过±180度），确保选择最短路径
            if (error > 180.0f) {
                error -= 360.0f;
            } else if (error < -180.0f) {
                error += 360.0f;
            }
            
            // 计算积分项
            integral += error;
            
            // 积分限幅
            if (integral > FORWARD_I_LIMIT) {
                integral = FORWARD_I_LIMIT;
            } else if (integral < -FORWARD_I_LIMIT) {
                integral = -FORWARD_I_LIMIT;
            }
            
            // 如果误差很小，逐渐减小积分项，防止过冲
            if (fabs(error) < (float)Forward_Error_Threshold/100.0f) {
                integral *= 0.95f;
            }
            
            // 计算微分项
            derivative = error - lastError;
            lastError = error;
            
            // 计算PID输出 - 使用放大100倍后的参数值，并转换回float
            angleCorrection = ((float)Forward_Kp/100.0f) * error + 
							  ((float)Forward_Ki/100.0f) * integral + 
							  ((float)Forward_Kd/100.0f) * derivative;
            
            // 输出限幅
            if (angleCorrection > STEERING_MAX_OUTPUT) {
                angleCorrection = STEERING_MAX_OUTPUT;
            } else if (angleCorrection < STEERING_MIN_OUTPUT) {
                angleCorrection = STEERING_MIN_OUTPUT;
            }
            
            // 基于角度纠正计算左右轮差速
            Motor_Left = forwardBase_PWM + angleCorrection;
            Motor_Right = forwardBase_PWM - angleCorrection;
            
            // 打印调试信息（降低频率，避免刷屏）
            if ( can_print_debug==true ) { // 每隔一定次数打印一次
                printf("直线修正: 当前角度=%.2f, 参考角度=%.2f, 误差=%.2f, 修正值=%.2f\r\n", 
					   currentAngle, referenceAngle, error, angleCorrection);
                }
            break;
    }


    
    // 执行电机控制
    if (Flag_Stop == 1) 
        Set_Pwm(0, 0); // 停止标志为1时停止电机
    else
        Set_Pwm(Motor_Left, Motor_Right); // 赋值给PWM寄存器
    
    return 0; // 返回0，表示没有完成直线移动，正在直行中
}
