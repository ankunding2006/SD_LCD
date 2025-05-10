/**
 * @file car_config.h
 * @brief 小车控制系统配置参数
 * @details 包含小车控制系统的各种参数配置，如PID参数、运行模式选择等
 */

#ifndef __CAR_CONFIG_H
#define __CAR_CONFIG_H

/*=========================== 运行模式配置 ===========================*/
/**
 * @brief 运行模式选择
 * @note 只能有一个模式为1，其他必须为0
 */
#define TEST_MODE 0   // 单项测试模式
#define Normal_Mode 0 // 正常模式
#define TASK1 0       // 任务1：A点直线前进到B点
#define TASK2 0       // 任务2：A->B->C->D->A
#define TASK3 0       // 任务3：A->C->B->D->A
#define TASK4 0       // 任务4：A->C->B->D->A 重复四圈

/*=========================== 调试功能配置 ===========================*/
/**
 * @brief 调试相关配置
 */
#define NOLINEDETECT 0              // 调试模式下是否禁用灰度传感器进行循迹(1=禁用)
#define PRINTF_ANGLE 0              // 是否打印角度数据(1=打印)
#define SET_ANGLE_WITH_KEY 0        // 是否手动通过按钮设置角度值(1=手动)
#define IS_SET_INIT_ANGLE_MANUALY 1 // 是否手动设置目标角度(1=手动)
#define MANUAL_INIT_ANGLE 146       // 手动设置的目标角度(度)

/*=========================== 功能测试选择 ===========================*/
/**
 * @brief 单项测试模式下的功能选择
 * @note 仅在TEST_MODE=1时有效，只能有一个为1
 */
#define TEST_STEERING_ROTATION 0         // 测试相对转向旋转
#define TEST_TRACKING 0                  // 测试循迹
#define TEST_TURNTO_ABSLUTE_ANGLE 0      // 测试转向到绝对角度
#define TEST_MOVE_FORWARD 0              // 测试直线前进
#define MOVE_FORWARD_WITH_ANGLE_HANDLE 0 // 测试指定角度前进
#define TEST_MOVE_FORWARD_WITH_ANGLE_UNTILE_SOME_GRAY_SENCOR_ACTIVED \
  1 // 测试指定角度前进，直到某个灰度传感器激活

/*=========================== 系统默认状态 ===========================*/
/**
 * @brief 系统默认状态配置
 */
#define GYROSCOPE_ON_DEFAULT 0       // 默认陀螺仪状态(1=开启)
#define FLAG_STOP_DEFAULT 0          // 默认停止状态(1=停止)
#define FLAG_SHOW_DEFAULT 0          // 默认显示状态(1=显示)
#define MODE_DEFAULT 0               // 初始模式选择(0=普通控制模式)
#define LONG_PRESS_THRESHOLD 1300    // 长按阈值(毫秒)
#define WAY_ANGLE_DEFAULT 1          // 角度获取算法(1=四元数, 2=卡尔曼, 3=互补滤波)
#define LINE_TRACKING_DEAD_TIME 1000 // 循迹死区时间(毫秒)
#define BEEP_ON_TIME 600             // 蜂鸣器开启时间(毫秒)

/*=========================== 速度控制参数 ===========================*/
/**
 * @brief 速度控制相关参数
 */
#define TARGET_VELOCITY_DEFAULT 15     // 默认目标速度
#define FORWARDBASE_PWM 2500           // 直线行驶基础PWM值
#define OPENLOOP_STEERING_BASE_PWM 840 // 开环转向基础PWM值

/*=========================== PID控制参数 ===========================*/
/**
 * @brief 平衡与速度PID参数(放大100倍)
 */
#define BALANCE_KP_DEFAULT 25500  // 平衡控制比例系数
#define BALANCE_KD_DEFAULT 135    // 平衡控制微分系数
#define VELOCITY_KP_DEFAULT 16000 // 速度控制比例系数
#define VELOCITY_KI_DEFAULT 120   // 速度控制积分系数
#define TURN_KP_DEFAULT 17000     // 转向控制比例系数
#define TURN_KD_DEFAULT 100       // 转向控制微分系数

/**
 * @brief 原地转向控制PID参数(放大100倍)
 */
#define STEERING_KP_DEFAULT 3000             // 转向控制比例系数
#define STEERING_KI_DEFAULT 30               // 转向控制积分系数
#define STEERING_KD_DEFAULT 80               // 转向控制微分系数
#define STEERING_ERROR_THRESHOLD_DEFAULT 225 // 转向控制误差阈值(度)
#define STEERING_STABLE_TIME 70              // 转向稳定需要保持的时间计数
#define STEERING_MAX_OUTPUT 800              // 转向控制最大PWM输出
#define STEERING_MIN_OUTPUT -800             // 转向控制最小PWM输出
#define STEERING_I_LIMIT 2000                // 转向控制积分限幅值
#define STEERING_SPEED_DEFAULT 5000          // 转向控制基础速度
#define PWM_Base 875                         // PWM基准值

/**
 * @brief 直线行驶角度修正PID参数(放大100倍)
 */
#define FORWARD_KP_DEFAULT 10000    // 直线行走角度修正比例系数
#define FORWARD_KI_DEFAULT 2        // 直线行走角度修正积分系数
#define FORWARD_KD_DEFAULT 0        // 直线行走角度修正微分系数
#define FORWARD_ERROR_THRESHOLD 200 // 直线行走角度修正误差阈值(度)
#define FORWARD_I_LIMIT 1000        // 直线行走积分限幅值
#define FORWARD_GraySENSOR_INDEX_1 7, 12
#define FORWARD_GraySENSOR_INDEX_2 1, 6

/**
 * @brief 灰度传感器循迹PID参数
 */
#define SENSOR_KP_DEFAULT 640 // 传感器比例系数
#define SENSOR_KI_DEFAULT 2.1 // 传感器积分系数
#define SENSOR_KD_DEFAULT 115 // 传感器微分系数
#define turn_PWM_Limit 1500   // 循迹转向PWM的限制值

/*=========================== 传感器参数配置 ===========================*/
/**
 * @brief 陀螺仪传感器配置参数
 */
#define GYRO_CHECK_THRESHOLD 1.0f // 陀螺仪数据连续读取差值阈值(1.0度)
#define GYRO_CHECK_INTERVAL 300   // 陀螺仪初始化稳定等待时间(ms)
#define GYRO_INIT_RETRIES 5       // 陀螺仪初始化重试次数
#define MIDDLE_ANGLE_DEFAULT 0    // 初始平衡角度设定

/*=========================== 调试信息配置 ===========================*/
/**
 * @brief 调试信息输出配置
 */
#define DEBUG_PRINT_INTERVAL 500                     // 调试信息发送间隔(ms)
#define DEBUG_PRINT_COUNT (DEBUG_PRINT_INTERVAL / 5) // 调试信息发送计数(基于5ms的中断周期)
#define ANGLE_PRINT_COUNT 1500000                    // 角度打印计数

/*=========================== 任务1参数配置 =========================== */

/*=========================== 任务2参数配置 =========================== */
#define TASK2_ANGLE_OFFSET 15.0f // 初始角度偏移

/*=========================== 任务3参数配置 ===========================*/
/**
 * @brief 任务3(A->C->B->D->A)的参数配置
 */
#define TASK3_ROTATION_ANGLE_1 33.5f // 初始从A点对准C点需要顺时针旋转的角度(度)
#define TASK3_ROTATION_ANGLE_2 69.5f // 从B到D前需要逆时针旋转的角度(度)
#define TASK3_STEER_TIME_C 500       // C点openLoopSteering的转向时间参数(中断次数)
#define TASK3_STEER_PWM_C 700        // C点openLoopSteering的PWM参数(速度值)
#define TASK3_STEER_TIME_D -500      // D点openLoopSteering的转向时间参数(中断次数)
#define TASK3_STEER_PWM_D 600        // D点openLoopSteering的PWM参数(速度值)
#define TASK3_MOVE_FORWARD_SPEED 15  // 直线行驶的速度(速度值)
#define TASK3_LINE_TRACKING_SPEED 15 // 循迹行驶的速度(速度值)
#define TASK3_B_PONIT_DELAY_TIME 60  // B点延时时间(毫秒)
#define RESET_WAIT_TIME 5000         // 重置等待时间
#define TASK3_INTERVAL 3000          // 任务间隔时间(毫秒)

/*=========================== 任务4参数配置 ===========================*/
/**
 * @brief 任务4(A->C->B->D->A 重复四圈)的参数配置
 */
#define TASK4_ROTATION_ANGLE_1 TASK3_ROTATION_ANGLE_1       // 初始从A点对准C点需要顺时针旋转的角度(度)
#define TASK4_ROTATION_ANGLE_2 TASK3_ROTATION_ANGLE_2       // 从B到D前需要逆时针旋转的角度(度)
#define TASK4_ROTATION_ANGLE_3 31.1f                        // 后3圈从B到D前需要逆时针旋转的角度(度)
#define TASK4_STEER_TIME_C TASK3_STEER_TIME_C               // C点openLoopSteering的转向时间参数
#define TASK4_STEER_PWM_C TASK3_STEER_PWM_C                 // C点openLoopSteering的PWM参数
#define TASK4_STEER_TIME_D TASK3_STEER_TIME_D               // D点openLoopSteering的转向时间参数
#define TASK4_STEER_PWM_D TASK3_STEER_PWM_D                 // D点openLoopSteering的PWM参数
#define TASK4_CYCLE_COUNT 4                                 // 循环执行次数
#define TASK4_INTERVAL 50                                   // 任务间隔时间(毫秒)
#define TASK4_LINE_TRACKING_SPEED TASK3_LINE_TRACKING_SPEED // 循迹行驶的速度
#define TASK4_MOVE_FORWARD_SPEED TASK3_MOVE_FORWARD_SPEED   // 任务4中直线行驶的速度(速度值)
#define TASK4_B_PONIT_DELAY_TIME TASK3_B_PONIT_DELAY_TIME   // B点延时时间(毫秒)

/*=========================== 稳定性检测参数 ===========================*/
/**
 * @brief moveForward稳定性检测参数
 */
#define MOVE_FORWARD_INIT_SAMPLES 6    // moveForward函数初始化时读取角度值的次数
#define MOVE_FORWARD_MAX_DEVIATION 2.0 // moveForward函数初始化时允许的最大角度偏差(度)

#endif /* __CAR_CONFIG_H */
