#include "main.h"

#include "driver/ledc.h"
#include "freertos/timers.h"

#define BUZZER_PIN 14

#define CHANNEL_BUZZER 3

#define BUZZER_ON 100

#define BUZZER_OFF 0

void channel_buzzer_config(uint32_t GPIO, uint32_t CHANNEL);

void set_buzzer_duty(uint32_t CHANNEL, uint8_t duty_cycle);

uint8_t buzzer_duty_adjust(uint8_t duty_cycle);

typedef struct{
    char Buzzer_min_temp[32];
    char Buzzer_max_temp[32];
}Buzzer_control_config_t;

Buzzer_control_config_t* get_Buzzer_control_config(void);
void save_Buzzer_control_config(Buzzer_control_config_t *config);
esp_err_t load_Buzzer_control_config(Buzzer_control_config_t *config);
void initialize_Buzzer_config(void);

void BuzzerControl_task(void *pvParameter);
void BuzzerControl(float temperature);

static esp_err_t Buzzer_Control_handler(httpd_req_t *req);
void BUZZER_control_uris(httpd_handle_t http_server_handle);
