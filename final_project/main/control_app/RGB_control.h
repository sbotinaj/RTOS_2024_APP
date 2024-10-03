
#include "main.h"
#include "RGB.h"


typedef struct {
    char minTemperatureR[32]; // Temperatura mínima para el LED rojo
    char maxTemperatureR[32]; // Temperatura máxima para el LED rojo
    char intensityR[32];      // Intensidad del LEchar
    char minTemperatureG[32]; // Temperatura mínima para el LED verde
    char maxTemperatureG[32]; // Temperatura máxima para el LED verde
    char intensityG[32];      // Intensidad del LEDchar
    char minTemperatureB[32]; // Temperatura mínima para el LED azul
    char maxTemperatureB[32]; // Temperatura máxima para el LED azul
    char intensityB[32];      // Intensidad del LED azul
} ledRGB_control_config_t;


//Obtener las configuraciones actuales de la estructura
ledRGB_control_config_t* get_ledRGB_control_config(void);

//Guardar en memoria flash las configuraciones 
void save_ledRGB_control_config(ledRGB_control_config_t *config);

//Cargar desde memoria flash las configuraciones
esp_err_t load_ledRGB_control_config(ledRGB_control_config_t *config);

//Funcion para automatizar la forma en la que se cargan los datos de la flash
esp_err_t  load_parameters(char *parameter, const char *root); 

//Incializa la configuracion establecida por default
void initialize_ledRGB_config(void);

//Tarea para las acciones del led rgb
void RGBControl_task(void *pvParameter);
void RGBControl(float temperature);

//Controlador par la uri de los datos ingrasados en la pagina
static esp_err_t ledRGB_Control_handler(httpd_req_t *req);

//Controlador par la uri de guardado de datos en la flash
static esp_err_t set_as_default_handler(httpd_req_t *req);

//Definicion del uris que se relacionan con el control del led rgb
void RGB_control_uris (httpd_handle_t http_server_handle);