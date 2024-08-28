
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h" // Add this line to include the header file
#include "driver/ledc.h" // Add this line to include the header file
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"

// Include the header file
#include "rgb.h"

// Global Variables
#define HEARTBEAT_GPIO 2 // GPIO2 is the built-in LED on the ESP32
#define QUEUE_SIZE 10 // Size queue for tasks
//LOGS TAGS
const static char *TAG = "ADC";
const static char *TAG2 = "ERROR";

// ADC Variables
static int adc_value;
//static int adc_value2;
static int adc_voltage;
//static int adc_voltage2;

int ADC2_ATTEN = ADC_ATTEN_DB_0;
int ADC2_BITWIDTH = ADC_BITWIDTH_11;
int ADC2_CHAN6 = ADC_CHANNEL_6;
int ADC2_CHAN4 = ADC_CHANNEL_4;

// NTC Variables
int B_constant = 3200; // B constant for NTC in Kelvin
int R0 = 47; // Resistance at T0 in Ohm
int T0 = 25; // Temperature at T0 in Celsius

#define NTC_TABLE_SIZE (sizeof(temp_voltage_table) / sizeof(temp_voltage_table[0]))

/*BEGIN FUNTIONS AND TASK DECLARATIONS*/

// Function declarations
// Task functions
void heartbeat_task(void *pvParameter);
void blink_rgb_task(void *pvParameter);
void voltage_to_temp_task(void *pvParameter);

/*END FUNTIONS AND TASK DECLARATIONS*/

/*BEGIN MAIN CODE*/
void app_main(void) 
{
    
    // Set log level to INFO
    esp_log_level_set("*", ESP_LOG_INFO);
  

    // Configure GPIO for heartbeat
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << HEARTBEAT_GPIO);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);

    // initialize the led rgb
    init_rgb(PIN_RED, PIN_GREEN, PIN_BLUE);
    
    // INITIAL ADC
    // ADC handle
    adc_oneshot_unit_handle_t adc2_handle;
    //adc handle for adc2 calibration
    adc_cali_handle_t adc2_handle_cali_ch6 = NULL;
    adc_cali_handle_t adc2_handle_cali_ch4 = NULL;

    // Initialize ADC whitout adc_lib

    //-------------ADC2 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
    };
    adc_oneshot_new_unit(&init_config2, &adc2_handle);

    //-------------ADC2 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC2_ATTEN,
        .bitwidth = ADC2_BITWIDTH,
    };
    adc_oneshot_config_channel(adc2_handle, ADC2_CHAN6, &config); // Potenciometer input
    adc_oneshot_config_channel(adc2_handle, ADC2_CHAN4, &config); // NTC input
    // calibrate the adc whitout adc_lib
    //-------------ADC2 Calibrate---------------//
    // Calibrate ADC line fitting
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_2, // Replace 'unit' with 'ADC_UNIT_2'
        .atten = ADC2_ATTEN,
        .bitwidth = ADC2_BITWIDTH,
    };
    adc_cali_create_scheme_line_fitting(&cali_config, &adc2_handle_cali_ch6); // Declare 'ret' variable
    adc_cali_create_scheme_line_fitting(&cali_config, &adc2_handle_cali_ch4); // Declare 'ret' variable


    while(1)  
    {
        // Create task heartbeat
        xTaskCreate(&heartbeat_task, "heartbeat_task", 2048, NULL, 1, NULL);

        // Read ADC values and display voltage
        adc_oneshot_read(adc2_handle, ADC_CHANNEL_6, &adc_value);
        ESP_LOGI(TAG, "ADC Value: %d", adc_value);
        vTaskDelay(pdMS_TO_TICKS(100));
        // read the voltage
        adc_cali_raw_to_voltage(adc2_handle_cali_ch6, adc_value, &adc_voltage);
        ESP_LOGI(TAG, "ADC Voltage: %dmV", adc_voltage);
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // Read ADC values and display voltage for NTC
        adc_oneshot_read(adc2_handle, ADC_CHANNEL_4, &adc_value);
        ESP_LOGI(TAG, "ADC Value NTC: %d", adc_value);
        vTaskDelay(pdMS_TO_TICKS(100));
        // read the voltage for NTC
        adc_cali_raw_to_voltage(adc2_handle_cali_ch4, adc_value, &adc_voltage);
        ESP_LOGI(TAG, "ADC Voltage NTC: %dmV", adc_voltage);

        // change the color of the RGB LED based on the ADC value
        fade_rgb_color(adc_value);


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

// voltage to temperature task function
void voltage_to_temp_task(void *pvParameter)
{
    float res_serie = 100 ;// resistance connected in series with the NTC
    float Vcc = 3.3 ;// voltage of the power supply
    float T; // declare the variable 'T'
    while(1) {
        // calculate the temperature from the voltage
        // calculate the resistance of the NTC
        float R = (adc_voltage * res_serie) / (Vcc - adc_voltage);
        // calculate the temperature
        T = B_constant / (log(R / R0) + (B_constant / (T0 + 273.15))) - 273.15; // convert to Celsius
    }
    return T;
}


/*END TASKS DEFINITIONS*/