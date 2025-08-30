/// HEADERS & MACROS //////////////////////////////////////

#include <stdint.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "freertos/semphr.h"
#include "rom/ets_sys.h"
#include "esp_timer.h"

#define TAG             "OLED-APP"
#define I2CCOM_LOG 

#include "hw_setup.h"

#define __scl OLED_SCL
#define __sda OLED_SDA
// #define I2CCOM_LOG
// #define I2CCOM_LOG_DATAFRAME
// #define I2CCOM_LOG_LEVEL    2

#include "oled128x64.h"

#define __led_blink(LED, D) gpio_set_level(LED, 0); esp_rom_delay_us(1000*D); \
                             gpio_set_level(LED, 1); esp_rom_delay_us(1000*D); \
                             gpio_set_level(LED, 0); esp_rom_delay_us(1000*D); \
                             gpio_set_level(LED, 1); esp_rom_delay_us(1000*D); \
                             gpio_set_level(LED, 0);

#ifndef __entry
    #define __entry(fmt, ...)   ets_printf("[%lld] [>>>] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#endif
#ifndef __exit
    #define __exit(fmt, ...)    ets_printf("[%lld] [<<<] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#endif
#ifndef __log
    #define __log(fmt, ...)     ets_printf("[%lld] [;-;] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#endif

/// GLOBAL VARIABLES //////////////////////////////////////

/// OLED state
volatile uint8_t oled_state = 0x2;
/// State list
enum OLED_STATE_T {
    OLED_OFF              = 0x0,
    OLED_ON               = 0x1,
    OLED_TURNON_ALL_PIXEL = 0x2
};

/// BEQ - button event queue
QueueHandle_t beq = NULL;

int __random() {
    static int core = 47;  // seed ban đầu
    core = ( (core * 53719) ^ (core << 5) )
         + ( (core >> 3) & 0x7FFF );
    core = core - (core / 19);
    core = core ^ (core << 7);

    // kết quả trong khoảng 0..9999
    return core % 10000;
}