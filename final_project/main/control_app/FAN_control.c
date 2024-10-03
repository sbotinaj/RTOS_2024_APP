#include "FAN_control.h"
#include "RGB_control.h"

static const char TAG[] = "Fan_control";

extern QueueHandle_t temperature_Fan_Control_Queue;
extern QueueHandle_t Fan_pwm_Queue;

extern QueueHandle_t fan_control_queue;


Fan_control_config_t *Fan_control_config = NULL;

void channel_fan_config(uint32_t GPIO, uint32_t CHANNEL)
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

void set_fan_duty(uint32_t CHANNEL, uint8_t duty_cycle)
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, CHANNEL, duty_cycle);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, CHANNEL);
}

uint8_t fan_duty_adjust(uint8_t duty_cycle)
{
    return (255 * duty_cycle) / 100;
}

//-----------------------------------------------------------------
Fan_control_config_t *get_Fan_control_config(void)
{
    return Fan_control_config;
}

void save_Fan_control_config(Fan_control_config_t *config)
{
    nvs_handle_t nvs_handle;

    // Abre el espacio de nombres NVS
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    // Guarda los valores en NVS
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "Fan_min_temp", config->Fan_min_temp));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "Fan_max_temp", config->Fan_max_temp));

    // Compromete los cambios en NVS
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));

    // Cierra el handle NVS
    nvs_close(nvs_handle);
}

esp_err_t load_Fan_control_config(Fan_control_config_t *config)
{

    esp_err_t err;
    err = load_parameters(config->Fan_min_temp, "Fan_min_temp");
    if (err != ESP_OK)
        return err;
    err = load_parameters(config->Fan_max_temp, "Fan_max_temp");
    return err;
}
void initialize_Fan_config(void)
{
    // Intenta cargar la configuración del LED RGB

    Fan_control_config_t *Fan_config = get_Fan_control_config();

    esp_err_t err = load_Fan_control_config(Fan_config);

    if (err == ESP_OK)
    {
        // La configuración se ha cargado correctamente
    }
    else
    {
        // Manejo de error: inicializa a valores predeterminados

        strncpy(Fan_config->Fan_min_temp, "0", sizeof(Fan_config->Fan_min_temp));
        strncpy(Fan_config->Fan_max_temp, "0", sizeof(Fan_config->Fan_max_temp));

        // Inicializa los valores de los LEDs B también si es necesario
    }
}

void FanControl_task(void *pvParameter)
{

    float temperature;
    // ledRGB_control_config_float ledRGB_control_config;
    while (1)
    {
        xQueueReceive(temperature_Fan_Control_Queue, &temperature, pdMS_TO_TICKS(100));
        FanControl(temperature);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void FanControl(float temperature)
{

    uint8_t duty_cycle_fan;



    Fan_control_config_t *Fan_control_config = get_Fan_control_config();

    float FanminTemperature = atof(Fan_control_config->Fan_min_temp);
    float FanmaxTemperature = atof(Fan_control_config->Fan_max_temp); 

    

    if (temperature >= FanminTemperature && temperature <= FanmaxTemperature)
    {
        duty_cycle_fan = ((temperature - FanminTemperature) / (FanmaxTemperature - FanminTemperature)) * 100;

        duty_cycle_fan = (int)duty_cycle_fan;
    }
    else if (temperature > FanmaxTemperature)
    {
        duty_cycle_fan = FAN_ON;
    }
    else
    {
        duty_cycle_fan = FAN_OFF;
    }
    xQueueSend(Fan_pwm_Queue, &duty_cycle_fan, pdMS_TO_TICKS(100));

    /* ESP_LOGI(TAG, "Fan_min_temp Now: %.2f", FanminTemperature);
    ESP_LOGI(TAG, "Fan_max_temp Now: %.2f", FanmaxTemperature);
    ESP_LOGI(TAG, "Duty Cycle Now: %u", duty_cycle_fan); */

    set_fan_duty(CHANNEL_FAN, fan_duty_adjust(duty_cycle_fan));
}

static esp_err_t Fan_Control_handler(httpd_req_t *req)
{
    size_t header_len;
    char *header_value;
    char *Fan_min_temp = NULL;
    char *Fan_max_temp = NULL;

    int content_length;

    ESP_LOGI(TAG, "/BuzzerControl.json requested");

    // Get the "Content-Length" header to determine the length of the request body
    header_len = httpd_req_get_hdr_value_len(req, "Content-Length");
    if (header_len <= 0)
    {
        // Content-Length header not found or invalid
        // httpd_resp_send_err(req, HTTP_STATUS_411_LENGTH_REQUIRED, "Content-Length header is missing or invalid");
        ESP_LOGI(TAG, "Content-Length header is missing or invalid");
        return ESP_FAIL;
    }

    // Allocate memory to store the header value
    header_value = (char *)malloc(header_len + 1);
    if (httpd_req_get_hdr_value_str(req, "Content-Length", header_value, header_len + 1) != ESP_OK)
    {
        // Failed to get Content-Length header value
        free(header_value);
        // httpd_resp_send_err(req, HTTP_STATUS_BAD_REQUEST, "Failed to get Content-Length header value");
        ESP_LOGI(TAG, "Failed to get Content-Length header value");
        return ESP_FAIL;
    }

    // Convert the Content-Length header value to an integer
    content_length = atoi(header_value);
    free(header_value);

    if (content_length <= 0)
    {
        // Content length is not a valid positive integer
        // httpd_resp_send_err(req, HTTP_STATUS_BAD_REQUEST, "Invalid Content-Length value");
        ESP_LOGI(TAG, "Invalid Content-Length value");
        return ESP_FAIL;
    }

    // Allocate memory for the data buffer based on the content length
    char *data_buffer = (char *)malloc(content_length + 1);

    // Read the request body into the data buffer
    if (httpd_req_recv(req, data_buffer, content_length) <= 0)
    {
        // Handle error while receiving data
        free(data_buffer);
        // httpd_resp_send_err(req, HTTP_STATUS_INTERNAL_SERVER_ERROR, "Failed to receive request body");
        ESP_LOGI(TAG, "Failed to receive request body");
        return ESP_FAIL;
    }

    // Null-terminate the data buffer to treat it as a string
    data_buffer[content_length] = '\0';

    // Parse the received JSON data
    cJSON *root = cJSON_Parse(data_buffer);
    free(data_buffer);

    if (root == NULL)
    {
        // JSON parsing error
        // httpd_resp_send_err(req, HTTP_STATUS_BAD_REQUEST, "Invalid JSON data");
        ESP_LOGI(TAG, "Invalid JSON data");
        return ESP_FAIL;
    }

    cJSON *Fan_min_temp_json = cJSON_GetObjectItem(root, "Fan_min_temp");
    cJSON *Fan_max_temp_json = cJSON_GetObjectItem(root, "Fan_max_temp");

    /* if (minTemperatureR_json == NULL || maxTemperatureR_json == NULL || intensityR_json == NULL ||
        !cJSON_IsNumber(minTemperatureR_json) || !cJSON_IsNumber(maxTemperatureR_json) || !cJSON_IsNumber(intensityR_json) ||
        minTemperatureG_json == NULL || maxTemperatureG_json == NULL || intensityG_json == NULL ||
        !cJSON_IsNumber(minTemperatureG_json) || !cJSON_IsNumber(maxTemperatureG_json) || !cJSON_IsNumber(intensityG_json) ||
        minTemperatureB_json == NULL || maxTemperatureB_json == NULL || intensityB_json == NULL ||
        !cJSON_IsNumber(minTemperatureB_json) || !cJSON_IsNumber(maxTemperatureB_json) || !cJSON_IsNumber(intensityB_json))
    {
        cJSON_Delete(root);
        ESP_LOGI(TAG, "Missing or invalid JSON data fields for LED control");
        return ESP_FAIL;
    } */

    // Extract from JSON
    Fan_min_temp = strdup(Fan_min_temp_json->valuestring);
    Fan_max_temp = strdup(Fan_max_temp_json->valuestring);
    cJSON_Delete(root);

    // Now, you have the parameters to control led rgb
    ESP_LOGI(TAG, "Received Fan_min_temp: %s", Fan_min_temp);
    ESP_LOGI(TAG, "Received Fan_max_temp: %s", Fan_max_temp);

    // Update the parameter to control led rgb
    Fan_control_config_t *Fan_control_config = get_Fan_control_config();
    memset(Fan_control_config, 0x00, sizeof(Fan_control_config_t));
    // memset(wifi_config->sta.ssid, 0x00, sizeof(wifi_config->sta.ssid));
    // memset(wifi_config->sta.password, 0x00, sizeof(wifi_config->sta.password));
    strncpy(Fan_control_config->Fan_min_temp, Fan_min_temp, sizeof(Fan_control_config->Fan_min_temp));
    strncpy(Fan_control_config->Fan_max_temp, Fan_max_temp, sizeof(Fan_control_config->Fan_max_temp));

    free(Fan_min_temp);
    free(Fan_max_temp);

    httpd_resp_set_hdr(req, "Connection", "close");
     httpd_resp_send(req, NULL, 0);

    return ESP_OK;
}

esp_err_t show_duty_fan_handler(httpd_req_t *req)
{
    // ESP_LOGI(TAG, "/show_duty requested");
    uint8_t duty_cycle_fan;
    xQueueReceive(Fan_pwm_Queue, &duty_cycle_fan, pdMS_TO_TICKS(100));
    char dutyJSON[100];

    sprintf(dutyJSON, "{\"duty_fan\":\"%u\"}", duty_cycle_fan);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, dutyJSON, strlen(dutyJSON));

    httpd_resp_set_hdr(req, "Connection", "close");
     httpd_resp_send(req, NULL, 0); 

    return ESP_OK;
}
void Fan_control_uris(httpd_handle_t http_server_handle)
{

    httpd_uri_t FanControl = {
        .uri = "/FanControl.json",
        .method = HTTP_POST,
        .handler = Fan_Control_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(http_server_handle, &FanControl);

    httpd_uri_t uri_show_pwm_fan = {
        .uri = "/show_duty_fan",
        .method = HTTP_GET,
        .handler = show_duty_fan_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(http_server_handle, &uri_show_pwm_fan);
}
