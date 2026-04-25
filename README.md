# ESP32-S3 AIDA64 Monitor

A desktop smart monitor based on ESP32-S3, ESP-IDF, LVGL, and a 3.5-inch touch display. It can show PC hardware status from AIDA64, synchronize network time, and present weather-style dashboard information on an embedded screen.

## Features

- PC hardware monitoring through AIDA64 RemoteSensor
- CPU, memory, and temperature display
- LVGL-based touch UI
- Network time synchronization
- Wi-Fi data fetching
- 8080 parallel LCD interface with DMA support

## Hardware

- MCU: ESP32-S3
- Display: 3.5-inch TFT LCD, 320x240, 8080 parallel interface
- Touch controller: FT6336U
- IO expander: XL9555

## Pin Mapping

| Function | GPIO | Note |
| --- | --- | --- |
| LCD_D0-D7 | 40, 38, 39, 48, 45, 21, 47, 14 | 8080 data bus |
| LCD_WR | 41 | Write clock |
| LCD_DC | 1 | Data/command select |
| LCD_CS | 2 | Chip select |
| Touch_SDA | 13 | I2C SDA |
| Touch_SCL | 12 | I2C SCL |
| XL9555_SDA | 10 | IO expander I2C SDA |
| XL9555_SCL | 11 | IO expander I2C SCL |

## AIDA64 Setup

1. Open AIDA64.
2. Go to `File` -> `Preferences` -> `Hardware Monitoring` -> `LCD`.
3. Select `RemoteSensor`.
4. Enable RemoteSensor LCD support.
5. Set the port to match the firmware configuration.
6. Add sensor labels expected by the firmware, such as CPU usage, CPU temperature, memory usage, and memory used.

## Build

Use ESP-IDF:

```bash
idf.py menuconfig
idf.py build
idf.py -p COMx flash monitor
```

## Project Value

This project demonstrates ESP32-S3 display development, LVGL UI integration, Wi-Fi data handling, and practical embedded dashboard design.
