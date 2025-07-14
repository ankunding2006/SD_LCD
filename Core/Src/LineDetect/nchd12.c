#include "soft_i2c.h"
#include "nchd12.h"

uint8_t flag = 0;
uint16_t pcf8575_read_bit12(uint8_t slave_num)
{
	uint8_t hdata, ldata;
	uint16_t bit12;
	i2c_Start();
	i2c_SendByte(slave_num | HOST_READ_COMMAND);
	uint8_t ack = i2c_WaitAck();
	flag = ack;
	ldata = i2c_ReadByte(1);
	hdata = i2c_ReadByte(0);
	i2c_Stop();
	bit12 = (uint16_t)(hdata << 8 | ldata) & 0x0fff;
	return bit12;
}

uint16_t pca9555_read_bit12(uint8_t slave_num)
{
	uint8_t hdata, ldata;
	uint16_t bit12;
	i2c_Start();
	i2c_SendByte(slave_num); //	写入从机地址
	i2c_WaitAck();
	i2c_SendByte(INPUT_PORT_REGISTER0); // 写入要读取的寄存器地址
	i2c_WaitAck();
	i2c_Start();																 /* 开始接收数据 */
	i2c_SendByte(slave_num | HOST_READ_COMMAND); // 发送从机地址并设置为读取
	i2c_WaitAck();
	ldata = i2c_ReadByte(1);
	hdata = i2c_ReadByte(0);
	i2c_Stop();
	bit12 = (uint16_t)(hdata << 8 | ldata) & 0x0fff;
	return bit12;
}
