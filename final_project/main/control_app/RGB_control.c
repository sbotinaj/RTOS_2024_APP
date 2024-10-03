#include "RGB_control.h"
#include "BUZZER_control.h"
#include "FAN_control.h"

//Cola externa para obtener el valor de temperatura del adc y realizar posteriores 
//comparaciones
extern QueueHandle_t temperature_LED_RGB_Control_Queue;


extern LedRGB ledRGB1;

ledRGB_control_config_t *ledRGB_control_config = NULL;

static const char TAG[] = "RGB_control";

ledRGB_control_config_t *get_ledRGB_control_config(void)
{
    return ledRGB_control_config;
}

void save_ledRGB_control_config(ledRGB_control_config_t *config)
{
    nvs_handle_t nvs_handle;

    // Abre el espacio de nombres NVS
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    // Guarda los valores en NVS
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "minTemperatureR", config->minTemperatureR));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "maxTemperatureR", config->maxTemperatureR));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "intensityR", config->intensityR));

    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "minTemperatureG", config->minTemperatureG));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "maxTemperatureG", config->maxTemperatureG));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "intensityG", config->intensityG));

    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "minTemperatureB", config->minTemperatureB));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "maxTemperatureB", config->maxTemperatureB));
    ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "intensityB", config->intensityB));

    // Compromete los cambios en NVS
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));

    // Cierra el handle NVS
    nvs_close(nvs_handle);
}

esp_err_t load_ledRGB_control_config(ledRGB_control_config_t *config)
{
    esp_err_t err;

    err = load_parameters(config->minTemperatureR, "minTemperatureR");
    if (err != ESP_OK)
        return err;

    err = load_parameters(config->maxTemperatureR, "maxTemperatureR");
    if (err != ESP_OK)
        return err;

    err = load_parameters(config->intensityR, "intensityR");
    if (err != ESP_OK)
        return err;

    err = load_parameters(config->minTemperatureG, "minTemperatureG");
    if (err != ESP_OK)
        return err;

    err = load_parameters(config->maxTemperatureG, "maxTemperatureG");
    if (err != ESP_OK)
        return err;

    err = load_parameters(config->intensityG, "intensityG");
    if (err != ESP_OK)
        return err;

    err = load_parameters(config->minTemperatureB, "minTemperatureB");
    if (err != ESP_OK)
        return err;

    err = load_parameters(config->maxTemperatureB, "maxTemperatureB");
    if (err != ESP_OK)
        return err;

    err = load_parameters(config->intensityB, "intensityB");
    return err;
}

esp_err_t load_parameters(char *parameter, const char *root)
{
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READONLY, &nvs_handle));

    size_t required_size;

    // Get the size of the parameter
    // ESP_ERROR_CHECK(nvs_get_str(nvs_handle, root, NULL, &required_size));
    esp_err_t err = nvs_get_str(nvs_handle, root, NULL, &required_size);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        // No hay datos, retorna un error específico
        return ESP_ERR_NVS_NOT_FOUND;
    }
    else if (err != ESP_OK)
    {
        // Otro error en la operación de NVS
        return err;
    }

    err = nvs_get_str(nvs_handle, root, NULL, &required_size);

    // Allocate memory for the parameter
    char *parameter_buffer = malloc(required_size);
    if (parameter_buffer == NULL)
    {
        // Handle memory allocation error
        ESP_LOGE(TAG, "Failed to allocate memory for %s", root);
        nvs_close(nvs_handle);
        return err;
    }
    // Get the parameter
    ESP_ERROR_CHECK(nvs_get_str(nvs_handle, root, parameter_buffer, &required_size));
    // Copy parameter to the output parameter
    strncpy(parameter, parameter_buffer, required_size);

    // Free the allocated memory
    free(parameter_buffer);
    nvs_close(nvs_handle);

    return err;
}

void initialize_ledRGB_config(void)
{
    // Intenta cargar la configuración del LED RGB

    ledRGB_control_config_t *ledRGB_config = get_ledRGB_control_config();

    esp_err_t err = load_ledRGB_control_config(ledRGB_config);

    if (err == ESP_OK)
    {
        // La configuración se ha cargado correctamente
    }
    else
    {
        // Manejo de error: inicializa a valores predeterminados

        strncpy(ledRGB_config->minTemperatureR, "0", sizeof(ledRGB_config->minTemperatureR));
        strncpy(ledRGB_config->maxTemperatureR, "0", sizeof(ledRGB_config->maxTemperatureR));
        strncpy(ledRGB_config->intensityR, "0", sizeof(ledRGB_config->intensityR));
        strncpy(ledRGB_config->minTemperatureG, "0", sizeof(ledRGB_config->minTemperatureG));
        strncpy(ledRGB_config->maxTemperatureG, "0", sizeof(ledRGB_config->maxTemperatureG));
        strncpy(ledRGB_config->intensityG, "0", sizeof(ledRGB_config->intensityG));
        strncpy(ledRGB_config->minTemperatureB, "0", sizeof(ledRGB_config->minTemperatureB));
        strncpy(ledRGB_config->maxTemperatureB, "0", sizeof(ledRGB_config->maxTemperatureB));
        strncpy(ledRGB_config->intensityB, "0", sizeof(ledRGB_config->intensityB));

        // Inicializa los valores de los LEDs B también si es necesario
    }
}

void RGBControl_task(void *pvParameter)
{

    float temperature;
    // ledRGB_control_config_float ledRGB_control_config;
    while (1)
    {
        xQueueReceive(temperature_LED_RGB_Control_Queue, &temperature, pdMS_TO_TICKS(100));
        RGBControl(temperature);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}

// void RGBControl(float temperature, ledRGB_control_config_float ledRGB_control_config)
void RGBControl(float temperature)
{
    ledRGB_control_config_t *ledRGB_control_config = get_ledRGB_control_config();

    float minTemperatureR = atof(ledRGB_control_config->minTemperatureR);
    float maxTemperatureR = atof(ledRGB_control_config->maxTemperatureR);
    float intensityR = atof(ledRGB_control_config->intensityR);

    float minTemperatureG = atof(ledRGB_control_config->minTemperatureG);
    float maxTemperatureG = atof(ledRGB_control_config->maxTemperatureG);
    float intensityG = atof(ledRGB_control_config->intensityG);

    float minTemperatureB = atof(ledRGB_control_config->minTemperatureB);
    float maxTemperatureB = atof(ledRGB_control_config->maxTemperatureB);
    float intensityB = atof(ledRGB_control_config->intensityB);

    if (temperature >= minTemperatureR && temperature <= maxTemperatureR)
    {
        set_ledRGB(intensityR,-1,-1,ledRGB1);
        //ledR(intensityR); // Enciende el LED rojo con la intensidad correspondiente
    }

    if (temperature >= minTemperatureG && temperature <= maxTemperatureG)
    {
        set_ledRGB(-1,intensityG,-1,ledRGB1);
        //ledG(intensityG); // Enciende el LED verde con la intensidad correspondiente
    }

    if (temperature >= minTemperatureB && temperature <= maxTemperatureB)
    {   
        set_ledRGB(-1,-1,intensityB,ledRGB1);
        //ledB(intensityB); // Enciende el LED azul con la intensidad correspondiente
    }
}

static esp_err_t ledRGB_Control_handler(httpd_req_t *req)
{
    size_t header_len;
    char *header_value;
    char *minTemperatureR = NULL;
    char *maxTemperatureR = NULL;
    char *intensityR = NULL;
    char *minTemperatureG = NULL;
    char *maxTemperatureG = NULL;
    char *intensityG = NULL;
    char *minTemperatureB = NULL;
    char *maxTemperatureB = NULL;
    char *intensityB = NULL;
    int content_length;

    ESP_LOGI(TAG, "/ledRGBControl.json requested");

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

    cJSON *minTemperatureR_json = cJSON_GetObjectItem(root, "minTemperatureR");
    cJSON *maxTemperatureR_json = cJSON_GetObjectItem(root, "maxTemperatureR");
    cJSON *intensityR_json = cJSON_GetObjectItem(root, "intensityR");

    cJSON *minTemperatureG_json = cJSON_GetObjectItem(root, "minTemperatureG");
    cJSON *maxTemperatureG_json = cJSON_GetObjectItem(root, "maxTemperatureG");
    cJSON *intensityG_json = cJSON_GetObjectItem(root, "intensityG");

    cJSON *minTemperatureB_json = cJSON_GetObjectItem(root, "minTemperatureB");
    cJSON *maxTemperatureB_json = cJSON_GetObjectItem(root, "maxTemperatureB");
    cJSON *intensityB_json = cJSON_GetObjectItem(root, "intensityB");

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
    minTemperatureR = strdup(minTemperatureR_json->valuestring);
    maxTemperatureR = strdup(maxTemperatureR_json->valuestring);
    intensityR = strdup(intensityR_json->valuestring);

    minTemperatureG = strdup(minTemperatureG_json->valuestring);
    maxTemperatureG = strdup(maxTemperatureG_json->valuestring);
    intensityG = strdup(intensityG_json->valuestring);

    minTemperatureB = strdup(minTemperatureB_json->valuestring);
    maxTemperatureB = strdup(maxTemperatureB_json->valuestring);
    intensityB = strdup(intensityB_json->valuestring);

    cJSON_Delete(root);

    // Now, you have the parameters to control led rgb
    ESP_LOGI(TAG, "Received minTemperatureR: %s", minTemperatureR);
    ESP_LOGI(TAG, "Received maxTemperatureR: %s", maxTemperatureR);
    ESP_LOGI(TAG, "Received intensityR: %s", intensityR);

    ESP_LOGI(TAG, "Received minTemperatureG: %s", minTemperatureG);
    ESP_LOGI(TAG, "Received maxTemperatureG: %s", maxTemperatureG);
    ESP_LOGI(TAG, "Received intensityG: %s", intensityG);

    ESP_LOGI(TAG, "Received minTemperatureB: %s", minTemperatureB);
    ESP_LOGI(TAG, "Received maxTemperatureB: %s", maxTemperatureB);
    ESP_LOGI(TAG, "Received intensityB: %s", intensityB);

    // Update the parameter to control led rgb
    ledRGB_control_config_t *ledRGB_control_config = get_ledRGB_control_config();
    memset(ledRGB_control_config, 0x00, sizeof(ledRGB_control_config_t));
    // memset(wifi_config->sta.ssid, 0x00, sizeof(wifi_config->sta.ssid));
    // memset(wifi_config->sta.password, 0x00, sizeof(wifi_config->sta.password));
    strncpy(ledRGB_control_config->minTemperatureR, minTemperatureR, sizeof(ledRGB_control_config->minTemperatureR));
    strncpy(ledRGB_control_config->maxTemperatureR, maxTemperatureR, sizeof(ledRGB_control_config->maxTemperatureR));
    strncpy(ledRGB_control_config->intensityR, intensityR, sizeof(ledRGB_control_config->intensityR));
    strncpy(ledRGB_control_config->minTemperatureG, minTemperatureG, sizeof(ledRGB_control_config->minTemperatureG));
    strncpy(ledRGB_control_config->maxTemperatureG, maxTemperatureG, sizeof(ledRGB_control_config->maxTemperatureG));
    strncpy(ledRGB_control_config->intensityG, intensityG, sizeof(ledRGB_control_config->intensityG));
    strncpy(ledRGB_control_config->minTemperatureB, minTemperatureB, sizeof(ledRGB_control_config->minTemperatureB));
    strncpy(ledRGB_control_config->maxTemperatureB, maxTemperatureB, sizeof(ledRGB_control_config->maxTemperatureB));
    strncpy(ledRGB_control_config->intensityB, intensityB, sizeof(ledRGB_control_config->intensityB));

    free(minTemperatureR);
    free(maxTemperatureR);
    free(intensityR);

    free(minTemperatureG);
    free(maxTemperatureG);
    free(intensityG);

    free(minTemperatureB);
    free(maxTemperatureB);
    free(intensityB);

   httpd_resp_set_hdr(req, "Connection", "close");
	httpd_resp_send(req, NULL, 0);
    
    return ESP_OK;
}

static esp_err_t set_as_default_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "/set_as_default.json requested");

    ledRGB_control_config_t *ledRGB_control_config = get_ledRGB_control_config();
    Buzzer_control_config_t *Buzzer_control_config = get_Buzzer_control_config();
    Fan_control_config_t *Fan_control_config = get_Fan_control_config();

    save_ledRGB_control_config(ledRGB_control_config);
    save_Buzzer_control_config(Buzzer_control_config);
    save_Fan_control_config(Fan_control_config);

    httpd_resp_set_hdr(req, "Connection", "close");
	httpd_resp_send(req, NULL, 0);

    return ESP_OK;
}

void RGB_control_uris(httpd_handle_t http_server_handle)
{
    httpd_uri_t ledRGBControl = {
        .uri = "/ledRGBControl.json",
        .method = HTTP_POST,
        .handler = ledRGB_Control_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(http_server_handle, &ledRGBControl);

    httpd_uri_t set_as_default = {
        .uri = "/set_as_default.json",
        .method = HTTP_POST,
        .handler = set_as_default_handler,
        .user_ctx = NULL};
    httpd_register_uri_handler(http_server_handle, &set_as_default);
}