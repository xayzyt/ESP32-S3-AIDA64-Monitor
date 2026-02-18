# ESP32-S3 AIDA64 Desktop Monitor 🖥️

![Language](https://img.shields.io/badge/Language-C-blue.svg)
![Framework](https://img.shields.io/badge/Framework-ESP--IDF%20v5.x-green.svg)
![GUI](https://img.shields.io/badge/GUI-LVGL%208.x-orange.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)

## 📖 Introduction

This is a smart desktop monitor project based on the **ESP32-S3** microcontroller. It serves as a stylish desktop clock and a powerful PC hardware monitor.

Using Wi-Fi, it connects to your PC to display real-time CPU/GPU status via AIDA64 and fetches local weather information from the internet.

### ✨ Features

* **🖥️ PC Monitoring**: Real-time display of CPU load, temperature, RAM usage (via AIDA64 HTTP Sensor).
* **☁️ Weather Station**: Fetches live weather data using Seniverse API.
* **⏰ Network Clock**: NTP time synchronization (Aliyun/Apple/Pool) for accurate timekeeping.
* **👆 Touch Interface**: Smooth UI based on **LVGL**, supporting capacitive touch gestures.
* **⚡ High Performance**: Driven by 8-bit 8080 parallel interface with DMA for high frame rates.

## 🛠️ Hardware Setup

* **MCU**: ESP32-S3 (Xtensa® 32-bit LX7)
* **Display**: 3.5" ST7789 TFT LCD (320x240) - 8080 Parallel
* **Touch Controller**: FT6336U
* **IO Expander**: XL9555

### 📌 Pinout Mapping

| Function | GPIO | Note |
| :--- | :--- | :--- |
| **LCD_D0-D7** | 40,38,39,48,45,21,47,14 | 8080 Parallel Data Bus |
| **LCD_WR** | 41 | Write Clock |
| **LCD_DC** | 1 | Data/Command |
| **LCD_CS** | 2 | Chip Select |
| **Touch_SDA** | 13 | I2C SDA |
| **Touch_SCL** | 12 | I2C SCL |
| **XL9555_SDA** | 10 | IO Expander I2C |
| **XL9555_SCL** | 11 | IO Expander I2C |

## 💻 Configuration Guide (Important!)

To make the monitor work, you must configure **AIDA64** on your PC correctly.

1. Open AIDA64 -> `File` -> `Preferences` -> `Hardware Monitoring` -> `LCD`.
2. Select **"RemoteSensor"** protocol.
3. Enable **"Enable RemoteSensor LCD support"**.
4. **Port**: Set to `8080` (or match the port defined in `aida64.c`).
5. **LCD Items**: Add the following 4 items and set their **Labels** strictly as follows (Case Sensitive!):
    * CPU Utilization -> Label: `CPU_Rate`
    * CPU Temperature -> Label: `CPU_Temp`
    * Memory Utilization -> Label: `MEM_Rate`
    * Memory Used -> Label: `MEM_Use`

> **Note**: The code uses specific string parsing. If labels do not match, data will not show up.

## 🚀 Build & Flash

This project is built using Espressif IoT Development Framework (ESP-IDF).

```bash
# 1. Set up environment
. $HOME/esp/esp-idf/export.sh

# 2. Configure (Set WiFi SSID/Password here if needed)
idf.py menuconfig

# 3. Build the project
idf.py build

# 4. Flash and Monitor
idf.py -p COMx flash monitor
