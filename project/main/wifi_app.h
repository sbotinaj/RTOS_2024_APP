#ifndef WIFI_APP_H
#define WIFI_APP_H

// Include any necessary libraries or headers
#include "wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
// Declare any global variables or constants

#define WIFI_SSID "SSID"
#define WIFI_PASS "PASSWORD"


// Declare function prototypes

// funtios for wifi connection whit client (station)
void wifi_init_sta(void);
void wifi_connect_sta(void);
void wifi_disconnect_sta(void);
void wifi_event_handler_sta(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);


#endif // WIFI_APP_H