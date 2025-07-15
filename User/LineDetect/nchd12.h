#ifndef __NCHD12_H
#define __NCHD12_H
#include "main.h"
/************************************************************PCA9555 驱动***********************************************************/
#define SUCCESS 0
#define ERROR 1

#define SLAVE_ADDR0 0x40

#define HOST_WRITE_COMMAND 0x00
#define HOST_READ_COMMAND 0x01

#define INPUT_PORT_REGISTER0 0x00              /* 输入端口寄存器0，负责IO00-IO07 */
#define INPUT_PORT_REGISTER1 0x01              /* 输入端口寄存器1，负责IO10-IO17 */
#define OUTPUT_PORT_REGISTER0 0x02             /* 输入端口寄存器0，负责IO00-IO07 */
#define OUTPUT_PORT_REGISTER1 0x03             /* 输入端口寄存器1，负责IO10-IO17 */
#define POLARITY_INVERSION_PORT_REGISTER0 0x04 /* 极性反转端口寄存器0，负责IO00-IO07 */
#define POLARITY_INVERSION_PORT_REGISTER1 0x05 /* 极性反转端口寄存器1，负责IO10-IO17 */
#define CONFIG_PORT_REGISTER0 0x06             /* 输入端口寄存器0，负责IO00-IO07 */
#define CONFIG_PORT_REGISTER1 0x07             /* 输入端口寄存器1，负责IO10-IO17 */

#define GPIO_PORT0 0
#define GPIO_PORT1 1

#define GPIO_0 0x01
#define GPIO_1 0x02
#define GPIO_2 0x04
#define GPIO_3 0x08
#define GPIO_4 0x10
#define GPIO_5 0x20
#define GPIO_6 0x40
#define GPIO_7 0x80

uint16_t pcf8575_read_bit12(uint8_t slave_num);
uint16_t pca9555_read_bit12(uint8_t slave_num);

#endif
