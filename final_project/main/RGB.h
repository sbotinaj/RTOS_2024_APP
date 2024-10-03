#include "main.h"
#include "driver/ledc.h"
#include "freertos/timers.h"



/*Definicion de Pines y Canales para el control
del LED_RGB1*/

#define RGB_PINES 3 // Numero de leds dentro del RGB

/*Se definen los pines para el primer led RGB*/
#define LED_RED1 26
#define LED_GREEN1 25
#define LED_BLUE1 33

/*Se definen los canales para cada uno de los pines del RGB*/
#define CHANNEL_LED_RED1 0
#define CHANNEL_LED_GREEN1 1
#define CHANNEL_LED_BLUE1 2

/*Control del encendido, apagado y valores intermedios de la intensidad
del led RGB*/
#define LED_RGB_ON 100
#define LED_RGB_OFF 0
#define LED_INTENSITY_INCREMENT 10

#pragma once

typedef struct
{
    uint32_t pines[RGB_PINES];    // GPIOs para los LEDs Rojo, Verde y Azul
    uint32_t channels[RGB_PINES]; // Canales LEDC para los LEDs Rojo, Verde y Azul
} LedRGB;

void channel_config(uint32_t GPIO, uint32_t CHANNEL);
void timer_config(void);
void config_RGB(uint32_t GPIOS[], uint32_t CHANNELS[]);
void set_rgb_duty(uint32_t CHANNELS[], uint32_t duty_cycle[]);
void duty_adjust(uint32_t duty_cycle_RGB1[]);

uint8_t pwm_up(uint8_t duty_cycle);
uint8_t pwm_down(uint8_t duty_cycle); 

void set_ledRGB(uint8_t duty_cycleR,uint8_t duty_cycleG,uint8_t duty_cycleB, LedRGB ledRGB_N);

