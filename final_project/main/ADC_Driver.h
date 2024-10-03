#include "main.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"



/*Parametros necesarios para leer temperatura con una ntc*/
#define ADC_RESOLUTION 4095 //Se utilizan los 12 bits del ADC
#define VCC 3.27  // Voltaje de alimentación
#define T_REF 298.15 //25°c en °K
#define NOMINAL_REFERENCE_NTC_defaul 47.0 //valor de la resistencia a 25 grados
#define REFERENCE_RESISTOR_defaul 210 //valor de la resistencia del divisor de voltaje
#define B_defaul 3950 //valor de la constante Beta


/*Canales a utilizar*/
#define TEMPERATURE_CHANNEL ADC_CHANNEL_4
#define HUMIDITY_CHANNEL ADC_CHANNEL_6



void ADC1_init();
void ADC1_set(adc_channel_t channel);
void ADC1_ReadTemperature_task(void *pvParameter);
void ADC1_ReadHumidity_task(void *pvParameter);

/*Conversion del valor raw del adc*/
float adc_to_ntc_temperature(int adc_reading);
float adc_to_FC28_humidity(int adc_reading);