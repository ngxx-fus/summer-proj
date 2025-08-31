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

## Dir-tree

```
    .
    ├── CMakeLists.txt
    ├── build
    │   ├── CMakeCache.txt
    │   ├── CMakeFiles
    │   ├── Makefile
    │   ├── ...
    │   └── hello_world.elf
    ├── imgs
    │   ├── i2c-arch.png
    │   ├── i2c-byte-ctl.png
    │   └── i2c-pixel-access.png
    ├── lib
    │   ├── README
    │   ├── oled128x64
    │   │   ├── i2c_com.h
    │   │   ├── oled128x64.h
    │   │   └── oled128x64_cmds.h
    │   └── text_utils
    │       ├── font_data.h
    │       └── gfxfont.h
    ├── main
    │   ├── CMakeLists.txt
    │   └── main.c
    ├── readme.md
    ├── sdkconfig
    └── sdkconfig.old
```

## ESP-IDF

### Install ESP-IDF

#### Clone ESP-IDF and install

```
git clone -b v5.2 --recursive https://github.com/espressif/esp-idf.git path/to/install/folder
cd path/to/install/folder
./install.sh
```

E.g:  path/to/install/folder : /home/fus/.fus/esp-idf


#### Add `export.sh` into system PATH

Append this line into `~/.zshrc` / `~/.bashrc` / ...

```
export idf_path=/home/fus/.fus/esp-idf 
export path=$path:$idf_path
```

#### Set-up env var before start project  

```
source export.sh
```

If you do not add `export.sh` to your path:

```
source /home/fus/.fus/esp-idf/export.sh
```

# PART-1: Drivers

This part focuses on writing drivers to control I/O. Additionally, this part will familiarize you with ESP-IDF development, including project structure, build process, and debugging.

## OLED (128x64) display 
### I2C Bit-banging
    
<img style="width:800px;" src=imgs/i2c-arch.png>            <br>
<img style="width:800px;" src=imgs/i2c-byte-ctl.png>        <br>
<img style="width:800px;" src=imgs/i2c-pixel-access.png>

### Implement I2C in an RTOS

    Failed, task too long --> Watchdog is triggered!

## Measure temperature and humidity

## Read ADC

## SDCard communication

# PART-2: Multitask system

This part focuses on building a real-time system. Reuse the driver written in the previous part to simultaneously display on the OLED, read values from sensors, and control LEDs.

## Tracking environment

## Controll LED using VR (variable resistance)

## Combination
