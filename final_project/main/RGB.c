#include "RGB.h"



static const char *TAG = "example";



void channel_config(uint32_t GPIO, uint32_t CHANNEL)
{
    ledc_channel_config_t channelConfig = {0};

    channelConfig.gpio_num = GPIO;
    channelConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
    channelConfig.channel = CHANNEL;
    channelConfig.intr_type = LEDC_INTR_DISABLE;
    channelConfig.timer_sel = LEDC_TIMER_0;
    channelConfig.duty = 0; // Inicia con los LEDs apagados. Ajustar según sea necesario.

    ledc_channel_config(&channelConfig);
}

void timer_config(void)
{
    ledc_timer_config_t timerConfig = {0};
    timerConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
    timerConfig.duty_resolution = LEDC_TIMER_8_BIT;
    timerConfig.timer_num = LEDC_TIMER_0;
    timerConfig.freq_hz = 20000;

    ledc_timer_config(&timerConfig);
}

void config_RGB(uint32_t GPIOS[], uint32_t CHANNELS[])
{

    for (int i = 0; i < RGB_PINES; ++i)
    {
        channel_config(GPIOS[i], CHANNELS[i]);
        //ESP_LOGI(TAG, "Canal configurado");
    }
    
}
void set_rgb_duty(uint32_t CHANNELS[], uint32_t duty_cycle[])
{
    for (int i = 0; i < RGB_PINES; ++i)
    {
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, CHANNELS[i], duty_cycle[i]);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, CHANNELS[i]);
    }
}

void duty_adjust( uint32_t duty_cycle_RGB1[])
{
    /*Se ingresa un valor de 0 a 100% pero,
    el valor de duty_cycle se mueve de 0 a 2^n,
    donde n es el número de bits empleado para el timer
    */


    int n = 8; // Número de bits del timer

    for (int i = 0; i < RGB_PINES; ++i)
    {
        // El led RGB empleado es de catodo comun
        duty_cycle_RGB1[i] = LED_RGB_ON-duty_cycle_RGB1[i];

        duty_cycle_RGB1[i] = (pow(2, n) * duty_cycle_RGB1[i]) / LED_RGB_ON;

        
    }
}



uint8_t pwm_up(uint8_t duty_cycle){

	if (duty_cycle +LED_INTENSITY_INCREMENT<=LED_RGB_ON){
		duty_cycle = duty_cycle + LED_INTENSITY_INCREMENT;
	}
	else{
		duty_cycle = LED_RGB_ON;
	}

	return duty_cycle;

}
uint8_t pwm_down(uint8_t duty_cycle){

	if (duty_cycle - LED_INTENSITY_INCREMENT>=LED_RGB_OFF){
		duty_cycle = duty_cycle- LED_INTENSITY_INCREMENT;
	}
	else{
		duty_cycle = LED_RGB_OFF;
	}

	return duty_cycle;

} 

void set_ledRGB(uint8_t duty_cycleR,uint8_t duty_cycleG,uint8_t duty_cycleB, LedRGB ledRGB_N)
{
	uint8_t duty_red1 = duty_cycleR;
	uint8_t duty_green1 = duty_cycleG;
	uint8_t duty_blue1 = duty_cycleB;
	uint32_t duty_cycle_RGB1[RGB_PINES] = {duty_red1, duty_green1, duty_blue1};
	
	duty_adjust(duty_cycle_RGB1);

	
	set_rgb_duty(ledRGB_N.channels, duty_cycle_RGB1);
} 



