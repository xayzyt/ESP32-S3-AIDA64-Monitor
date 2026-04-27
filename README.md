# ESP32-S3 AIDA64 Monitor

基于 ESP32-S3 的桌面监控终端，使用 ESP-IDF、FreeRTOS 和 LVGL 开发。设备可以显示网络时间、天气信息和 AIDA64 RemoteSensor 提供的 PC 硬件状态。

![C](https://img.shields.io/badge/C-00599C?style=flat-square&logo=c&logoColor=white)
![ESP-IDF](https://img.shields.io/badge/ESP--IDF-E7352C?style=flat-square&logo=espressif&logoColor=white)
![FreeRTOS](https://img.shields.io/badge/FreeRTOS-173B6C?style=flat-square)
![LVGL](https://img.shields.io/badge/LVGL-343839?style=flat-square)

## 功能

- AIDA64 数据显示：CPU、内存、温度等信息
- 网络时间同步：SNTP
- 天气信息获取：HTTP + cJSON
- Web 配网：AP 模式下通过网页配置 Wi-Fi
- LVGL 图形界面
- 3.5 寸 8080 并口屏幕驱动
- FT6336U 触摸驱动
- XL9555 IO 扩展芯片驱动

## 硬件

| 模块 | 型号 / 说明 |
| --- | --- |
| MCU | ESP32-S3 |
| 屏幕 | 3.5 寸 TFT LCD，320x240，8080 并口 |
| 触摸 | FT6336U |
| IO 扩展 | XL9555 |
| 联网 | Wi-Fi |

## 引脚

| 功能 | GPIO |
| --- | --- |
| LCD_D0-D7 | 40, 38, 39, 48, 45, 21, 47, 14 |
| LCD_WR | 41 |
| LCD_DC | 1 |
| LCD_CS | 2 |
| Touch_SDA | 13 |
| Touch_SCL | 12 |
| XL9555_SDA | 10 |
| XL9555_SCL | 11 |

## 目录

| 路径 | 说明 |
| --- | --- |
| `main/main.c` | 程序入口 |
| `main/lv_port.c` | LVGL 移植 |
| `main/aida64.c` | AIDA64 数据解析 |
| `main/weather.c` | 天气请求和解析 |
| `main/ui/generated/` | LVGL 生成代码和资源 |
| `components/ap_wifi/` | 配网和 Wi-Fi 管理 |
| `components/bsp/` | 板级外设驱动 |
| `html/apcfg.html` | Web 配网页面 |

## AIDA64 设置

1. 打开 AIDA64。
2. 进入 `File -> Preferences -> Hardware Monitoring -> LCD`。
3. 启用 `RemoteSensor`。
4. 确认设备和电脑在同一局域网。
5. 在 AIDA64 中添加需要显示的传感器项目。

## 构建

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p COMx flash monitor
```
