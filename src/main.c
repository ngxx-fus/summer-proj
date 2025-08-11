// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "esp_log.h"
// #include "hw_setup.h"

// #define TAG "APP"

// // Flag to toggle yellow LED blinking
// volatile bool yellow_blink_enabled = true;

// void app_main(void);

// // ---- LED Task ----
// void led_blink_task(void *pvParameter) {
//     const gpio_num_t led_pin = (gpio_num_t)pvParameter;
//     const TickType_t delay = (led_pin == GREEN_LED) ? 2000 / portTICK_PERIOD_MS : 1000 / portTICK_PERIOD_MS;

//     while (1) {
//         if (led_pin == YELLOW_LED && !yellow_blink_enabled) {
//             gpio_set_level(led_pin, 0); // ensure it's off
//             vTaskDelay(delay);
//             continue;
//         }

//         gpio_set_level(led_pin, 1);
//         vTaskDelay(delay / 2);
//         gpio_set_level(led_pin, 0);
//         vTaskDelay(delay / 2);
//     }
// }

// // ---- Button Task ----
// void button_task(void *pvParameter) {
//     const gpio_num_t button_pin = PULLUP_BUTTON;

//     bool last_state = true;

//     while (1) {
//         bool state = gpio_get_level(button_pin);

//         // Detect falling edge (press)
//         if (last_state && !state) {
//             yellow_blink_enabled = !yellow_blink_enabled;
//             ESP_LOGI(TAG, "Yellow LED blinking: %s", yellow_blink_enabled ? "ON" : "OFF");
//             vTaskDelay(300 / portTICK_PERIOD_MS); // debounce delay
//         }

//         last_state = state;
//         vTaskDelay(50 / portTICK_PERIOD_MS);
//     }
// }

// // ---- Init GPIOs ----
// void init_gpio() {
//     gpio_config_t io_conf = {};
//     io_conf.intr_type = GPIO_INTR_DISABLE;
//     io_conf.mode = GPIO_MODE_OUTPUT;
//     io_conf.pin_bit_mask = (1ULL << YELLOW_LED) | (1ULL << GREEN_LED);
//     io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
//     io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
//     gpio_config(&io_conf);

//     gpio_config_t btn_conf = {};
//     btn_conf.intr_type = GPIO_INTR_DISABLE;
//     btn_conf.mode = GPIO_MODE_INPUT;
//     btn_conf.pin_bit_mask = (1ULL << PULLUP_BUTTON);
//     btn_conf.pull_up_en = GPIO_PULLUP_ENABLE;
//     btn_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
//     gpio_config(&btn_conf);
// }

// // ---- Main Entry ----
// void app_main(void) {
//     init_gpio();

//     xTaskCreate(led_blink_task, "Yellow_LED_Task", 2048, (void *)YELLOW_LED, 1, NULL);
//     xTaskCreate(led_blink_task, "Green_LED_Task", 2048, (void *)GREEN_LED, 1, NULL);
//     xTaskCreate(button_task,    "Button_Task",    2048, NULL,               1, NULL);
// }


#include <stdio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"

void app_main(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    uint32_t flash_size;
    esp_flash_get_size(NULL, &flash_size);

    printf("ESP32 Chip model = %s\n",
           chip_info.model == CHIP_ESP32 ? "ESP32" :
           chip_info.model == CHIP_ESP32S2 ? "ESP32-S2" :
           chip_info.model == CHIP_ESP32S3 ? "ESP32-S3" :
           chip_info.model == CHIP_ESP32C3 ? "ESP32-C3" : "Unknown");

    printf("Cores: %d\n", chip_info.cores);
    printf("Revision: %d\n", chip_info.revision);
    printf("Flash size: %luMB\n", flash_size / (1024 * 1024));
}
