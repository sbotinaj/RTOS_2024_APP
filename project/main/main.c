
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h" // Add this line to include the header file
#include "rgb.h" // Add this line to include the header file

// Global Variables
#define HEARTBEAT_GPIO 2

// Function declaration
void heartbeat_task(void *pvParameter);
void blink_rgb_task(void *pvParameter);

/*BEGIN MAIN CODE*/
void app_main(void) 
{
    // Configure GPIO
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << HEARTBEAT_GPIO);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);

    // initialize the led rgb
    init_rgb(PIN_RED, PIN_GREEN, PIN_BLUE);

    while(1)  
    {
        // Create the heartbeat task
        xTaskCreate(&heartbeat_task, "heartbeat_task", 2048, NULL, 5, NULL);
        // Create the blink rgb task
        xTaskCreate(&blink_rgb_task, "blink_rgb_task", 2048, NULL, 5, NULL);

    }
}
/*END MAIN CODE*/

/*BEGIN TASKS DEFINITIONS*/

// heartbeat task function
void heartbeat_task(void *pvParameter)
{
    while(1) {
        // turn the LED on
        gpio_set_level(HEARTBEAT_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // turn the LED off
        gpio_set_level(HEARTBEAT_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// blink rgb task function
void blink_rgb_task(void *pvParameter)
{
    while(1) {
        // blink the RGB LED
        set_rgb_color(1023, 0, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        set_rgb_color(0, 1023, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        set_rgb_color(0, 0, 1023);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/*END TASKS DEFINITIONS*/