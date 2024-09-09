
#include "wifi_app.h"

// INIT WIFI STATION

void wifi_init_sta(void)
{
    // Initialize the TCP/IP stack
    tcpip_adapter_init();
    // Create the event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Initialize the WiFi stack in station mode with configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // Register the event handler
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler_sta, NULL));
    // Set the WiFi configuration
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    // Start the WiFi connection
    ESP_ERROR_CHECK(esp_wifi_start());
}
