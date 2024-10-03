#include "ADC_Driver.h"

static const char *ADC = "ADC_READ";

// Parametros para la incializacion y configuracion del ADC
adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc_cali_handle;

// Se declara la cola externa
extern QueueHandle_t adc_read_temperature_Queue;
extern QueueHandle_t adc_read_humidity_Queue;
extern QueueHandle_t temperature_LED_RGB_Control_Queue;
extern QueueHandle_t temperature_Buzzer_Control_Queue;
extern QueueHandle_t temperature_Fan_Control_Queue;



void ADC1_init()
{
    // Inicializacion del ADC
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
}
void ADC1_set(adc_channel_t channel)
{
    // Configuracion del canal
    adc_oneshot_chan_cfg_t adc_channel = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    // se configuran los parametros anteriores en el canal 4
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, channel, &adc_channel));

    // Se inicia la calibracion
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle));
}

void ADC1_ReadTemperature_task(void *pvParameter)
{

    // Valor en crudo del ADC y valor calibrado
    static int adc_raw = 0;
    static int voltage = 0;
    while (1)
    {
        // Leemos el raw Value
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, TEMPERATURE_CHANNEL, &adc_raw));

        // aplicamos calibracion al valor
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_raw, &voltage));

        float temperature = adc_to_ntc_temperature(adc_raw);

        // Se envia el valor a la cola
        xQueueSend(adc_read_temperature_Queue, &temperature, pdMS_TO_TICKS(100));
        xQueueSend(temperature_LED_RGB_Control_Queue, &temperature, pdMS_TO_TICKS(100));
        xQueueSend(temperature_Buzzer_Control_Queue, &temperature, pdMS_TO_TICKS(100));
        xQueueSend(temperature_Fan_Control_Queue, &temperature, pdMS_TO_TICKS(100));
        

        // Se imprime el valor
        //ESP_LOGI(ADC, "Temperature: %.2f", temperature);

        // Se espera 1 segundo
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void ADC1_ReadHumidity_task(void *pvParameter)
{

    // Valor en crudo del ADC y valor calibrado
    static int adc_raw = 0;
    static int voltage = 0;
    while (1)
    {
        // Leemos el raw Value
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, HUMIDITY_CHANNEL, &adc_raw));

        // aplicamos calibracion al valor
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, adc_raw, &voltage));

        float humidity = adc_to_FC28_humidity(adc_raw);

        // Se envia el valor a la cola
        xQueueSend(adc_read_humidity_Queue, &humidity, pdMS_TO_TICKS(100));

        // Se imprime el valor
        //ESP_LOGI(ADC, "Humidity: %.2f", humidity);

        // Se espera 1 segundo
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}



float adc_to_ntc_temperature(int adc_reading){
    // Se convierte la lectura del ADC a voltaje
    float voltage_reading = (float)adc_reading / ADC_RESOLUTION * VCC;

    // Se convierte el voltaje a resistencia usando la ecuación del divisor de tensión
    float resistance = (REFERENCE_RESISTOR_defaul * (VCC - voltage_reading)) / voltage_reading;

    // Se convierte la resistencia a temperatura usando la ecuación de Steinhart-Hart simplificada
    float temperature_kelvin = 1 / ((log(resistance / NOMINAL_REFERENCE_NTC_defaul) / B_defaul) + (1 / T_REF));
    float temperature_celsius = temperature_kelvin - 273.15;

    return temperature_celsius;
}


float adc_to_FC28_humidity(int adc_reading)
{
    // Se convierte la lectura del ADC a voltaje
    float voltage_reading = (float)adc_reading / ADC_RESOLUTION * VCC;

    float humidity = 100*(1-(voltage_reading/3.3));

    return humidity;
}