#include "adc_lib.h"


// Function definitions

// Initialize ADC
void init_adc(int ADC_ATTEN, int ADC_BITWIDTH, int ADC_CHAN, adc_oneshot_unit_handle_t adc_handle) {
    //-------------ADC2 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
    };
    adc_oneshot_new_unit(&init_config2, &adc_handle);

    //-------------ADC2 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH,
    };
    adc_oneshot_config_channel(adc_handle, ADC_CHAN, &config);
}

// calibrate ADC


void calibrate_adc(int ADC_ATTEN, int ADC_BITWIDTH, int ADC_CHAN, adc_cali_handle_t handle) {
    //-------------ADC2 Calibrate---------------//
    // Calibrate ADC line fitting

    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_2, // Replace 'unit' with 'ADC_UNIT_2'
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle); // Declare 'ret' variable
    if (ret == ESP_OK) {
        ESP_LOGI("ADC", "Calibration successful");
    } else {
        ESP_LOGE("ADC", "Calibration failed");
    }
}



