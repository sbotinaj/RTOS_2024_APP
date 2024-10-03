#include "main.h"

#include "driver/ledc.h"
#include "freertos/timers.h"

#define FAN_PIN 12
#define CHANNEL_FAN 4

#define FAN_ON 100
#define FAN_OFF 0


void channel_fan_config(uint32_t GPIO, uint32_t CHANNEL);

void set_fan_duty(uint32_t CHANNEL, uint8_t duty_cycle);

uint8_t fan_duty_adjust(uint8_t duty_cycle);

typedef struct{
    char Fan_min_temp[32];
    char Fan_max_temp[32];
}   Fan_control_config_t;

Fan_control_config_t* get_Fan_control_config(void);

void save_Fan_control_config(Fan_control_config_t *config);
esp_err_t load_Fan_control_config(Fan_control_config_t *config);
void initialize_Fan_config(void);

void FanControl_task(void *pvParameter);
void FanControl(float temperature);

static esp_err_t Fan_Control_handler(httpd_req_t *req);
void Fan_control_uris(httpd_handle_t http_server_handle);
