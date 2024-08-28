#include "rgb.h"
#include "driver/ledc.h"

void init_rgb(int GPIO_RED, int GPIO_GREEN, int GPIO_BLUE) {
    // Configure the LED channels
    ledc_channel_config_t ledc_channel[3] = {
        {
            .channel    = LEDC_CHANNEL_0,
            .duty       = 0,
            .gpio_num   = GPIO_RED,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
        },
        {
            .channel    = LEDC_CHANNEL_1,
            .duty       = 0,
            .gpio_num   = GPIO_GREEN,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
        },
        {
            .channel    = LEDC_CHANNEL_2,
            .duty       = 0,
            .gpio_num   = GPIO_BLUE,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
        }
    };

    // Configure the LED timer
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0
    };

    // Set the LED channels
    for (int i = 0; i < 3; i++) {
        ledc_channel_config(&ledc_channel[i]);
    }

    // Set the LED timer
    ledc_timer_config(&ledc_timer);
}

void set_rgb_color(int red, int green, int blue) {
    // TODO: Implement code to set the RGB color
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, red);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, green);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, blue);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
}

void fade_rgb_color(int adc_input) {

    // Fade the RGB color based on the ADC input
    int red = adc_input;
    int green = 1023 - adc_input;
    int blue = 0;

    // Set the RGB color
    set_rgb_color(red, green, blue);
}



// Other functions and definitions can be added as needed
