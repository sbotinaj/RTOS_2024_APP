
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"  // Add this line to include the header file
#include "driver/ledc.h" // Add this line to include the header file

// ADC one-shot library
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include <math.h>        // Add this line to include the header file
#include "driver/uart.h" // Add this line to include the header file

// Include the header file
#include "rgb.h"

/*BEGIN GLOBAL VARIABLES*/

// variables for Queue

QueueHandle_t adcQueue;  // Queue for the ADC values
QueueHandle_t rgbQueue;  // Queue for the RGB values
QueueHandle_t uartQueue; // Queue for the UART values

// Queue for commands

QueueHandle_t R0Queue; // Queue for the R0 values
QueueHandle_t T0Queue; // Queue for the T0 values
QueueHandle_t BQueue;  // Queue for the B values

QueueHandle_t rangeTempMinQueue; // Queue for the range temperature min values
QueueHandle_t rangeTempMaxQueue; // Queue for the range temperature max values

// Queue for the GPIO interrupt

QueueHandle_t gpio_evt_queue = NULL;
QueueHandle_t gpio_flag_queue;

// off leds queue

QueueHandle_t off_leds_queue; // Queue for the off leds values


// time of sending the data to the Queue and receiving the data from the Queue

const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;

// GPIO interrupt pin

#define GPIO_INTERR GPIO_NUM_33

// ADC configuration

#define ADC_NUM ADC_UNIT_2        // ADC2
#define ADC_CHANNEL 4             // GPIO 32
#define ADC_WIDTH 12              // 12 bits
#define ADC_ATTEN ADC_ATTEN_DB_12 // attenuation of 11 dB

// variables for UART init
#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_3)
#define UART_NUM UART_NUM_0

// variables for UART configuration
#define BAUD_RATE 115200
#define DATA_BITS UART_DATA_8_BITS
#define STOP_BITS 1
#define PARITY UART_PARITY_DISABLE

// variables for UART buffer

#define BUF_SIZE (1024)

// ranges for the temperature


// variables for NTC

/*END GLOBAL VARIABLES*/

/*BEGIN FUNCTION DEFINITIONS*/

// init oneshot ADC2 chnnel 4

/*END FUNCTION DEFINITIONS*/

/*BEGIN TASKS DEFINITIONS*/

static void heartbeat_task(void *pvParameters);

static void uart_task(void *pvParameters);

static void temp_read_task(void *pvParameters);

static void rgb_task(void *pvParameters);

static void button_task(void *pvParameters);

/*END TASKS DEFINITIONS*/

/*BEGIN MAIN CODE*/
void app_main(void)
{
    
    
    // create the Queue
    adcQueue = xQueueCreate(10, sizeof(int32_t));
    rgbQueue = xQueueCreate(10, sizeof(float_t));
    uartQueue = xQueueCreate(10, sizeof(int32_t));

    // create the Queue for the commands
     R0Queue = xQueueCreate(2, sizeof(int32_t));
     T0Queue = xQueueCreate(2, sizeof(int32_t));
     BQueue = xQueueCreate(2, sizeof(int32_t));

    rangeTempMinQueue = xQueueCreate(2, sizeof(int32_t));
    rangeTempMaxQueue = xQueueCreate(2, sizeof(int32_t));

    // create Queue for the GPIO interrupt
    gpio_evt_queue = xQueueCreate(2, sizeof(uint32_t));
    gpio_flag_queue = xQueueCreate(2, sizeof(uint32_t));

    // create the Queue for the off leds
    off_leds_queue = xQueueCreate(2, sizeof(uint32_t));

    // // initialize the RGB LED
    // create the heartbeat task
    xTaskCreate(heartbeat_task, "heartbeat_task", 1024 * 2, NULL, 1, NULL);
    // create the UART task
    xTaskCreate(uart_task, "uart_task", 1024 * 4, NULL, 10, NULL);
    //  create the temperature read task
    xTaskCreatePinnedToCore(temp_read_task, "temp_read_task", 1024 * 4, NULL, 3, NULL, 1);
    // create the RGB task
    xTaskCreate(rgb_task, "rgb_task", 1024 * 4, NULL, 2, NULL);
    // create the button task
    //xTaskCreate(button_task, "button_task", 1024 * 2, NULL, 11, NULL);
    xTaskCreatePinnedToCore(button_task, "button_task", 1024 * 2, NULL, 11, NULL, 1);
    // xTaskCreatePinnedToCore(rgb_task, "rgb_task", 1024 * 4, NULL, 4, NULL, 0);
    // delay for 0.1 seconds
    vTaskDelay(100 / portTICK_PERIOD_MS);
}
/*END MAIN CODE*/

/*BEGIN FUNCTION PROTOTYPES*/

// function adc_init

/*END FUNCTION PROTOTYPES*/

/*BEGIN TASKS PROTOTYPES*/
static void uart_task(void *pvParameters)
{
    // configure the UART
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = DATA_BITS,
        .parity = PARITY,
        .stop_bits = STOP_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT};
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

    while (1)
    {
        // read data from the UART
        int len = uart_read_bytes(UART_NUM, data, (BUF_SIZE - 1), 100 / portTICK_PERIOD_MS);
        if (len > 0)
        {
            // print the data to the console
            printf("Received data: %s\n", data);

            // COMMANDS
            // if the command is "R0=num;"
            if (strncmp((char *)data, "R0=", 3) == 0)
            {
                // get the number from the command
                int R0 = strtol((char *)&data[3], NULL, 10);
                xQueueSend(R0Queue, &R0, 100 / portTICK_PERIOD_MS);
                printf("changed R0 to: %d\n", R0);
            }
            // if the command is "T0=num;"
            else if (strncmp((char *)data, "T0=", 3) == 0)
            {
                // get the number from the command
                int T0 = strtof((char *)&data[3], NULL);
                xQueueSend(T0Queue, &T0, 100 / portTICK_PERIOD_MS);
                printf("changed T0 to: %d\n", T0);
            }
            // if the command is "B=num;"
            else if (strncmp((char *)data, "B=", 2) == 0)
            {
                // get the number from the command
                int B = strtof((char *)&data[2], NULL);
                xQueueSend(BQueue, &B, 100 / portTICK_PERIOD_MS);
                printf("changed B to: %d\n", B);
            }
            // if the command is "RANGE_TEMP_MIN=num;"
            else if (strncmp((char *)data, "RANGE_TEMP_MIN=", 15) == 0)
            {
                // get the number from the command
                int RANGE_TEMP_MIN = strtol((char *)&data[15], NULL, 10);
                xQueueSend(rangeTempMinQueue, &RANGE_TEMP_MIN, 100 / portTICK_PERIOD_MS);
                printf("changed RANGE_TEMP_MIN to: %d\n", RANGE_TEMP_MIN);
            }
            // if the command is "RANGE_TEMP_MAX=num;"
            else if (strncmp((char *)data, "RANGE_TEMP_MAX=", 15) == 0)
            {
                // get the number from the command
                int RANGE_TEMP_MAX = strtol((char *)&data[15], NULL, 10);
                xQueueSend(rangeTempMaxQueue, &RANGE_TEMP_MAX, 100 / portTICK_PERIOD_MS);
                printf("changed RANGE_TEMP_MAX to: %d\n", RANGE_TEMP_MAX);
            }
            else
            {
                // print an error message
                printf("Invalid command\n");
            }
        }
    }
}
// temperature read task
static void temp_read_task(void *pvParameters)
{
    int time_to_wait = 1000/portTICK_PERIOD_MS;
    int gpio_flag = 0;
    
    int32_t R_serie = 100;
    int32_t R0;
    int32_t T0;
    int32_t B;
    // ADC INIT
    //  variables for the ADC value
    int adc_reading = 0;
    float temp = 0;
    float voltage_v = 0;

    adc_cali_handle_t handle_calibrated;
    //-------------ADC2 Init---------------//
    adc_oneshot_unit_handle_t adc2_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_NUM,
    };
    adc_oneshot_new_unit(&init_config, &adc2_handle);

    //-------------ADC2 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL, &config);

    //-------------ADC2 Calibration---------------//
    // calibration with line fitting
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_NUM,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_WIDTH,
    };
    adc_cali_create_scheme_line_fitting(&cali_config, &handle_calibrated);
    R0 = 47;
    T0 = 25;
    B = 2600;

    // receive the NTC values from the Queue
    
    // variables for calculating the temperature
    float Rt = 0;

    // verifica si se envio el valor R0 por la cola, de lo contrario se usa el valor por defecto
    while (1)
    {
        if(xQueueReceive(R0Queue, &R0, 0) == pdTRUE){
            printf("R0: %d\n", (int)R0);
        }

        xQueueReceive(T0Queue, &T0, 0);

        xQueueReceive(BQueue, &B, 0);
        // read the ADC value
        adc_oneshot_read(adc2_handle, ADC_CHANNEL, &adc_reading);
        // convert the ADC value to voltage
        //adc_cali_raw_to_voltage(handle_calibrated, adc_reading, &voltage);
        //conver adc to voltage 
        voltage_v = adc_reading * 3.3 /pow(2, ADC_WIDTH);
        // convert the mv to v 
        //voltage_v = voltage / 1000;
        // convert the voltage to temperature
        Rt = (R_serie * voltage_v) / (3.3 - voltage_v);
        temp = B/(log(Rt/R0)+B/T0);


        // send the temperature to the Queue
        xQueueSend(adcQueue, &temp, xTicksToWait);
        // if receive gpio flag queue
        xQueueReceive(gpio_flag_queue, &gpio_flag, 0);
        if(gpio_flag != 0){
            time_to_wait = 100/portTICK_PERIOD_MS;
        }
        else{
            printf("ADC: %d, Voltage: %.3f mV, Temperature: %.2f C\n", adc_reading, voltage_v, temp);
            vTaskDelay(time_to_wait);
        }

        // delay for 0.1 second
        vTaskDelay(xTicksToWait);

    }
}
// RGB task
static void rgb_task(void *pvParameters)
{
    init_rgb(PIN_RED, PIN_GREEN, PIN_BLUE);
    set_rgb_color(255, 255, 255);

    // adc value
    float temp;
    // variables for the RGB values
    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;

    int RANGE_TEMP_MAX = 25;
    int RANGE_TEMP_MIN = 15;

    int flag_off = 0;

    while (1)
    {
        xQueueReceive(adcQueue, &temp, xTicksToWait); // temp from the Queue
        xQueueReceive(rangeTempMinQueue, &RANGE_TEMP_MIN, xTicksToWait); 
        xQueueReceive(rangeTempMaxQueue, &RANGE_TEMP_MAX, xTicksToWait);
        xQueueReceive(off_leds_queue, &flag_off, 0);

        if (flag_off == 2)
        {
            set_rgb_color(0, 0, 0);
            fprintf(stderr, "LEDs off\n");
        }
        else
        {
            // change the RGB color based on the temperature
            if (temp < RANGE_TEMP_MIN)
            {
                red = 0;
                green = 255;
                blue = 255;
                set_rgb_color(red, green, blue);

            }
            else if (temp > RANGE_TEMP_MAX)
            {
                red = 255;
                green = 255;
                blue = 0;
                set_rgb_color(red, green, blue);
            }
            else
            {
                red = 255;
                green = 0;
                blue = 255;
                set_rgb_color(red, green, blue);
            }
            // delay for 0.1 second
            vTaskDelay(xTicksToWait);
        }
        
    }
}
// heartbeat task
static void heartbeat_task(void *pvParameters)
{
    // configure the GPIO pin 2
    gpio_reset_pin(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    // loop forever
    while (1)
    {
        // toggle the GPIO pin 2
        gpio_set_level(GPIO_NUM_2, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_NUM_2, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
// button task
static void gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void button_task(void *pvParameters)
{
    uint32_t io_num;
    int count_press = 0; // variable for counting the number of button presses
    //gpio_evt_queue = xQueueCreate(2, sizeof(uint32_t));
    esp_rom_gpio_pad_select_gpio(GPIO_INTERR);
    gpio_set_direction(GPIO_INTERR, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_INTERR, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(GPIO_INTERR, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_INTERR, gpio_isr_handler, (void *)GPIO_INTERR);
    while (1)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            count_press++;
            if (count_press ==2)
            {
                io_num = 2;
                count_press = 0;
                xQueueSend(off_leds_queue, &io_num, 0);
                printf("Button pressed\n");
            }
            else
            {
                xQueueSend(gpio_flag_queue, &io_num, 0);
                printf("Button pressed\n"); 
            }
        }
        else
        {
            printf("Queue error\n");
        }
        vTaskDelay(xTicksToWait);
    }
}
/*END TASKS PROTOTYPES*/