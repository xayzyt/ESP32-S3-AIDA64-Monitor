
#include "ft6336u_driver.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "xl9555.h"

#define TAG "ft6336u_driver" // 定义日志标签

// 定义 I2C 总线句柄和设备句柄（静态变量，仅本文件可见）
static i2c_master_bus_handle_t ft6336u_bus_handle = NULL;
static i2c_master_dev_handle_t ft6336u_dev_handle = NULL;
static bool ft6336u_int_flag = false; // 用于记录是否有触摸中断发生的标志位

// --- FT6336U 初始化函数 ---
// 参数：sda (数据引脚), scl (时钟引脚)
esp_err_t ft6336u_driver(gpio_num_t sda, gpio_num_t scl)
{
    // 配置 I2C 主机总线参数
    i2c_master_bus_config_t bus_config =
    {
        .clk_source = I2C_CLK_SRC_DEFAULT, // 使用默认时钟源
        .sda_io_num = sda,                 // 设置 SDA 引脚
        .scl_io_num = scl,                 // 设置 SCL 引脚
        .glitch_ignore_cnt = 7,            // 滤除毛刺的计数周期
        .i2c_port = -1,                    // 自动选择 I2C 端口
    };
    // 创建新的 I2C 主机总线
    i2c_new_master_bus(&bus_config, &ft6336u_bus_handle);

    // 配置 I2C 从设备参数 (FT6336U)
    const i2c_device_config_t dev_config =
    {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, // 7位地址模式
        .device_address = 0x38,                // FT6336U 的 I2C 设备地址 (0x38)
        .scl_speed_hz = 400 * 1000,            // I2C 通信速率 400kHz
    };
    // 将 FT6336U 设备添加到 I2C 总线上，获取设备句柄
    i2c_master_bus_add_device(ft6336u_bus_handle, &dev_config, &ft6336u_dev_handle);

    uint8_t read_buf[1];
    uint8_t write_buf[2];
    
    // 尝试读取芯片ID或状态，验证通信是否正常
    write_buf[0] = 0xA3; // 寄存器地址 0xA3 (通常用于读取芯片 ID 信息)
    esp_err_t ret = ESP_FAIL;
    // 发送寄存器地址并读取1字节数据，超时时间500ms
    ret = i2c_master_transmit_receive(ft6336u_dev_handle, write_buf, 1, read_buf, 1, 500);
    if(ret != ESP_OK)
    {
        return ret; // 如果通信失败，直接返回错误
    }

    // 配置中断模式
    write_buf[0] = 0xA4; // 寄存器 0xA4 (中断模式控制)
    write_buf[1] = 0x00; // 写入 0x00 (通常设置为轮询模式或特定的中断触发模式)
    // 发送配置命令
    ret = i2c_master_transmit(ft6336u_dev_handle, write_buf, 2, 500);
    return ret; // 返回初始化结果
}

// --- 读取触摸坐标函数 ---
// 参数：x, y (坐标指针), state (按下状态指针)
void ft6335u_read(int16_t* x, int16_t* y, int* state)
{
    uint8_t read_buf[5];  // 缓冲区：存放读回的触摸数据
    uint8_t write_buf[1]; // 缓冲区：存放要读取的寄存器地址
    static int16_t last_x = 0; // 静态变量，保存上一次的 X 坐标
    static int16_t last_y = 0; // 静态变量，保存上一次的 Y 坐标

    // 如果没有中断标志（表示没有触摸事件），返回上一次的坐标
    if(!ft6336u_int_flag)
    {
        *x = last_x;
        *y = last_y;
        *state = 0; // 状态设为未按下
        return;
    }

    write_buf[0] = 0x02; // 从寄存器 0x02 开始读取 (TD_STATUS: 触摸点数)
    // 读取5个字节的数据：
    // Reg 0x02: 触摸点数
    // Reg 0x03: 触摸点1 X坐标高位 & 事件标志
    // Reg 0x04: 触摸点1 X坐标低位
    // Reg 0x05: 触摸点1 Y坐标ID & Y坐标高位
    // Reg 0x06: 触摸点1 Y坐标低位
    i2c_master_transmit_receive(ft6336u_dev_handle, write_buf, 1, read_buf, 5, 500);

    // 检查 read_buf[0] 的低4位，判断触摸点数量是否为1
    if((read_buf[0] & 0x0f) != 1)
    {
        // 如果不是单点触摸，返回旧数据
        *x = last_x;
        *y = last_y;
        *state = 0;
        return;
    }

    // 解析 X 坐标：(高位 & 0x0F) 左移 8 位 | 低位
    int16_t current_x = ((read_buf[1] & 0x0F) << 8) | read_buf[2];
    // 解析 Y 坐标：(高位 & 0x0F) 左移 8 位 | 低位
    int16_t current_y = ((read_buf[3] & 0x0F) << 8) | read_buf[4];

    // 更新静态变量
    last_x = current_x;
    last_y = current_y;

    // 赋值给输出参数
    *x = current_x;
    *y = current_y;
    *state = 1; // 状态设为已按下
}

// --- 设置中断标志位的函数 ---
// 由外部中断回调函数调用
void ft6336u_int_info(bool flag)
{
    ft6336u_int_flag = flag; // 更新全局中断标志
}
