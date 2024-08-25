#ifndef ADC_LIB_H
#define ADC_LIB_H

// Include any necessary libraries here
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_log.h"
// Define any constants or macros here
// 
// ADC Configuration
#define ADC2_BITWIDTH ADC_BITWIDTH_DEFAULT
#define ADC2_ATTEN ADC_ATTEN_DB_0
#define ADC2_CHAN6 ADC_CHANNEL_6 // GPIO 14 is ADC2_CHANNEL_6
#define ADC2_CHAN4 ADC_CHANNEL_4 // GPIO 13 is ADC2_CHANNEL_4

// Declare function prototypes here

// Initialize ADC
void init_adc(int ADC_ATTEN, int ADC_BITWIDTH, int ADC_CHAN, adc_oneshot_unit_handle_t adc_handle);
// calibrate ADC
void calibrate_adc(int ADC_ATTEN, int ADC_BITWIDTH, int ADC_CHAN, adc_cali_handle_t handle);
#endif // ADC_LIB_H