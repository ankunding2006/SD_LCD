#ifndef CAR_CONFIG
#define CAR_CONFIG

#define TEST_IN_MAIN 0
#define TEST_IN_INTERRUPT 1 // 在中断中进行测试
#define TEST_BEFORE_MAIN 0

#define TEST_TASK 1                    // 任务测试
#define TEST_DRIVER 0                  // 驱动测试
#define TASK_RUN_ONLY_ONCE 1           // 任务是否只运行一次
#define TEST_DRIVER_EMM_V5 0           // Emm_V5驱动直接测试
#define TEST_CONTROL_SET_MOTOR_SPEED 1 //  set_motor_speed 函数测试

#define TEST_MOTOR_ADDR 0x01 // 要测试的电机地址
#define TEST_MOTOR_SPEED 200 // 驱动测试中的电机速度 (RPM)

#define TEST_CAR_TO_ROOM 1
#define TEST_ROOM_NUM 1
#define TEST_CAR_TO_CROSSING 0
#define TEST_CROSSING_NUM 2
#define TEST_OPEN_LOOP_STEERING 0
#define TEST_OPEN_LOOP_STEERING_TIME 1000
#define TEST_OPEN_LOOP_STEERING_SPEED 40
#define TEST_OPEN_LOOP_STEERING_SPEED_BASE 70

#define TEST_REPEAT_DELAY_S 5 // 两次测试任务之间的延迟时间（秒）

// Motion parameters
#define CAR_BASE_SPEED 50        // 基础循迹速度 (单位:转/分钟)
#define CAR_MAX_SPEED 100        // 最大循迹速度 (单位:转/分钟)
#define DEFAULT_ACCELERATION 230 // 默认电机加速度 (0-255)

// Steering control parameters
#define TURN_90_DURATION_MS 500 // 90度转向所需时间 (ms)
#define TURN_90_SPEED 50        // 90度转向时的电机差速 (RPM)
#define TURN_90_BASE_SPEED 30   // 90度转向时的基础速度 (RPM)

#define TURN_180_DURATION_MS 1000 // 180度转向所需时间 (ms)
#define TURN_180_SPEED 70         // 180度转向时的电机差速 (RPM)
#define TURN_180_BASE_SPEED 0     // 180度转向时的基础速度 (0表示原地转向)

#define SCALE_FACTOR 6 // 比例系数

// State machine timing parameters
#define CROSSING_DEBOUNCE_MS 1000     // 识别为新路口的防抖时间 (ms)
#define LEAVING_CROSSING_DELAY_MS 500 // 确认离开路口的延迟时间 (ms)

// Gray sensor parameters
#define CONSECUTIVE_GRAY_SENSORS 6 // 连续灰度传感器数量

#endif
