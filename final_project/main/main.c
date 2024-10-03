/**
 * Application entry point.
 */

#include "main.h"
#include "nvs_flash.h"
// #include "http_server.h"
#include "wifi_app.h"
#include "driver/gpio.h"
#include "ADC_Driver.h"
#include "RGB.h"
#include "http_server.h"
#include "control_app/RGB_control.h"
#include "control_app/BUZZER_control.h"
#include "control_app/FAN_control.h"

#include "esp_system.h"    // Para usar esp_random()

/* typedef struct
{
    uint32_t pines[RGB_PINES];    // GPIOs para los LEDs Rojo, Verde y Azul
    uint32_t channels[RGB_PINES]; // Canales LEDC para los LEDs Rojo, Verde y Azul
} LedRGB; 
 */

// Inicialización de un LED RGB
LedRGB ledRGB1 = {
	.pines = {LED_RED1, LED_GREEN1, LED_BLUE1},
	.channels = {CHANNEL_LED_RED1, CHANNEL_LED_GREEN1, CHANNEL_LED_BLUE1},
};


QueueHandle_t adc_read_temperature_Queue;
QueueHandle_t temperature_LED_RGB_Control_Queue;
QueueHandle_t temperature_Buzzer_Control_Queue;
QueueHandle_t Buzzer_pwm_Queue;

QueueHandle_t temperature_Fan_Control_Queue;
QueueHandle_t Fan_pwm_Queue;

QueueHandle_t adc_read_humidity_Queue;

QueueHandle_t fan_control_queue;





void app_main(void)
{
	temperature_LED_RGB_Control_Queue= xQueueCreate(10, sizeof(float));
	adc_read_temperature_Queue = xQueueCreate(10, sizeof(float));
	temperature_Buzzer_Control_Queue =xQueueCreate(10, sizeof(float));
	adc_read_humidity_Queue = xQueueCreate(10, sizeof(float));
	Buzzer_pwm_Queue = xQueueCreate(10, sizeof(uint8_t));

	temperature_Fan_Control_Queue =xQueueCreate(10, sizeof(float));
	Fan_pwm_Queue = xQueueCreate(10, sizeof(uint8_t));

	fan_control_queue = xQueueCreate(5, sizeof(Fan_control_config_t));
	
	
	

	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	init_obtain_time();
	wifi_app_start();

	/*Timer para la señal PWM del led RGB*/
	timer_config();

	/*Se configuran los pines y canales para el RGB1*/
	config_RGB(ledRGB1.pines, ledRGB1.channels);

	/*Se configura el pin y el canal del buzzer*/
	channel_buzzer_config(BUZZER_PIN,CHANNEL_BUZZER);

	/*Se configura el pin y el canal del ventilador*/
	channel_buzzer_config(FAN_PIN,CHANNEL_FAN);

	
	
	// Inicializamos el ADC
	ADC1_init();

	// Se configuran los canales por lo que se hace la lectura
	ADC1_set(TEMPERATURE_CHANNEL);
	ADC1_set(HUMIDITY_CHANNEL);

	xTaskCreatePinnedToCore(ADC1_ReadTemperature_task, "ADC1_ReadTemperature_task", 2048, NULL, 3, NULL, 1);
	xTaskCreatePinnedToCore(ADC1_ReadHumidity_task, "ADC1_ReadHumidity_task", 2048, NULL, 2, NULL, 1);

	

	// Start Wifi
	
	//configure_led();
	
	


	

	
}














