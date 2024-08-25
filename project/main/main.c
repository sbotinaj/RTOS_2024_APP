
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h" // Add this line to include the header file
#include "driver/ledc.h" // Add this line to include the header file
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"

// Include the header file
#include "rgb.h"
#include "adc_lib.h"

// Global Variables
#define HEARTBEAT_GPIO 2 // GPIO2 is the built-in LED on the ESP32

//log tag
const static char *TAG = "ADC";
// ADC handle
adc_oneshot_unit_handle_t adc2_handle;
static int adc_value;
static int adc_value2;
static int adc_voltage;
static int adc_voltage2;

/*BEGIN FUNTIONS AND TASK DECLARATIONS*/

// Function declarations
// Task functions
void heartbeat_task(void *pvParameter);
void blink_rgb_task(void *pvParameter);

/*END FUNTIONS AND TASK DECLARATIONS*/

/*BEGIN MAIN CODE*/
void app_main(void) 
{
    // Set log level to INFO
    esp_log_level_set("*", ESP_LOG_INFO);
    
    // Configure GPIO
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << HEARTBEAT_GPIO);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);

    // initialize the led rgb
    init_rgb(PIN_RED, PIN_GREEN, PIN_BLUE);
    // initialize the adc
    init_adc(ADC2_ATTEN, ADC2_BITWIDTH, ADC2_CHAN6, adc2_handle);
    init_adc(ADC2_ATTEN, ADC2_BITWIDTH, ADC2_CHAN4, adc2_handle);
    
    // Create task heartbeat
    xTaskCreate(&heartbeat_task, "heartbeat_task", 2048, NULL, 5, NULL);


    while(1)  
    {

        // Read ADC values and display voltage
        ESP_ERROR_CHECK(adc_oneshot_read(adc2_handle, ADC_CHANNEL_6, &adc_value));
        ESP_LOGI(TAG, "ADC Value: %d", adc_value);
        // read the voltage
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc2_handle, ADC_CHANNEL_6, &adc_voltage));
        ESP_LOGI(TAG, "ADC Voltage: %dmV", adc_voltage);


    }
}
/*END MAIN CODE*/

/*BEGIN FUNTIONS DEFINITIOS*/


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