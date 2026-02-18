#ifndef __XL9555_H
#define __XL9555_H

#include "driver/gpio.h"
#include "esp_err.h"
#include "driver/i2c_master.h"


/* XL9555命令宏 */
#define XL9555_INPUT_PORT0_REG      0                               /* 输入寄存器0地址 */
#define XL9555_INPUT_PORT1_REG      1                               /* 输入寄存器1地址 */
#define XL9555_OUTPUT_PORT0_REG     2                               /* 输出寄存器0地址 */
#define XL9555_OUTPUT_PORT1_REG     3                               /* 输出寄存器1地址 */
#define XL9555_INVERSION_PORT0_REG  4                               /* 极性反转寄存器0地址 */
#define XL9555_INVERSION_PORT1_REG  5                               /* 极性反转寄存器1地址 */
#define XL9555_CONFIG_PORT0_REG     6                               /* 方向配置寄存器0地址 */
#define XL9555_CONFIG_PORT1_REG     7                               /* 方向配置寄存器1地址 */

#define XL9555_ADDR                 0x20                            /* XL9555地址(左移了一位)-->请看手册（9.1. Device Address） */

/* XL9555各个IO的功能 */
#define IO0_0                       0x0001
#define IO0_1                       0x0002
#define IO0_2                       0x0004
#define IO0_3                       0x0008
#define IO0_4                       0x0010
#define IO0_5                       0x0020
#define IO0_6                       0x0040
#define IO0_7                       0x0080
#define IO1_0                       0x0100
#define IO1_1                       0x0200
#define IO1_2                       0x0400
#define IO1_3                       0x0800
#define IO1_4                       0x1000
#define IO1_5                       0x2000
#define IO1_6                       0x4000
#define IO1_7                       0x8000

typedef void(*xl9555_input_cb_t)(uint16_t io_num,int level);

 /** 初始化xl9555芯片和用到的i2c总线
 * @param sda sda的gpio口
 * @param scl scl的gpio口
 * @param int_io 中断的gpio口
 * @param f 回调函数用于告知gpio口的电平跳变
 * @return 无 
*/
void xl9555_init(gpio_num_t sda,gpio_num_t scl,gpio_num_t int_io,xl9555_input_cb_t f);

/** 读取某个gpio口的电平
 * @param pin gpio口
 * @return 电平值
*/
int xl9555_pin_read(uint16_t pin);

/** 设置某个gpio口的电平
 * @param pin gpio口
 * @param val 电平值
 * @return 无
*/
esp_err_t xl9555_pin_write(uint16_t pin, int val);

/** 写入16位的数据
 * @param ret寄存器地址
 * @param data 要写入的数据，XL9555一般是2个字节，所以这里用16位值
 * @return 成功或失败
*/
esp_err_t xl9555_write_word(uint8_t reg, uint16_t data);

/** 读取16位的GPIO电平值
 * @param reg 寄存器地址
 * @param data 寄存器地址的值
 * @return 成功或失败
*/
esp_err_t xl9555_read_word(uint8_t reg, uint16_t *data); 

/** 设置输入输出
 * @param config_value 配置值，如果对应的位是0，则对应的GPIO口设置为【输出】，如果是1，则为【输入】
 * @return 无
*/
esp_err_t xl9555_ioconfig(uint16_t config_value);
#endif
