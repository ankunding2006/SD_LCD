#ifndef CAR_CONFIG
#define CAR_CONFIG

#define MAIN_TEST 1

// Motion parameters
#define CAR_BASE_SPEED 200       // 基础循迹速度 (单位:转/分钟)
#define CAR_MAX_SPEED 400        // 最大循迹速度 (单位:转/分钟)
#define DEFAULT_ACCELERATION 252 // 默认电机加速度 (0-255)

// Steering control parameters
#define TURN_90_DURATION_MS 500         // 90度转向所需时间 (ms)
#define TURN_180_DURATION_MS 1000       // 180度转向所需时间 (ms)
#define TURN_SPEED 150                  // 转向时的电机差速 (单位:转/分钟)
#define OPEN_LOOP_STEERING_BASE_SPEED 0 // 开环转向时的基础速度 (0表示原地转向)

// State machine timing parameters
#define CROSSING_DEBOUNCE_MS 1000     // 识别为新路口的防抖时间 (ms)
#define LEAVING_CROSSING_DELAY_MS 500 // 确认离开路口的延迟时间 (ms)

#endif
