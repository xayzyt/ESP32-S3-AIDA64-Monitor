# ESP32-S3 AIDA64 Monitor

基于 **ESP32-S3 + ESP-IDF + FreeRTOS + LVGL** 的桌面智能监控终端。设备通过 Wi-Fi 获取天气和网络时间，也可以读取 AIDA64 RemoteSensor 数据，把 PC 的 CPU、内存、温度等状态显示到 3.5 寸触摸屏上。

![C](https://img.shields.io/badge/C-00599C?style=flat-square&logo=c&logoColor=white)
![ESP-IDF](https://img.shields.io/badge/ESP--IDF-E7352C?style=flat-square&logo=espressif&logoColor=white)
![FreeRTOS](https://img.shields.io/badge/FreeRTOS-173B6C?style=flat-square)
![LVGL](https://img.shields.io/badge/LVGL-343839?style=flat-square)
![Wi-Fi](https://img.shields.io/badge/Wi--Fi-0078D4?style=flat-square)

## 项目亮点

这不是一个只会点亮屏幕的 Demo。这个项目把屏幕驱动、触摸输入、联网配置、HTTP 数据获取、AIDA64 数据解析和 LVGL UI 刷新串成了一个完整的嵌入式应用。

```text
AIDA64 RemoteSensor -> HTTP/SSE 数据解析 -> UI 数据模型 -> LVGL -> 8080 LCD + DMA
天气 / SNTP        -> HTTP + cJSON      -> UI 更新
手机浏览器          -> AP 配网页面        -> Wi-Fi Manager
```

## 功能

- PC 硬件状态显示：CPU 使用率、内存、温度等 AIDA64 数据。
- 网络时间同步：通过 SNTP 获取时间。
- 天气信息展示：HTTP 请求天气 API，并使用 cJSON 解析结果。
- 手机 Web 配网：设备端开启 AP 和网页服务，支持浏览器配置 Wi-Fi。
- LVGL 图形界面：包含主界面、监控界面和自定义字体/图片资源。
- LCD + 触摸 + IO 扩展：适配 ST7789、FT6336U、XL9555 等外设。
- 8080 并口刷屏：使用 ESP-IDF LCD 接口和 DMA 提升显示效率。

## 硬件

| 模块 | 说明 |
| --- | --- |
| MCU | ESP32-S3 |
| Display | 3.5-inch TFT LCD, 320x240, Intel 8080 parallel interface |
| Touch | FT6336U |
| IO Expander | XL9555 |
| Network | Wi-Fi |

## 关键引脚

| Function | GPIO | Note |
| --- | --- | --- |
| LCD_D0-D7 | 40, 38, 39, 48, 45, 21, 47, 14 | 8080 data bus |
| LCD_WR | 41 | Write clock |
| LCD_DC | 1 | Data / command select |
| LCD_CS | 2 | Chip select |
| Touch_SDA | 13 | I2C SDA |
| Touch_SCL | 12 | I2C SCL |
| XL9555_SDA | 10 | IO expander I2C SDA |
| XL9555_SCL | 11 | IO expander I2C SCL |

## 仓库结构

| 路径 | 说明 |
| --- | --- |
| `main/main.c` | 应用入口，组织初始化和任务流程 |
| `main/lv_port.c` | LVGL 移植层，连接显示和输入设备 |
| `main/aida64.c` | AIDA64 RemoteSensor 数据获取与解析 |
| `main/weather.c` | 天气 API 请求和 cJSON 解析 |
| `main/ui/generated/` | LVGL UI 生成代码、字体和图片资源 |
| `components/ap_wifi/` | AP 配网、Wi-Fi 管理、WebSocket / Web 服务相关代码 |
| `components/bsp/` | 按键、触摸、XL9555 等板级外设驱动 |
| `html/apcfg.html` | 设备端 Web 配网页面 |
| `partitions_webserver.csv` | Web 服务相关分区表 |

## AIDA64 配置

1. 打开 AIDA64。
2. 进入 `File -> Preferences -> Hardware Monitoring -> LCD`。
3. 选择并启用 `RemoteSensor`。
4. 设置端口，并保证设备和 PC 在同一局域网。
5. 添加固件需要读取的传感器标签，例如 CPU 使用率、CPU 温度、内存占用等。

## 构建与烧录

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p COMx flash monitor
```

## 面试时我会重点讲

- ESP-IDF 下如何拆分组件：`ap_wifi`、`bsp`、应用层、UI 层。
- LVGL 如何和 8080 LCD、触摸输入、DMA 刷屏对接。
- HTTP/SSE 数据如何解析并安全更新到 UI。
- AP + STA 配网的状态管理和用户体验取舍。
- 为什么这个项目是“嵌入式应用闭环”，而不是单个外设实验。

