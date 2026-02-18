#include "lv_port.h"
#include "xl9555.h"             // IO扩展芯片驱动，用于控制复位引脚和背光
#include "esp_log.h"            // ESP32日志打印库
#include "esp_lcd_panel_io.h"   // ESP LCD 通用IO接口
#include "esp_lcd_panel_vendor.h" // 包含厂家特定的LCD命令（如ST7789）
#include "esp_lcd_panel_ops.h"  // LCD操作函数（如reset, init, draw_bitmap）
#include "driver/gpio.h"        // GPIO驱动
#include "esp_lvgl_port.h"      // 官方的LVGL移植组件
#include "freertos/FreeRTOS.h"  // 操作系统核心
#include "freertos/task.h"      // 任务管理
#include "ft6336u_driver.h"     // FT6336U触摸屏驱动

#define TAG "lv_port"           // 日志标签，方便调试时过滤信息
#define LCD_WIDTH 320           // 屏幕宽度像素
#define LCD_HEIGHT 240          // 屏幕高度像素
#define LCD_RST_IO IO1_3        // 定义连接在XL9555扩展芯片上的复位引脚号

// 定义全局静态变量，只在本文件内部可见
static lv_display_t *lvgl_disp = NULL;          // LVGL显示设备的句柄（指针）
static esp_lcd_panel_io_handle_t io_handle = NULL; // 硬件IO句柄（负责通过I80总线发数据）
static esp_lcd_panel_handle_t lcd_panel = NULL;    // LCD面板句柄（负责发ST7789控制命令）
static lv_indev_t *touch_indev = NULL;          // LVGL触摸设备的句柄（指针）

// --- 触摸屏输入回调函数 ---
// LVGL 会定期调用此函数来获取当前的触摸坐标
void indev_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
    int16_t x = 0, y = 0;
    int state = 0;
    // 调用底层驱动读取物理坐标
    ft6335u_read(&x, &y, &state); 
    
    // 坐标映射转换：
    // 因为屏幕安装方向可能旋转了，这里将物理坐标转换为逻辑坐标
    // 逻辑X = 屏幕宽度 - 物理Y - 1 (相当于旋转+镜像)
    data->point.x = LCD_WIDTH - y - 1; 
    data->point.y = x;                 // 逻辑Y = 物理X
    data->state = state;               // 传递按下/抬起状态
}

// --- 硬件初始化函数（配置底层Intel 8080并口） ---
void lv_display_hard_init(void)
{
    ESP_LOGI(TAG, "Initialize Intel 8080 bus"); // 打印日志：正在初始化8080总线
    esp_lcd_i80_bus_handle_t i80_bus = NULL;    // 定义总线句柄

    // 配置8080并行总线的引脚和参数
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT, // 使用默认时钟源
        .dc_gpio_num = GPIO_NUM_1,      // DC引脚（数据/命令选择脚），接GPIO 1
        .wr_gpio_num = GPIO_NUM_41,     // WR引脚（写时钟），接GPIO 41
        .data_gpio_nums = {             // 8位并行数据线 D0-D7
            GPIO_NUM_40, GPIO_NUM_38, GPIO_NUM_39, GPIO_NUM_48,
            GPIO_NUM_45, GPIO_NUM_21, GPIO_NUM_47, GPIO_NUM_14,
        },
        .bus_width = 8,                 // 总线宽度8位
        .max_transfer_bytes = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t), // 单次最大传输字节数（一帧画面大小）
        .dma_burst_size = 64,           // DMA突发传输大小，越大通常越快但占总线
    };
    // 创建I80总线，如果有错误会报错（ESP_ERROR_CHECK宏会中止程序）
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    // 配置面板IO参数（告诉ESP32如何与ST7789通信）
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = GPIO_NUM_2,      // CS片选引脚，接GPIO 2
        .pclk_hz = 10 * 1000 * 1000,    // 像素时钟 10MHz
        .trans_queue_depth = 10,        // 传输队列深度（允许缓存多少个传输命令）
        .dc_levels = {
            .dc_idle_level = 0,         // 空闲时DC电平
            .dc_cmd_level = 0,          // 发命令时DC为低电平0
            .dc_dummy_level = 0,        // 哑周期电平
            .dc_data_level = 1,         // 发数据时DC为高电平1
        },
        .lcd_cmd_bits = 8,              // 命令字长8位
        .lcd_param_bits = 8,            // 参数字长8位
        .flags = {
            .swap_color_bytes = 1,      // 交换字节序（大小端转换），解决颜色红蓝反转问题
        },
    };
    // 创建面板IO句柄
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install LCD driver of st7789"); // 打印日志：安装ST7789驱动
    // 配置ST7789特有的参数
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = GPIO_NUM_NC,  // 复位引脚设为“未连接”，因为我们在外面用XL9555控制了
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, // 像素顺序 RGB
        .bits_per_pixel = 16,           // 色深16位（RGB565格式）
    };
    // 创建ST7789面板句柄
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &lcd_panel));

    esp_lcd_panel_reset(lcd_panel);        // 软件复位屏幕
    esp_lcd_panel_init(lcd_panel);         // 发送初始化命令序列
    esp_lcd_panel_swap_xy(lcd_panel, true); // 交换XY轴（用于横竖屏切换）
    esp_lcd_panel_mirror(lcd_panel, false, true); // 镜像翻转（参数：x不镜像，y镜像，用于矫正画面方向）
    esp_lcd_panel_invert_color(lcd_panel, false); // 颜色反转关闭（如果颜色像底片一样则需要改为true）
    esp_lcd_panel_disp_on_off(lcd_panel, true);   // 开启显示
}

// --- LVGL 整体初始化函数 ---
void lv_port_init(void)
{
    // 1. 硬件复位屏幕（通过XL9555扩展芯片控制）
    xl9555_pin_write(LCD_RST_IO, 0);       // 拉低复位引脚
    vTaskDelay(pdMS_TO_TICKS(100));        // 延时100ms（保持低电平以复位）
    xl9555_pin_write(LCD_RST_IO, 1);       // 拉高复位引脚（释放复位）
    vTaskDelay(pdMS_TO_TICKS(120));        // 延时120ms等待屏幕内部启动完成

    // 2. 初始化上面的硬件总线和屏幕参数
    lv_display_hard_init();
    
    // 初始化触摸屏驱动（连接在GPIO 13 SDA, 12 SCL）
    ft6336u_driver(GPIO_NUM_13, GPIO_NUM_12); 

    // 3. 配置 LVGL 运行任务（FreeRTOS 任务配置）
     const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,         // 任务优先级（中等偏高）
        .task_stack = 8192,         // 栈大小 8KB（防止栈溢出）
        .task_affinity = 1,         // 绑定在 CPU核心 1 上运行（核心0通常跑WiFi/蓝牙）
        .task_max_sleep_ms = 500,   // LVGL 任务最大睡眠时间
        .timer_period_ms = 5        // 心跳周期 5ms（每5ms检查一次UI刷新和输入）
    };
    lvgl_port_init(&lvgl_cfg);      // 启动 LVGL 守护任务

    ESP_LOGD(TAG, "Add LCD screen");
    // 4. 将屏幕注册到 LVGL 中
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,     // 传入前面创建的IO句柄
        .panel_handle = lcd_panel,  // 传入面板句柄
        .buffer_size = LCD_WIDTH * 80, // 显存缓冲区大小（这里设为80行，即1/3屏幕大小，以节省内存）
        .double_buffer = 0,         // 不使用双缓冲（0=单缓冲）
        .hres = LCD_WIDTH,          // 水平分辨率
        .vres = LCD_HEIGHT,         // 垂直分辨率
        .color_format = LV_COLOR_FORMAT_RGB565, // 颜色格式 RGB565
        .rotation = {
            .swap_xy = true,        // 告诉LVGL底层已经交换了XY（匹配 esp_lcd_panel_swap_xy）
            .mirror_x = false,      // X轴不镜像
            .mirror_y = true,       // Y轴镜像（匹配 esp_lcd_panel_mirror）
        },
        .flags = {
            .buff_dma = true,       // 开启DMA传输（不占用CPU搬运数据，提高刷新率）
        }
    };
    // 添加显示设备，返回指针赋值给全局变量
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    // 5. 注册触摸输入设备
    lvgl_port_lock(0); // 获取锁，防止多线程冲突
    touch_indev = lv_indev_create(); // 创建输入设备
    lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER); // 设置类型为指针（触摸屏/鼠标）
    lv_indev_set_read_cb(touch_indev, indev_read_cb);      // 设置读取数据的回调函数
    lvgl_port_unlock(); // 释放锁
}
