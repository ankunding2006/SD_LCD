#define TEST_MODE    // 测试模式 
//#define NOLINEDETECT // 调试模式，不使用灰度传感器

// 原地转向控制相关宏定义
#define STEERING_STABLE_TIME        70    // 转向稳定需要保持的时间计数
#define STEERING_MAX_OUTPUT        3000    // 转向控制最大PWM输出
#define STEERING_MIN_OUTPUT       -3000    // 转向控制最小PWM输出
#define STEERING_I_LIMIT          1000     // 转向控制积分限幅值
#define PWM_Base                  1000	   // PWM基准值

#define turn_PWM_Limit 1500 // 循迹转向PWM的限制值，防止过大过小

// 角度获取算法选择
#define WAY_ANGLE_DEFAULT           1    // 1：四元数  2：卡尔曼  3：互补滤波

// 运行控制标志位默认值
#define FLAG_STOP_DEFAULT           1    // 默认停止
#define FLAG_SHOW_DEFAULT           0    // 默认显示打开

// 初始模式选择
#define MODE_DEFAULT                0    // 默认为普通控制模式

// 初始角度设定
#define MIDDLE_ANGLE_DEFAULT        0    // 初始平衡角度设定

// 速度和PID控制参数 (放大100倍)
#define TARGET_VELOCITY_DEFAULT     16   // 默认目标速度
#define BALANCE_KP_DEFAULT      25500    // 平衡控制比例系数
#define BALANCE_KD_DEFAULT        135    // 平衡控制微分系数
#define VELOCITY_KP_DEFAULT     16000    // 速度控制比例系数
#define VELOCITY_KI_DEFAULT       120    // 速度控制积分系数
#define TURN_KP_DEFAULT         17000    // 转向控制比例系数
#define TURN_KD_DEFAULT           100    // 转向控制微分系数

// 灰度传感器PID参数
#define SENSOR_KP_DEFAULT          640   // 传感器比例系数
#define SENSOR_KI_DEFAULT          2.1   // 传感器积分系数
#define SENSOR_KD_DEFAULT          115   // 传感器微分系数

// 转向控制PID参数及相关变量（放大100倍）
#define STEERING_KP_DEFAULT        200   // 转向控制比例系数
#define STEERING_KI_DEFAULT        180   // 转向控制积分系数
#define STEERING_KD_DEFAULT        100   // 转向控制微分系数
#define STEERING_ERROR_THRESHOLD_DEFAULT 500   // 转向控制误差阈值(度)
#define STEERING_SPEED_DEFAULT    5000   // 转向控制基础速度
