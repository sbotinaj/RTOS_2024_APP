#include "BUZZER_control.h"
#include "RGB_control.h"


static const char TAG[] = "Buzzer_control";

extern QueueHandle_t temperature_Buzzer_Control_Queue;
extern QueueHandle_t Buzzer_pwm_Queue;
Buzzer_control_config_t *Buzzer_control_config = NULL;

void channel_buzzer_config(uint32_t GPIO, uint32_t CHANNEL)
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

void set_buzzer_duty(uint32_t CHANNEL, uint8_t duty_cycle){
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, CHANNEL, duty_cycle);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, CHANNEL);
    
}

uint8_t buzzer_duty_adjust(uint8_t duty_cycle){

    
    return (255 * duty_cycle) / 100;
}

//-----------------------------------------------------------------
Buzzer_control_config_t* get_Buzzer_control_config(void){
    return Buzzer_control_config;
}

void save_Buzzer_control_config(Buzzer_control_config_t *config)
{
    nvs_handle_t nvs_handle;

    // Abre el espacio de nombres NVS
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    // Guarda los valores en NVS
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "Buzzer_min_temp", config->Buzzer_min_temp));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "Buzzer_max_temp", config->Buzzer_max_temp));
   

    // Compromete los cambios en NVS
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));

    // Cierra el handle NVS
    nvs_close(nvs_handle);
}

esp_err_t load_Buzzer_control_config(Buzzer_control_config_t *config)
{
    esp_err_t err;

    err = load_parameters(config->Buzzer_min_temp, "Buzzer_min_temp");
    if (err != ESP_OK)
        return err;
    err = load_parameters(config->Buzzer_max_temp, "Buzzer_max_temp");
    return err;
}



void initialize_Buzzer_config(void)
{
    // Intenta cargar la configuración del LED RGB

    Buzzer_control_config_t *Buzzer_config = get_Buzzer_control_config();

    esp_err_t err = load_Buzzer_control_config(Buzzer_config);

    if (err == ESP_OK)
    {
        // La configuración se ha cargado correctamente
    }
    else
    {
        // Manejo de error: inicializa a valores predeterminados

        strncpy(Buzzer_config->Buzzer_min_temp, "0", sizeof(Buzzer_config->Buzzer_min_temp));
        strncpy(Buzzer_config->Buzzer_max_temp, "0", sizeof(Buzzer_config->Buzzer_max_temp));
        

        // Inicializa los valores de los LEDs B también si es necesario
    }
}

void BuzzerControl_task(void *pvParameter)
{

    float temperature;
    // ledRGB_control_config_float ledRGB_control_config;
    while (1)
    {
        xQueueReceive(temperature_Buzzer_Control_Queue, &temperature, pdMS_TO_TICKS(100));
        BuzzerControl(temperature);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}

void BuzzerControl(float temperature){

    uint8_t duty_cycle_buzzer;

    Buzzer_control_config_t *Buzzer_control_config = get_Buzzer_control_config();

    float BuzzerminTemperature = atof(Buzzer_control_config->Buzzer_min_temp);
    float BuzzermaxTemperature = atof(Buzzer_control_config->Buzzer_max_temp);

    if (temperature >= BuzzerminTemperature && temperature <= BuzzermaxTemperature)
    {
        duty_cycle_buzzer= ((temperature-BuzzerminTemperature)/(BuzzermaxTemperature-BuzzerminTemperature))*100;

        duty_cycle_buzzer = (int)duty_cycle_buzzer;
    }else if(temperature>BuzzermaxTemperature)
    {
        duty_cycle_buzzer = BUZZER_ON;
        
    }else{
        duty_cycle_buzzer =BUZZER_OFF;
    } 
    xQueueSend(Buzzer_pwm_Queue, &duty_cycle_buzzer, pdMS_TO_TICKS(100));

    set_buzzer_duty(CHANNEL_BUZZER,buzzer_duty_adjust(duty_cycle_buzzer));


}



static esp_err_t Buzzer_Control_handler(httpd_req_t *req)
{
    size_t header_len;
    char *header_value;
    char *Buzzer_min_temp = NULL;
    char *Buzzer_max_temp = NULL;
  
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

    cJSON *Buzzer_min_temp_json = cJSON_GetObjectItem(root, "Buzzer_min_temp");
    cJSON *Buzzer_max_tempR_json = cJSON_GetObjectItem(root, "Buzzer_max_temp");
    

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
    Buzzer_min_temp = strdup(Buzzer_min_temp_json->valuestring);
    Buzzer_max_temp = strdup(Buzzer_max_tempR_json->valuestring);
 
    cJSON_Delete(root);

    // Now, you have the parameters to control led rgb
    ESP_LOGI(TAG, "Received Buzzer_min_temp: %s", Buzzer_min_temp);
    ESP_LOGI(TAG, "Received Buzzer_max_temp: %s", Buzzer_max_temp);
    

    // Update the parameter to control led rgb
    Buzzer_control_config_t *Buzzer_control_config = get_Buzzer_control_config();
    memset(Buzzer_control_config, 0x00, sizeof(Buzzer_control_config_t));
    // memset(wifi_config->sta.ssid, 0x00, sizeof(wifi_config->sta.ssid));
    // memset(wifi_config->sta.password, 0x00, sizeof(wifi_config->sta.password));
    strncpy(Buzzer_control_config->Buzzer_min_temp, Buzzer_min_temp, sizeof(Buzzer_control_config->Buzzer_min_temp));
    strncpy(Buzzer_control_config->Buzzer_max_temp, Buzzer_max_temp, sizeof(Buzzer_control_config->Buzzer_max_temp));
   

    free(Buzzer_min_temp);
    free(Buzzer_max_temp);

    httpd_resp_set_hdr(req, "Connection", "close");
	httpd_resp_send(req, NULL, 0); 
    

    return ESP_OK;
}

esp_err_t show_duty_buzzer_handler(httpd_req_t *req)
{
	// ESP_LOGI(TAG, "/show_duty requested");
    uint8_t duty_cycle_buzzer;
    xQueueReceive(Buzzer_pwm_Queue, &duty_cycle_buzzer, pdMS_TO_TICKS(100));
	char dutyJSON[100];


	sprintf(dutyJSON, "{\"duty_buzzer\":\"%u\"}", duty_cycle_buzzer);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, dutyJSON, strlen(dutyJSON));

    httpd_resp_set_hdr(req, "Connection", "close");
	httpd_resp_send(req, NULL, 0); 

	return ESP_OK;
}


void BUZZER_control_uris(httpd_handle_t http_server_handle)
{
    httpd_uri_t BuzzerControl = {
        .uri = "/BuzzerControl.json",
        .method = HTTP_POST,
        .handler = Buzzer_Control_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(http_server_handle, &BuzzerControl);

    httpd_uri_t uri_show_pwm_buzzer = {
			.uri = "/show_duty_buzzer",
			.method = HTTP_GET,
			.handler = show_duty_buzzer_handler,
			.user_ctx = NULL};
		httpd_register_uri_handler(http_server_handle, &uri_show_pwm_buzzer);

}