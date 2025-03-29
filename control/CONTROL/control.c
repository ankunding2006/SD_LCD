/***********************************************
��ԭ����һ��ƽ�⳵�Ŀ��ƴ��룬���ڸĳ���һ��С���Ŀ��ƴ���
����֮ǰ�����������stm32f103ϵ�еĿ��������еģ����ڸĳ�
��stm32f407ϵ�еĿ���������

������ʱû��ʹ��mpu6050������Ҳû��ʹ������ģ�飬����������
ģ��Ĵ��붼û���õ�,����������ģ��Ĵ��뻹�Ǳ���������ļ�
�У��Ժ���ܻ��õ�.

���ֽ׶���ֻ����ɻ�����С�����ƣ�������ֻ��Ҫʹ�õ���������
�������ģ�飬������ֻ��Ҫʹ�õ�������ģ��Ĵ��룬������ģ��
�Ĵ��붼����Ҫʹ�ã������һ����Щģ��Ĵ���ɾ������ֻ��������
���͵������ģ��Ĵ���
***********************************************/
#include "control.h"

// ���ļ���ͷ���ȫ�ֱ�������
float Velocity_Left, Velocity_Right; // �������ٶȣ�ȫ�ֱ���

/**************************************************************************
Function: Control function
Input   : none
Output  : none
�������ܣ����еĿ��ƴ��붼��������
		 5ms�ⲿ�ж��ɶ�ʱ��TIM6�������ϸ�֤���������ݴ����ʱ��ͬ��
��ڲ�������
		 �ϸ�֤���������ݴ����ʱ��ͬ��
��ڲ�������
����  ֵ����
**************************************************************************/

volatile int Encoder_Left, Encoder_Right; // ���ұ��������������
volatile int Balance_Pwm, Velocity_Pwm, Turn_Pwm;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6)
	{
		Encoder_Left = Read_Encoder(3);							// ��ȡ���ֱ�������ֵ��ǰ��Ϊ��������Ϊ��
		Encoder_Right = -Read_Encoder(5);						// �޸�ΪTIM5��ǰ��Ϊ��������Ϊ��
																// ����A���TIM2_CH1,����A���TIM4_CH2,�����������������ļ�����ͬ
		Get_Velocity_Form_Encoder(Encoder_Left, Encoder_Right); // ����������ת�ٶȣ�mm/s��                                       					//10ms����һ��
		if (delay_flag == 1)
		{
			delay_50++;
			if (delay_50 == 10)
				delay_50 = 0, delay_flag = 0, LD_Successful_Receive_flag = 0; // ���������ṩ50ms�ľ�׼��ʱ��ʾ������Ҫ50ms�߾�����ʱ
		}


		Key(); // ɨ�谴��״̬ ����˫�����Ըı�С������״̬
		// �ٶȿ���
		int actual_velocity = Velocity(Encoder_Left, Encoder_Right); // ��ȡ�ٶȿ��Ƶ�PWM
	    Velocity(Encoder_Left, Encoder_Right);

		// TODO : ת�����
		Turn_Pwm = 0;

		// ʹ�ü������ʵ���ٶȣ����޸�Target_Velocity
		Motor_Left = actual_velocity - (float)ZoomRatio / 1000 * Turn_Pwm;
		Motor_Right = actual_velocity + (float)ZoomRatio / 1000 * Turn_Pwm;
		if (Flag_Stop == 1)
			Set_Pwm(0, 0);
		else
			Set_Pwm(Motor_Left, Motor_Right); // ��ֵ��PWM�Ĵ���
	}
}

/**************************************************************************
Function: Vertical PD control
Input   : Angle:angle��Gyro��angular velocity
Output  : balance��Vertical control PWM
�������ܣ�ֱ��PD����
��ڲ�����Angle:�Ƕȣ�Gyro�����ٶ�
����  ֵ��balance��ֱ������PWM
**************************************************************************/
int Balance(float Angle, float Gyro)
{
	float Angle_bias, Gyro_bias;
	int balance;
	Angle_bias = Middle_angle - Angle; // ���ƽ��ĽǶ���ֵ �ͻ�е���
	Gyro_bias = 0 - Gyro;
	balance = -Balance_Kp / 100 * Angle_bias - Gyro_bias * Balance_Kd / 100; // ����ƽ����Ƶĵ��PWM  PD����   kp��Pϵ�� kd��Dϵ��
	return balance;
}

/**************************************************************************
Function: Speed PI control
Input   : encoder_left��Left wheel encoder reading��encoder_right��Right wheel encoder reading
Output  : Speed control PWM
�������ܣ��ٶȿ���PWM
��ڲ�����encoder_left�����ֱ�����������encoder_right�����ֱ���������
����  ֵ���ٶȿ���PWM
**************************************************************************/
// �޸�ǰ�������ٶȣ����޸�Target_Velocity�����磬�ĳ�60�ͱȽ�����
int Velocity(int encoder_left, int encoder_right)
{
	volatile static float velocity, Encoder_Least, Encoder_bias, Movement = 0;
	volatile static float Encoder_Integral = 0;

	//================�ٶ�PI������=====================//
	Encoder_Least = Target_Velocity * 2 - (encoder_left + encoder_right); // ��ȡ�����ٶ�ƫ��=Ŀ���ٶ�-�����ٶȣ����ұ�����֮�ͣ�
	Encoder_bias *= 0.86f;												  // ���f��׺��ָ��Ϊ�����ȸ�����
	Encoder_bias += Encoder_Least * 0.14f;								  // ���f��׺��ָ��Ϊ�����ȸ�����
	Encoder_Integral += Encoder_bias;									  // ���ֳ�λ�� ����ʱ�䣺10ms
	Encoder_Integral = Encoder_Integral + Movement;						  // ����ң�������ݣ�����ǰ������
	if (Encoder_Integral > 4000)
		Encoder_Integral = 4000; // �����޷�
	if (Encoder_Integral < -4000)
		Encoder_Integral = -4000;														 // �����޷�
	velocity = -Encoder_bias * Velocity_Kp / 100 - Encoder_Integral * Velocity_Ki / 100; // �ٶȿ���
	if (Flag_Stop == 1)
		Encoder_Integral = 0; // ����رպ��������
	Velocity_Pwm = velocity;
	return velocity;
}


/**************************************************************************
Function: Assign to PWM register
Input   : motor_left��Left wheel PWM��motor_right��Right wheel PWM
Output  : none
�������ܣ���ֵ��PWM�Ĵ���
��ڲ���������PWM������PWM
����  ֵ����
**************************************************************************/
void Set_Pwm(int motor_left, int motor_right)
{
	if (motor_left > 0)
		AIN1 = 0, AIN2 = 1; // �޸�Ϊ������
	else
		AIN1 = 1, AIN2 = 0; // �޸�Ϊ������
	PWMA = myabs(motor_left);
	if (motor_right > 0)
		BIN1 = 0, BIN2 = 1; // �޸�Ϊ������
	else
		BIN1 = 1, BIN2 = 0; // �޸�Ϊ������
	PWMB = myabs(motor_right);
}
/**************************************************************************
Function: PWM limiting range
Input   : IN��Input  max��Maximum value  min��Minimum value
Output  : Output
�������ܣ�����PWM��ֵ
��ڲ�����IN���������  max���޷����ֵ  min���޷���Сֵ
����  ֵ���޷����ֵ
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
Function: Press the key to modify the car running state
Input   : none
Output  : none
�������ܣ������޸�С������״̬
��ڲ�������
����  ֵ����
**************************************************************************/
void Key(void)
{
	u8 tmp;
	tmp = click(); // ����
	if (tmp == 1)
	{
		Flag_Stop = !Flag_Stop;
		// ��ӡ������Ϣ
		printf("Flag_Stop:%d\r\n", Flag_Stop);
	} // ��������С������ͣ
}

/**************************************************************************
Function: Absolute value function
Input   : a��Number to be converted
Output  : unsigned int
�������ܣ�����ֵ����
��ڲ�����a����Ҫ�������ֵ����
����  ֵ���޷�������
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
�������ܣ�����������ת��Ϊ�ٶȣ�mm/s��
��ڲ�������
����  ֵ����
**************************************************************************/
void Get_Velocity_Form_Encoder(int encoder_left, int encoder_right)
{
	float Rotation_Speed_L, Rotation_Speed_R; // ���ת��  ת��=������������5msÿ�Σ�*��ȡƵ��/��Ƶ��/���ٱ�/����������
	Rotation_Speed_L = encoder_left * Control_Frequency / EncoderMultiples / Reduction_Ratio / Encoder_precision;
	Velocity_Left = Rotation_Speed_L * PI * Diameter_67; // ���f��׺��ָ��Ϊ�����ȸ�����
	Rotation_Speed_R = encoder_right * Control_Frequency / EncoderMultiples / Reduction_Ratio / Encoder_precision;
	Velocity_Right = Rotation_Speed_R * PI * Diameter_67; // ���f��׺��ָ��Ϊ�����ȸ�����
}

// TODO: ��ɴ�����״̬�Ļ�ȡ
/**************************************************************************
�������ܣ�ת��PID���Ƽ���
��ڲ�������
����  ֵ��ת�����PWM
**************************************************************************/
int Sensor_PID(void)
{
    
    return 0;  // ����ת�������
}

// TODO: ��ɰ�ť״̬�Ļ�ȡ

// �޸��������ⲿ����
int click(void)
{
    return 0; // �ް�������
}
