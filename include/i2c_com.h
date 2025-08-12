#ifndef __I2C_COM_H__
#define __I2C_COM_H__

/*Params of delay:
Start condition:
            ______
                  |
                  |
        SDA       |____________
            ____________
                        |
                        |
        SCL             |______

              ... |t1   | ...               : Time

Stop condition:
                         ______
                        |
                        |
        SDA ____________|
                   ____________
                  |
                  |
        SCL ______|
        
              ... |t1   | ...               : Time

Normal send (write op):
                   _______________
                  |               |
                  |               |
        SDA ______|_ _ _ _ _ _ _ _|______
                       _______
                      |       |
                      |       |
        SCL __________|       |__________

            |??   |t1 |t2     |??  |??   |   : Time
----> Period: 2*(t0+t1)+t2


*/

#include <stdint.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "freertos/semphr.h"

#ifndef __sda
#define __sda 0
#endif

#ifndef __scl
#define __scl 0
#endif

#ifndef __t1
#define __t1        1
#endif
#ifndef __t2
#define __t2        2
#endif

#ifndef set_scl
#define set_scl(level) gpio_set_level(__scl, level)
#endif 

#ifndef set_sda
#define set_sda(level) gpio_set_level(__sda, level)
#endif

#ifndef i2c_com_delay
// #define i2c_com_delay(__stick)
// #define i2c_com_delay(__stick) vTaskDelay(__stick);
#define i2c_com_delay(__us) esp_rom_delay_us(__us)
#endif 

#ifndef get_sda
#define get_sda() gpio_get_level(__sda)
#endif

#ifndef get_scl
#define get_scl() gpio_get_level(__scl)
#endif

#ifndef logic_cast
#define logic_cast(value) ((value)>0?1:0)
#endif

#ifndef wait_for_scl
// #define wait_for_scl 
#define wait_for_scl for(uint32_t t_wait = 0; t_wait < 1000 && get_scl() == 0; ++t_wait) i2c_com_delay(1);
#endif 

void         config_headers();
void         start_condition();
void         stop_condition();
void         scl_mono_pulse();
void         send_address_frame(uint8_t address_7bit, uint8_t mode);
void         send_data_frame(uint8_t address_7bit);
uint8_t      get_response();
uint8_t      read_byte(uint8_t address_7bit);
uint8_t      read_byte_array(uint8_t address_7bit, uint8_t amount);
uint8_t      send_word(uint8_t address_7bit, uint8_t byte_h, uint8_t byte_l);
uint8_t      send_byte(uint8_t address_7bit, uint8_t data);
uint32_t     send_byte_array(uint8_t address_7bit, uint8_t data[], uint32_t array_size);

enum __I2C_STATUS_BITMASK{ 
    I2C_COM_SET_HEADER=0x1,
};

portMUX_TYPE i2c_spinlock = portMUX_INITIALIZER_UNLOCKED;

void config_headers(){
    gpio_config_t sda_header_config = {
        .intr_type      = GPIO_INTR_DISABLE,
        .mode           = GPIO_MODE_OUTPUT_OD,
        .pin_bit_mask   = (1ULL << __sda),
        .pull_down_en   = GPIO_PULLDOWN_DISABLE,
        .pull_up_en     = GPIO_PULLUP_ENABLE
    };
    gpio_config(&sda_header_config);

    gpio_config_t scl_header_config = {
        .intr_type      = GPIO_INTR_DISABLE,
        .mode           = GPIO_MODE_OUTPUT_OD,
        .pin_bit_mask   = (1ULL << __scl),
        .pull_down_en   = GPIO_PULLDOWN_DISABLE,
        .pull_up_en     = GPIO_PULLUP_ENABLE
    };
    gpio_config(&scl_header_config);
    set_scl(1);
    set_sda(1);
    i2c_com_delay(__t2);
}

void start_condition(){
    set_sda(1); set_scl(1);
    i2c_com_delay(__t1);
    set_sda(0);
    i2c_com_delay(__t1);
    set_scl(0);
}

void stop_condition(){
    set_sda(0); set_scl(0);
    i2c_com_delay(__t1);
    set_scl(1);
    i2c_com_delay(__t1);
    set_sda(1);
}

void scl_mono_pulse(){
    set_scl(0);
    i2c_com_delay(__t1);
    set_scl(1);
    i2c_com_delay(__t2);
    wait_for_scl;
    set_scl(0);
}

void send_address_frame(uint8_t address_7bit, uint8_t mode){
    /// 7 pulses of scl 
    uint8_t i = 0x40;
    while(i > 0){
        set_sda(logic_cast(address_7bit & i));
        scl_mono_pulse();
        i>>=1;
    };
    /// last pulse for R/W
    set_sda(logic_cast(mode));
    scl_mono_pulse(); 
}

void send_data_frame(uint8_t data){
    /// 8 pulses of scl 
    uint8_t i = 0x80;
    while(i > 0){
        set_sda(logic_cast(data & i));
        scl_mono_pulse();
        i>>=1;
    };
}

uint8_t get_response(){
    uint8_t res; // 0-ACK, 1-NCK
    i2c_com_delay(__t1);
    set_sda(1); /// release the SDA line
    i2c_com_delay(__t1);
    set_scl(1);
    i2c_com_delay(__t2/2);
    res = get_sda();
    i2c_com_delay((__t2/2) + (__t2%2));
    wait_for_scl;
    set_scl(0);
    return res;
}

uint8_t send_byte(uint8_t address_7bit, uint8_t data){
    taskENTER_CRITICAL(&i2c_spinlock);
    uint8_t res;
    start_condition();
    send_address_frame(address_7bit, 0);
    if( get_response() ){
        stop_condition();
        taskEXIT_CRITICAL(&i2c_spinlock);
        return -1;
    }
    send_data_frame(data);
    res = get_response();
    stop_condition();
    taskEXIT_CRITICAL(&i2c_spinlock);
    return res;
}

uint8_t send_word(uint8_t address_7bit, uint8_t byte_h, uint8_t byte_l){
    taskENTER_CRITICAL(&i2c_spinlock);
    uint8_t res;
    start_condition();
    send_address_frame(address_7bit, 0);
    res = get_response();
    if( res ){
        stop_condition();
        return -1;
    }
    i2c_com_delay(__t1);
    send_data_frame(byte_h);
    res = get_response();
    if( res ){
        stop_condition();
        taskEXIT_CRITICAL(&i2c_spinlock);
        return 0;
    }
    i2c_com_delay(__t1);
    send_data_frame(byte_l);
    res = get_response();
    if( res ){
        stop_condition();
        taskEXIT_CRITICAL(&i2c_spinlock);
        return 1;
    }
    stop_condition();
    taskEXIT_CRITICAL(&i2c_spinlock);
    return res;
}

uint32_t send_byte_array(uint8_t address_7bit, uint8_t data[], uint32_t array_size){
    taskENTER_CRITICAL(&i2c_spinlock);
    uint32_t res;
    start_condition();
    send_address_frame(address_7bit, 0);
    res = get_response();
    if( res ){
        stop_condition();
        return -1;
    }
    for(uint8_t i = 0; i < array_size; ++i){
        i2c_com_delay(__t1);
        send_data_frame(data[i]);
        res = get_response();
        if( res ){
            stop_condition();
            taskEXIT_CRITICAL(&i2c_spinlock);
            return i;
        }
    }
    stop_condition();
    taskEXIT_CRITICAL(&i2c_spinlock);
    return res;
}

#endif
