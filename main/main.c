#include "main.h"

/// @brief Install 
void IRAM_ATTR button_isr_handler(void* arg) {
    __entry("[ISR LOG] ");
    oled_state = (oled_state + 1)%3;
    uint8_t _task_code = 0x1;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(beq, &_task_code, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    __exit("[ISR LOG] ");
}

/// @brief set up button / led 
void gpio_init() {
    __entry("gpio_init()");
    gpio_config_t led_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED0) | (1ULL << LED1),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&led_conf);

    gpio_config_t btn_conf = {
        .intr_type      = GPIO_INTR_NEGEDGE,
        .mode           = GPIO_MODE_INPUT,
        .pin_bit_mask   = (1ULL << PULLUP_BUTTON),
        .pull_down_en   = GPIO_PULLDOWN_DISABLE,
        .pull_up_en     = GPIO_PULLUP_DISABLE
    };
    gpio_config(&btn_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PULLUP_BUTTON, button_isr_handler, (void*) PULLUP_BUTTON);
    beq = xQueueCreate(10, sizeof(uint8_t));
    if (beq == NULL) {
        __log("BUTTON_SETUP" "Failed to create button event queue.");
    }
    __exit("gpio_init()");
}

void screenctl(void *pvParameters) {
    __entry("screenctl(...)");
    uint8_t _isr_code = 0;
    while (1) {
        if (xQueueReceive(beq, &_isr_code, portMAX_DELAY) == pdPASS) {
            if (_isr_code == 0x1) {
                switch (oled_state){
                    case OLED_OFF:
                        __log("MODE: OLED_OFF");
                        oled_off();
                        gpio_set_level(LED0, 1);
                        break;
                    case OLED_ON:
                        __log("MODE: OLED_ON");
                        oled_on();
                        oled_flush();
                        gpio_set_level(LED0, 0);
                        break;
                    case OLED_TURNON_ALL_PIXEL:
                        __log("MODE: OLED_TURNON_ALL_PIXEL");
                        oled_turn_on_all_pixels();
                        gpio_set_level(LED0, 1);
                        break;
                }
            }
        }
    }
    vTaskDelete(NULL);
}

void screen_test(void *pvParameters) {
    __entry("screen_test(...)");
    // xy_t x = 40;
    // xy_t y = 20;
    // xy_t d = 5;
    // oled_draw_empty_rect(x, y, x+15, y+15, d, COLOR_WHITE);
    // x = 15, y = 90;
    // oled_draw_empty_rect(x, y, x+15, y+15, d, COLOR_WHITE);
    
    // for(uint16_t i = 0; i < 1024; ++i) {
    //     oled_canvas[i] = 0xFF;
    //     oled_flush();
    //     vTaskDelay(1);
    // }
    for( ; ; ){
        xy_t x = ((random()%64)+64)%64;
        xy_t y = ((random()%128)+128)%128;
        xy_t d = ((random()%3)+3)%3;
        oled_draw_empty_rect(x, y, x+15, y+15, d, (random()%2)?COLOR_BLACK:COLOR_WHITE);
        vTaskDelay(1);
        oled_flush();
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
    __exit("screen_test(...)");
}

void global_init(){
    __entry("global_init()");
    gpio_init();
    ESP_LOGI(TAG, "[oled_init]");
    oled_init(); 
    ESP_LOGI(TAG, "[oled_turn_on_all_pixels]");
    oled_turn_on_all_pixels();
    __exit("global_init()");
}

void app_main(void){
    __entry("app_main()");
    global_init();
    __log("[+task] screenctl");
    xTaskCreate(
        screenctl,        
        "ScreenTask",
        2048,
        NULL,
        7,
        NULL
    );
    __log("[+task] screen_test");
    xTaskCreate(
        screen_test,        
        "ScreenTest",
        2048, 
        NULL,
        2,
        NULL
    );
    __exit("app_main()");
}

