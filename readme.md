# About

This project was made to review (or practice) FreeRTOS.

## Hardware

-   ESP32 (esp32doit-devkit-v1)
-   DHT11 sensor (temperature and humidity)
-   OLED 128x64 (I2C)
-   SDCard (Currently unavailable)
-   Button (External pull-up ciruit)
-   LEDs (Two LED with 1KOhm resistor)
-   Variable resistance (Voltage divider circuit, 0->3.3V)

## VSCode-PlatformIO Config 

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = espidf

monitor_speed = 115200
upload_speed = 921600
upload_port = /dev/ttyUSB0
```

# PART-1: Drivers

This part focuses on writing drivers to control I/O. Additionally, this part will familiarize you with ESP-IDF development, including project structure, build process, and debugging.

## OLED display on 128x64

## Measure temperature and humidity

## Read ADC

## SDCard communication

# PART-2: Multitask system

This part focuses on building a real-time system. Reuse the driver written in the previous part to simultaneously display on the OLED, read values from sensors, and control LEDs.

## Tracking environment

## Controll LED using VR (variable resistance)

## Combination

