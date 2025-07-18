#include "gray_detection.h"
#include "car_config.h"

_gray_state gray_state;

float gray_status[2] = {0}, gray_status_backup[2][20] = {0}; // 灰度传感器状态与历史值
uint32_t gray_status_worse = 0;                              // 灰度管异常状态计数器
u16 scaleFactor = SCALE_FACTOR;                               // 比例系数

/***************************************************
函数名: int Calculate_Turn_Value(void)
说明:	根据灰度传感器的值计算转向值
入口:	无
出口:	转向值（左转为负，右转为正）
备注:	检测到黑线时gray_state.gray.bitX为1，否则为0
             gray_state.gray.bit1为车头最右侧的传感器的值
             gray_state.gray.bit12为最左侧传感器的值
****************************************************/
int Calculate_Turn_Value(void)
{
    // 将传感器状态和权重放入数组中，便于循环处理
    const uint8_t sensors[12] = {
        gray_state.gray.bit1, gray_state.gray.bit2, gray_state.gray.bit3,
        gray_state.gray.bit4, gray_state.gray.bit5, gray_state.gray.bit6,
        gray_state.gray.bit7, gray_state.gray.bit8, gray_state.gray.bit9,
        gray_state.gray.bit10, gray_state.gray.bit11, gray_state.gray.bit12};
    const int weights[12] = {-16, -11, -8, -5, -3, -1, 1, 3, 5, 8, 11, 16};

    int weighted_sum = 0;
    int active_sensors = 0;
    int consecutive_count = 0;
    int max_consecutive = 0;

    // 在一次循环中计算所有需要的值
    for (int i = 0; i < 12; i++)
    {
        if (sensors[i])
        {
            weighted_sum += weights[i];
            active_sensors++;
            consecutive_count++;
        }
        else
        {
            if (consecutive_count > max_consecutive)
            {
                max_consecutive = consecutive_count;
            }
            consecutive_count = 0; // 重置连续计数
        }
    }

    // 循环结束后，再次检查以捕获结尾的连续序列
    if (consecutive_count > max_consecutive)
    {
        max_consecutive = consecutive_count;
    }

    // 新的判断逻辑：只有当检测到CONSECUTIVE_GRAY_SENSORS个或更多 **连续** 的传感器时，才认为是十字路口
    if (max_consecutive >= CONSECUTIVE_GRAY_SENSORS)
    {
        return INT16_MAX; // 返回一个极大值，表示小车已经到达十字路口中心
    }

    // 如果没有传感器检测到黑线，返回一个极小值
    if (active_sensors == 0)
    {
        return INT16_MIN; // 表示没有有效的转向值
    }

    // 计算加权平均值
    float position = (float)weighted_sum / active_sensors;

    // 转换为PWM值，这里使用一个比例系数放大效果
    // 可以根据实际需求调整这个系数
    int turn_pwm = (int)(position * scaleFactor); // 比例系数20可以根据实际控制效果调整

    // 对PWM值进行限幅，防止过大过小
    if (turn_pwm > turn_PWM_Limit)
        turn_pwm = turn_PWM_Limit;
    if (turn_pwm < -turn_PWM_Limit)
        turn_pwm = -turn_PWM_Limit;

    return turn_pwm; // 返回转向值，左转为负，右转为正
}

/***************************************************
函数名: void gpio_input_check_channel_12_linewidth_10mm(void)
说明:	12路灰度管gpio检测
入口:	无
出口:	无
备注:	无
作者:	无名创新
****************************************************/
void gpio_input_check_channel_12_linewidth_10mm(void)
{
    for (uint16_t i = 19; i > 0; i--)
    {
        gray_status_backup[0][i] = gray_status_backup[0][i - 1];
    }
    gray_status_backup[0][0] = gray_status[0];
    switch (gray_state.state)
    {
    case 0x0001:
        gray_status[0] = -11;
        gray_status_worse /= 2;
        break; // 000000000001b
    case 0x0003:
        gray_status[0] = -10;
        gray_status_worse /= 2;
        break; // 000000000011b
    case 0x0002:
        gray_status[0] = -9;
        gray_status_worse /= 2;
        break; // 000000000010b
    case 0x0006:
        gray_status[0] = -8;
        gray_status_worse /= 2;
        break; // 000000000110b
    case 0x0004:
        gray_status[0] = -7;
        gray_status_worse /= 2;
        break; // 000000000100b
    case 0x000C:
        gray_status[0] = -6;
        gray_status_worse /= 2;
        break; // 000000001100b
    case 0x0008:
        gray_status[0] = -5;
        gray_status_worse /= 2;
        break; // 000000001000b
    case 0x0018:
        gray_status[0] = -4;
        gray_status_worse /= 2;
        break; // 000000011000b
    case 0x0010:
        gray_status[0] = -3;
        gray_status_worse /= 2;
        break; // 000000010000b
    case 0x0030:
        gray_status[0] = -2;
        gray_status_worse /= 2;
        break; // 000000110000b
    case 0x0020:
        gray_status[0] = -1;
        gray_status_worse /= 2;
        break; // 000000100000b
    case 0x0060:
        gray_status[0] = 0;
        gray_status_worse /= 2;
        break; // 000001100000b
    case 0x0040:
        gray_status[0] = 1;
        gray_status_worse /= 2;
        break; // 000001000000b
    case 0x00C0:
        gray_status[0] = 2;
        gray_status_worse /= 2;
        break; // 000011000000b
    case 0x0080:
        gray_status[0] = 3;
        gray_status_worse /= 2;
        break; // 000010000000b
    case 0x0180:
        gray_status[0] = 4;
        gray_status_worse /= 2;
        break; // 000110000000b
    case 0x0100:
        gray_status[0] = 5;
        gray_status_worse /= 2;
        break; // 000100000000b
    case 0x0300:
        gray_status[0] = 6;
        gray_status_worse /= 2;
        break; // 001100000000b
    case 0x0200:
        gray_status[0] = 7;
        gray_status_worse /= 2;
        break; // 001000000000b
    case 0x0600:
        gray_status[0] = 8;
        gray_status_worse /= 2;
        break; // 011000000000b
    case 0x0400:
        gray_status[0] = 9;
        gray_status_worse /= 2;
        break; // 010000000000b
    case 0x0C00:
        gray_status[0] = 10;
        gray_status_worse /= 2;
        break; // 110000000000b
    case 0x0800:
        gray_status[0] = 11;
        gray_status_worse /= 2;
        break; // 100000000000b
    case 0x0000:
        gray_status[0] = gray_status_backup[0][0];
        gray_status_worse++;
        break; // 00000000b
    default:   // 其它特殊情况单独判断
    {
        gray_status[0] = gray_status_backup[0][0];
        gray_status_worse++;
    }
    }

    static uint16_t tmp_cnt = 0;
    switch (gray_state.state) // 停止线检测
    {
    case 0x0030:
        tmp_cnt++;
        break; // 000000110000b
    case 0x0020:
        tmp_cnt++;
        break; // 000000100000b
    case 0x0060:
        tmp_cnt++;
        break; // 000001100000b
    case 0x0040:
        tmp_cnt++;
        break; // 000001000000b
    case 0x00C0:
        tmp_cnt++;
        break;   // 000011000000b
    case 0x00F0: // 000011110000b
    {
        if (tmp_cnt >= 10)
        {
            tmp_cnt = 0;
        }
    }
    break;
    }
}

void gpio_input_check_channel_12_linewidth_20mm(void)
{
    for (uint16_t i = 19; i > 0; i--)
    {
        gray_status_backup[1][i] = gray_status_backup[1][i - 1];
    }
    gray_status_backup[1][0] = gray_status[1];
    switch (gray_state.state)
    {
    case 0x0001:
        gray_status[1] = -11;
        break; // 000000000001b
    case 0x0003:
        gray_status[1] = -10;
        break; // 000000000011b
    case 0x0002:
        gray_status[1] = -9;
        break; // 000000000010b
    case 0x0007:
        gray_status[1] = -9;
        break; // 000000000111b
    case 0x0006:
        gray_status[1] = -8;
        break; // 000000000110b
    case 0x0004:
        gray_status[1] = -7;
        break; // 000000000100b
    case 0x000E:
        gray_status[1] = -7;
        break; // 000000001110b
    case 0x000C:
        gray_status[1] = -6;
        break; // 000000001100b
    case 0x0008:
        gray_status[1] = -5;
        break; // 000000001000b
    case 0x001C:
        gray_status[1] = -5;
        break; // 000000011100b
    case 0x0018:
        gray_status[1] = -4;
        break; // 000000011000b
    case 0x0010:
        gray_status[1] = -3;
        break; // 000000010000b
    case 0x0038:
        gray_status[1] = -3;
        break; // 000000111000b
    case 0x0030:
        gray_status[1] = -2;
        break; // 000000110000b
    case 0x0020:
        gray_status[1] = -1;
        break; // 000000100000b
    case 0x0070:
        gray_status[1] = -1;
        break; // 000001110000b
    case 0x0060:
        gray_status[1] = 0;
        break; // 000001100000b
    case 0x0040:
        gray_status[1] = 1;
        break; // 000001000000b
    case 0x00E0:
        gray_status[1] = 1;
        break; // 000011100000b
    case 0x00C0:
        gray_status[1] = 2;
        break; // 000011000000b
    case 0x0080:
        gray_status[1] = 3;
        break; // 000010000000b
    case 0x01C0:
        gray_status[1] = 3;
        break; // 000111000000b
    case 0x0180:
        gray_status[1] = 4;
        break; // 000110000000b
    case 0x0100:
        gray_status[1] = 5;
        break; // 000100000000b
    case 0x0380:
        gray_status[1] = 5;
        break; // 001110000000b
    case 0x0300:
        gray_status[1] = 6;
        break; // 001100000000b
    case 0x0200:
        gray_status[1] = 7;
        break; // 001000000000b
    case 0x0700:
        gray_status[1] = 7;
        break; // 011100000000b
    case 0x0600:
        gray_status[1] = 8;
        break; // 011000000000b
    case 0x0400:
        gray_status[1] = 9;
        break; // 010000000000b
    case 0x0E00:
        gray_status[1] = 9;
        break; // 111000000000b
    case 0x0C00:
        gray_status[1] = 10;
        break; // 110000000000b
    case 0x0800:
        gray_status[1] = 11;
        break; // 100000000000b
    case 0x0000:
    default: // 其它特殊情况单独判断
    {
        gray_status[1] = 0;
    }
    }
}
/**
 * @brief 更新灰度传感器数据
 * @note 通过I2C读取12路灰度传感器数据
 * 更新gray_state.gray.bit1~gray_state.gray.bit12的值
 * 其中gray_state.gray.bit1为车头最右侧的传感器的值,
 * gray_state.gray.bit12为最左侧传感器的值
 *
 */
void grey_sensor_Read(void)
{
    gray_state.state = pca9555_read_bit12(0x20 << 1); // 通过I2C方式读取12路灰度传感器数据
    gpio_input_check_channel_12_linewidth_10mm();     // 针对10mm引导线的偏差提取
    gpio_input_check_channel_12_linewidth_20mm();     // 针对20mm引导线的偏差提取
}

void grey_sensor_Init(void)
{
    i2c_CheckDevice(0x40); // 灰度传感器IIC初始化
}

void grey_sensorData_print(void)
{
    // 检测结果打印
    printf("%d%d%d%d%d%d%d%d%d%d%d%d \t %d \t %f \t %f\n",
           gray_state.gray.bit12,
           gray_state.gray.bit11,
           gray_state.gray.bit10,
           gray_state.gray.bit9,
           gray_state.gray.bit8,
           gray_state.gray.bit7,
           gray_state.gray.bit6,
           gray_state.gray.bit5,
           gray_state.gray.bit4,
           gray_state.gray.bit3,
           gray_state.gray.bit2,
           gray_state.gray.bit1,
           gray_state.state, gray_status[0], gray_status[1]);
}

/**
 * @brief 检测指定区域的灰度传感器是否有检测到黑线
 * @param start_graySensor 起始灰度传感器索引(从1开始)
 * @param end_graySensor 结束灰度传感器索引
 * @return int 返回检测到黑线的传感器值，或INT16_MIN表示没有检测到黑线
 */
int IsCertainGraySenorsAcrived(uint8_t start_graySensor, uint8_t end_graySensor)
{
    // 确保传感器索引在有效范围内
    if (start_graySensor < 1)
        start_graySensor = 1;
    if (end_graySensor > 12)
        end_graySensor = 12;

    // 检查传感器的对应位
    for (uint8_t i = start_graySensor; i <= end_graySensor; i++)
    {
        // 由于gray_state.gray.bit1对应state的最低位，所以需要计算对应的位
        // 传感器1对应二进制位0，传感器2对应二进制位1，以此类推
        if (gray_state.state & (1 << (i - 1)))
        {
            printf("传感器%d检测到黑线\n", i);
            return i;
        }
    }

    return INT16_MIN; // 未检测到黑线
}
