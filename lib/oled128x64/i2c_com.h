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

#include <stdarg.h>
#include <stdint.h>
#include "hal/gpio_ll.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

#if defined(I2CCOM_LOG)

    #ifdef __log
        #define i2ccom_log __log
    #else
        #include "rom/ets_sys.h"
        #include "esp_timer.h"
        #include <stdarg.h>

        #define i2ccom_log(fmt, ...) \
                ets_printf("[%lld] [I2CCOM] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)

    #endif

    #ifndef I2CCOM_LOG_LEVEL
        // #define I2CCOM_LOG_LEVEL 
        // #define I2CCOM_LOG_LEVEL 0x01 // LOG send byte array functions, startup
        // #define I2CCOM_LOG_LEVEL 0x02 // LOG send word/byte functions
        // #define I2CCOM_LOG_LEVEL 0x03 // LOG send adress/data (a part of frame)
        // #define I2CCOM_LOG_LEVEL 0x04 // LOG start/stop  condition
        // #define I2CCOM_LOG_LEVEL 0x05 // LOG mono pulse 
    #endif

    #ifndef I2CCOM_LOG_DATAFRAME
        // #define I2CCOM_LOG_DATAFRAME
    #endif
        
#endif

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
// #define set_scl(level) (level)?(GPIO.out_w1ts = (1 << (__scl))):(GPIO.out_w1tc = (1 << (__scl)));
static inline void set_scl(int level) {
    if (level) {
        GPIO.out_w1ts = (1 << __scl);
    } else {
        GPIO.out_w1tc = (1 << __scl);
    }
}



#endif 

#ifndef set_sda
// #define set_sda(level) (level)?(GPIO.out_w1ts = (1 << (__sda))):(GPIO.out_w1tc = (1 << (__sda)));
static inline void set_sda(int level) {
    if (level) {
        GPIO.out_w1ts = (1 << __sda);
    } else {
        GPIO.out_w1tc = (1 << __sda);
    }
}
#endif

#ifndef i2c_com_delay
// #define i2c_com_delay(__stick)
// #define i2c_com_delay(__stick) vTaskDelay(__stick);
#define i2c_com_delay(__us) esp_rom_delay_us(__us)
#endif 

#ifndef get_sda
#define get_sda() ((GPIO.in >> __sda) & 0x1)
// #define get_sda() gpio_get_level(__sda)
#endif

#ifndef get_scl
#define get_scl() ((GPIO.in >> __scl) & 0x1)
// #define get_scl() gpio_get_level(__scl)
#endif

#ifndef logic_cast
#define logic_cast(value) ((value)>0?1:0)
#endif

#ifndef wait_for_scl
#define wait_for_scl 
// #define wait_for_scl for(uint32_t t_wait = 0; t_wait < 1000 && get_scl() == 0; ++t_wait);
// #define wait_for_scl for(uint32_t t_wait = 0; t_wait < 1000 && get_scl() == 0; ++t_wait) i2c_com_delay(1);
#endif 

#ifndef i2c_spinlock
#define i2c_spinlock i2c_spinlock_0
#endif


void         i2c_config_headers();
static inline void  i2c_start_condition();
static inline void  i2c_stop_condition();
static inline void  scl_mono_pulse();
static inline void  i2c_send_address_frame(uint8_t address_7bit, uint8_t mode);
static inline void  i2c_send_data_frame(uint8_t address_7bit);
static inline uint8_t i2c_get_response();
uint8_t      i2c_read_byte(uint8_t address_7bit);
uint8_t      i2c_read_byte_array(uint8_t address_7bit, uint8_t amount);
uint8_t      i2c_send_word(uint8_t address_7bit, uint8_t byte_h, uint8_t byte_l);
uint8_t      i2c_send_byte(uint8_t address_7bit, uint8_t data);
uint8_t      i2c_send_byte_array(uint8_t address_7bit, uint8_t data[], uint32_t array_size);
uint8_t      i2c_send_byte_array_w_ctl_byte(uint8_t address_7bit, uint8_t ctl_byte, uint8_t data[], uint32_t array_size);

portMUX_TYPE i2c_spinlock = portMUX_INITIALIZER_UNLOCKED;

void i2c_config_headers(){
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0)
        i2ccom_log("[>>>] i2c_config_headers()");
    #endif

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
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0)
        i2ccom_log("[<<<] i2c_config_headers()");
    #endif
}

static inline void i2c_start_condition(){
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x3)
        i2ccom_log("[>>>] i2c_start_condition()");
    #endif
    set_sda(1); set_scl(1);
    i2c_com_delay(__t1);
    set_sda(0);
    i2c_com_delay(__t1);
    set_scl(0);
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x3)
        i2ccom_log("[<<<] i2c_start_condition()");
    #endif
}

static inline void i2c_stop_condition(){
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x3)
        i2ccom_log("[>>>] i2c_stop_condition()");
    #endif
    set_sda(0); set_scl(0);
    i2c_com_delay(__t1);
    set_scl(1);
    i2c_com_delay(__t1);
    set_sda(1);
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x3)
        i2ccom_log("[<<<] i2c_stop_condition()");
    #endif
}

static inline void scl_mono_pulse(){
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x4)
        i2ccom_log("[>>>] scl_mono_pulse()");
    #endif
    set_scl(0);
    i2c_com_delay(__t1);
    set_scl(1);
    i2c_com_delay(__t2);
    wait_for_scl;
    set_scl(0);
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x4)
        i2ccom_log("[<<<] scl_mono_pulse()");
    #endif
}

void i2c_send_address_frame(uint8_t address_7bit, uint8_t mode){
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x2)
        i2ccom_log("[>>>] i2c_send_address_frame()");
    #endif
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
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x2)
        i2ccom_log("[<<<] i2c_send_address_frame()");
    #endif
}

static inline void i2c_send_data_frame(uint8_t data){
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x2)
        i2ccom_log("[>>>] i2c_send_data_frame()");
    #endif
    #if defined(I2CCOM_LOG) && defined(I2CCOM_LOG_DATAFRAME)
        i2ccom_log("[-->] <0x%02X>", data);
    #endif
    /// 8 pulses of scl 
    uint8_t i = 0x80;
    while(i > 0){
        set_sda(logic_cast(data & i));
        scl_mono_pulse();
        i>>=1;
    };
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x2)
        i2ccom_log("[<<<] i2c_send_data_frame()");
    #endif
}

static inline uint8_t i2c_get_response(){
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x5)
        i2ccom_log("[>>>] i2c_get_response()");
    #endif
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
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x5)
        i2ccom_log("[<<<] i2c_get_response()");
    #endif
    return res;
}

uint8_t i2c_send_byte(uint8_t address_7bit, uint8_t data){
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x1)
        i2ccom_log("[>>>] i2c_send_byte()");
    #endif
    taskENTER_CRITICAL(&i2c_spinlock);
    uint8_t res;
    i2c_start_condition();
    i2c_send_address_frame(address_7bit, 0);
    if( i2c_get_response() ){
        i2c_stop_condition();
        taskEXIT_CRITICAL(&i2c_spinlock);
        return -1;
    }
    i2c_send_data_frame(data);
    res = i2c_get_response();
    i2c_stop_condition();
    taskEXIT_CRITICAL(&i2c_spinlock);
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x1)
        i2ccom_log("[<<<] i2c_send_byte()");
    #endif
    return res;
}

uint8_t i2c_send_word(uint8_t address_7bit, uint8_t byte_h, uint8_t byte_l){
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x1)
        i2ccom_log("[>>>] i2c_send_word()");
    #endif
    taskENTER_CRITICAL(&i2c_spinlock);
    uint8_t res;
    i2c_start_condition();
    i2c_send_address_frame(address_7bit, 0);
    res = i2c_get_response();
    if( res ){
        i2c_stop_condition();
        taskEXIT_CRITICAL(&i2c_spinlock);
        return -1;
    }
    i2c_com_delay(__t1);
    i2c_send_data_frame(byte_h);
    res = i2c_get_response();
    if( res ){
        i2c_stop_condition();
        taskEXIT_CRITICAL(&i2c_spinlock);
        return 0;
    }
    i2c_com_delay(__t1);
    i2c_send_data_frame(byte_l);
    res = i2c_get_response();
    if( res ){
        i2c_stop_condition();
        taskEXIT_CRITICAL(&i2c_spinlock);
        return 1;
    }
    i2c_stop_condition();
    taskEXIT_CRITICAL(&i2c_spinlock);
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x1)
        i2ccom_log("[<<<] i2c_send_word()");
    #endif
    return res;
}

uint8_t i2c_send_byte_array(uint8_t address_7bit, uint8_t data[], uint32_t array_size){
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x0)
        i2ccom_log("[>>>] i2c_send_byte_array()");
    #endif
    taskENTER_CRITICAL(&i2c_spinlock);
    uint8_t res;
    i2c_start_condition();
    i2c_send_address_frame(address_7bit, 0);
    res = i2c_get_response();
    if( res ){
        i2c_stop_condition();
        taskEXIT_CRITICAL(&i2c_spinlock);
        return -1;
    }
    for(uint8_t i = 0; i < array_size; ++i){
        i2c_com_delay(__t1);
        i2c_send_data_frame(data[i]);
        res = i2c_get_response();
        if( res ){
            i2c_stop_condition();
            taskEXIT_CRITICAL(&i2c_spinlock);
            return i;
        }
    }
    i2c_stop_condition();
    taskEXIT_CRITICAL(&i2c_spinlock);
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x0)
        i2ccom_log("[<<<] i2c_send_byte_array()");
    #endif
    return res;
}

uint8_t i2c_send_byte_array_w_ctl_byte(uint8_t address_7bit, uint8_t ctl_byte, uint8_t data[], uint32_t array_size){
    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x0)
        i2ccom_log("[>>>] i2c_send_byte_array_w_ctl_byte()");
    #endif
    
    taskENTER_CRITICAL(&i2c_spinlock);
    uint8_t res;
    i2c_start_condition();
    i2c_send_address_frame(address_7bit, 0);
    res = i2c_get_response();
    if( res ){
        i2c_stop_condition();
        taskEXIT_CRITICAL(&i2c_spinlock);
        return -1;
    }
    i2c_com_delay(__t1);
    i2c_send_data_frame(ctl_byte);
    res = i2c_get_response();
    if( res ){
        i2c_stop_condition();
        taskEXIT_CRITICAL(&i2c_spinlock);
        return -1;
    }
    for(uint32_t i = 0; i < array_size; ++i){
        i2c_com_delay(__t1);
        i2c_send_data_frame(data[i]);
        res = i2c_get_response();
        if( res ){
            i2c_stop_condition();
            taskEXIT_CRITICAL(&i2c_spinlock);
            return i;
        }
    }
    i2c_stop_condition();
    taskEXIT_CRITICAL(&i2c_spinlock);

    #if defined(I2CCOM_LOG) && (I2CCOM_LOG_LEVEL > 0x0)
        i2ccom_log("[<<<] i2c_send_byte_array_w_ctl_byte()");
    #endif
    return res;
}



#endif