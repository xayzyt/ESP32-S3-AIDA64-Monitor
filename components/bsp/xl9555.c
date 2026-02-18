#include "xl9555.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string.h>

static i2c_master_bus_handle_t xl9555_i2c_master = NULL;
static i2c_master_dev_handle_t xl9555_i2c_device  =NULL;
static xl9555_input_cb_t    xl9555_input_callback = NULL;
static gpio_num_t xl9555_isr_io = GPIO_NUM_NC;
static uint16_t xl9555_io_config = 0xFFFF;
static EventGroupHandle_t   xl9555_isr_event = NULL;
#define TAG "xl9555"

//交换二值
#define SWAP_NUM(X,Y)   do{\
    (X)=X^(Y);\
    (Y)=(X)^(Y);\
    (X)=(X)^(Y);\
}while(0)

#define XL9555_ISR_BIT    BIT0 
#define LITTLE_ENDIAN   0
#define BIG_ENDIAN      1

//判断字节序
static inline int check_endian(void)
{
    static const uint32_t endian_value = 0x12345678;
    const char* endian_array = (const char*)&endian_value;
    if(endian_array[0] == 0x78)
        return LITTLE_ENDIAN;
    else if(endian_array[0] == 0x12)
        return BIG_ENDIAN;
    return -1;
}

/** 读取16位的GPIO电平值
 * @param data 返回的16个GPIO值，用位表示
 * @return 成功或失败
*/
esp_err_t xl9555_read_word(uint8_t reg,uint16_t *data)
{
    uint8_t memaddr_buf[1];
    memaddr_buf[0]  = reg;
    esp_err_t ret = i2c_master_transmit_receive(xl9555_i2c_device, &memaddr_buf[0], 1, (uint8_t*)data, 2, 500);
    if(check_endian() == BIG_ENDIAN)
    {
        //如果是大端字节序就交换高低两个字节，16位变量低位存放的是XL9555的前8个数据
        uint8_t* value_array = (uint8_t*)data;
        SWAP_NUM(value_array[0],value_array[1]);
    }
    return ret;
}

/** 写入16位的数据
 * @param ret寄存器地址
 * @param data 要写入的数据，XL9555一般是2个字节，所以这里用16位值
 * @return 成功或失败
*/
esp_err_t xl9555_write_word(uint8_t reg, uint16_t data)
{
    esp_err_t ret;
    uint8_t *write_buf = (uint8_t*)malloc(2 + 1);
    if(!write_buf)
        return ESP_FAIL;
    write_buf[0] = reg;
    memcpy(&write_buf[1],&data,2);
    if(check_endian() == BIG_ENDIAN)
    {
        //如果是大端字节序就需要交换
        SWAP_NUM(write_buf[1],write_buf[2]);
    }
    ret = i2c_master_transmit(xl9555_i2c_device, write_buf, 2 + 1, 500);
    free(write_buf);
    return ret;
}


/** 设置某个gpio口的电平
 * @param pin gpio口
 * @param val 电平值
 * @return 无
*/
esp_err_t xl9555_pin_write(uint16_t pin, int val)
{
    uint16_t r_data;
    xl9555_read_word(XL9555_INPUT_PORT0_REG,&r_data);
    if(val)
        r_data |= pin;
    else
        r_data &= ~pin;
    return xl9555_write_word(XL9555_OUTPUT_PORT0_REG, r_data);
}

/** 读取某个gpio口的电平
 * @param pin gpio口
 * @return 电平值
*/
int xl9555_pin_read(uint16_t pin)
{
    uint16_t r_data = 0;
    xl9555_read_word(XL9555_INPUT_PORT0_REG,&r_data);
    return (r_data & pin) ? 1 : 0;
}

/** 设置输入输出
 * @param config_value 配置值，如果对应的位是0，则对应的GPIO口设置为【输出】，如果是1，则为【输入】
 * @return 无
*/
esp_err_t xl9555_ioconfig(uint16_t config_value)
{
    esp_err_t err;
    xl9555_io_config = config_value;
    do
    {
        err = xl9555_write_word(XL9555_CONFIG_PORT0_REG, config_value);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "%s configure %X failed, ret: %d", __func__, config_value, err);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    } while (err != ESP_OK);
    return err;
}

/**
 * 外部中断服务函数
 * @param arg 中断引脚号，在注册中断回调函数时已通过参数带进来
 * @return 无
 */
static void IRAM_ATTR xl9555_exit_gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    BaseType_t task_woken;
    if (gpio_num == xl9555_isr_io)
    {
        if (gpio_get_level(xl9555_isr_io) == 0)
        {
            xEventGroupSetBitsFromISR(xl9555_isr_event,XL9555_ISR_BIT,&task_woken);
        }
    }
}

/**
 * 外部中断初始化程序
 * @param 无
 * @return 无
 */
static void xl9555_isr_init(void)
{
    gpio_config_t gpio_init_struct;
    xl9555_isr_event = xEventGroupCreate();
    /* 配置BOOT引脚和外部中断 */
    gpio_init_struct.mode = GPIO_MODE_INPUT;                    /* 选择为输入模式 */
    gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;           /* 上拉使能 */
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;      /* 下拉失能 */
    gpio_init_struct.intr_type = GPIO_INTR_NEGEDGE;             /* 下降沿触发 */
    gpio_init_struct.pin_bit_mask = 1ull << xl9555_isr_io;      /* 设置的引脚的位掩码 */
    gpio_config(&gpio_init_struct);                             /* 配置使能 */
    
    /* 注册中断服务 */
    gpio_install_isr_service(0);
    
    /* 设置GPIO的中断回调函数 */
    gpio_isr_handler_add(xl9555_isr_io, xl9555_exit_gpio_isr_handler, (void*) xl9555_isr_io);
}

static void xl9555_intput_scan(void* param)
{
    esp_err_t ret;
    EventBits_t ev;
    uint16_t last_input = 0;
    xl9555_read_word(XL9555_INPUT_PORT0_REG,&last_input); 
    while(1)
    {
        uint16_t input;
        ev = xEventGroupWaitBits(xl9555_isr_event,XL9555_ISR_BIT,pdTRUE,pdFALSE,pdMS_TO_TICKS(10*1000));
        if(ev & XL9555_ISR_BIT)
        {
            if ( gpio_get_level(xl9555_isr_io) != 0)
            {
                continue;
            }
            ret = xl9555_read_word(XL9555_INPUT_PORT0_REG,&input);        //读取输入寄存器
            if(ret == ESP_OK)
            {
                for(int i = 0;i < 16;i++)
                {
                    if(xl9555_io_config & (1 <<i))//判断是否已经将对应端口设置为输入
                    {
                        uint8_t value = input&(1<<i)?1:0;
                        uint8_t last_value = last_input&(1<<i)?1:0;
                        if(value != last_value && xl9555_input_callback)
                        {
                            xl9555_input_callback(1<<i,value);
                        }
                    }
                }
            }
            last_input = input;
        }
    }
}

/** 初始化xl9555芯片和用到的i2c总线
 * @param sda sda的gpio口
 * @param scl scl的gpio口
 * @param int_io 中断的gpio口
 * @param f 回调函数用于告知gpio口的电平跳变
 * @return 无 
*/
void xl9555_init(gpio_num_t sda,gpio_num_t scl,gpio_num_t int_io,xl9555_input_cb_t f)
{
    uint16_t r_data;
    i2c_master_bus_config_t bus_config = 
    {
        .i2c_port = 1,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .trans_queue_depth = 0,
        .flags.enable_internal_pullup = 1,
    };
    i2c_new_master_bus(&bus_config,&xl9555_i2c_master);

    i2c_device_config_t dev_config = 
    {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = XL9555_ADDR,
        .scl_speed_hz = 100000,
    };
    xl9555_input_callback = f;
    xl9555_isr_io = int_io;
    i2c_master_bus_add_device(xl9555_i2c_master, &dev_config, &xl9555_i2c_device);

    /* 上电先读取一次清除中断标志 */
    xl9555_read_word(XL9555_INPUT_PORT0_REG,&r_data);

    if(xl9555_isr_io != GPIO_NUM_NC)
	{
        xl9555_isr_init();
		xTaskCreatePinnedToCore(xl9555_intput_scan,"xl9555",4096,NULL,3,NULL,1);
	}
}
