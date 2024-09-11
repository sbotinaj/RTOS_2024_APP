#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "lwip/netdb.h"

// API includes
#include "esp_http_client.h" // HTTP client
#include "cJSON.h"			 // JSON parser

#include "esp_sntp.h" // SNTP client

#include "wifi_std.h"

// Tag used for ESP serial console messages
static const char TAG[] = "wifi_app";
static const char TAG_HTTP[] = "htttp_client";
static const char TAG_SNTP[] = "sntp_client";

//static const char TAG_STA[] = "wifi_sta";

// Queue handle used to manipulate the main queue of events
static QueueHandle_t wifi_app_queue_handle;

// netif objects for the station and access point
esp_netif_t *esp_netif_sta = NULL;
esp_netif_t *esp_netif_ap = NULL;

/**
 * WiFi application event handler
 * @param arg data, aside from event data, that is passed to the handler when it is called
 * @param event_base the base id of the event to register the handler for
 * @param event_id the id fo the event to register the handler for
 * @param event_data event data
 */
static void wifi_app_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	if (event_base == WIFI_EVENT)
	{
		switch (event_id)
		{
		case WIFI_EVENT_AP_START:
			ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
			break;

		case WIFI_EVENT_AP_STOP:
			ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
			break;

		case WIFI_EVENT_AP_STACONNECTED:
			ESP_LOGI(TAG, "WIFI_EVENT_AP_STACONNECTED");
			break;

		case WIFI_EVENT_AP_STADISCONNECTED:
			ESP_LOGI(TAG, "WIFI_EVENT_AP_STADISCONNECTED");
			break;

		case WIFI_EVENT_STA_START:
			ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
			esp_wifi_connect();
			break;
		
		case WIFI_EVENT_SCAN_DONE:
			ESP_LOGI(TAG, "WIFI_EVENT_SCAN_DONE");
			break;

		case WIFI_EVENT_STA_CONNECTED:
			ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
			
			break;

		case WIFI_EVENT_STA_DISCONNECTED:
			ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
			break;
		}
	}
	else if (event_base == IP_EVENT)
	{
		switch (event_id)
		{
		case IP_EVENT_STA_GOT_IP:
			ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
			wifi_app_send_message(WIFI_APP_MSG_STA_CONNECTED_GOT_IP);
			break;
		}
	}
}

/**
 * Initializes the WiFi application event handler for WiFi and IP events.
 */
static void wifi_app_event_handler_init(void)
{
	// Event loop for the WiFi driver
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Event handler for the connection
	esp_event_handler_instance_t instance_wifi_event;
	esp_event_handler_instance_t instance_ip_event;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_wifi_event));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_ip_event));
}

/**
 * Initializes the TCP stack and default WiFi configuration.
 */
static void wifi_app_default_wifi_init(void)
{
	// Initialize the TCP stack
	esp_netif_init();

	// Default WiFi config - operations must be in this order!
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	esp_netif_sta = esp_netif_create_default_wifi_sta();
	//esp_netif_ap = esp_netif_create_default_wifi_ap();
}

/**
 * Configures the WiFi access point settings and assigns the static IP to the SoftAP.
 */
// static void wifi_app_soft_ap_config(void)
// {
// 	// SoftAP - WiFi access point configuration
// 	wifi_config_t ap_config =
// 		{
// 			.ap = {
// 				.ssid = WIFI_AP_SSID,
// 				.ssid_len = strlen(WIFI_AP_SSID),
// 				.password = WIFI_AP_PASSWORD,
// 				.channel = WIFI_AP_CHANNEL,
// 				.ssid_hidden = WIFI_AP_SSID_HIDDEN,
// 				.authmode = WIFI_AUTH_WPA2_PSK,
// 				.max_connection = WIFI_AP_MAX_CONNECTIONS,
// 				.beacon_interval = WIFI_AP_BEACON_INTERVAL,
// 			},
// 		};

// 	// Configure DHCP for the AP
// 	esp_netif_ip_info_t ap_ip_info;
// 	memset(&ap_ip_info, 0x00, sizeof(ap_ip_info));

// 	esp_netif_dhcps_stop(esp_netif_ap);				///> must call this first
// 	inet_pton(AF_INET, WIFI_AP_IP, &ap_ip_info.ip); ///> Assign access point's static IP, GW, and netmask
// 	inet_pton(AF_INET, WIFI_AP_GATEWAY, &ap_ip_info.gw);
// 	inet_pton(AF_INET, WIFI_AP_NETMASK, &ap_ip_info.netmask);
// 	ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info)); ///> Statically configure the network interface
// 	ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));			   ///> Start the AP DHCP server (for connecting stations e.g. your mobile device)

// 	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));					///> Setting the mode as Access Point / Station Mode
// 	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));		///> Set our configuration
// 	ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_AP_BANDWIDTH)); ///> Our default bandwidth 20 MHz
// 	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_STA_POWER_SAVE));					///> Power save set to "NONE"
// }

/**
 * Configures the WiFi station settings.
 */
static void wifi_app_sta_config(void)
{
	// Station - WiFi station configuration
	wifi_config_t sta_config =
	{
		.sta = {
			.ssid = WIFI_STA_SSID,					   // SSID for the station
			.password = WIFI_STA_PASSWORD,			   // Password for the station
			.channel = WIFI_STA_CHANNEL,			   // Channel for the station
			.threshold.authmode = WIFI_STA_AUTH_MODE,  // Authentication mode for the station
			.sae_pwe_h2e = WIFI_STA_SAE_MODE,	   // PMF capable for the station
			.sae_h2e_identifier = WIFI_STA_H2E_ID, // PMF required for the station
		}
};

// Configure the station
esp_wifi_set_mode(WIFI_MODE_STA);						///> Setting the mode as Access Point / Station Mode
esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config);		///> Set our configuration
// log console
ESP_LOGI(TAG, "Configuring WiFi Station");
//esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_AP_BANDWIDTH); ///> Our default bandwidth 20 MHz
//esp_wifi_set_ps(WIFI_STA_POWER_SAVE);					///> Power save set to "NONE"
}


// struct for save data of curl api
typedef struct {
	char *memory;
	size_t size;
} data_time;

/*
	get time from API
*/
static void get_time_from_api(void)
{
	esp_http_client_config_t config = {
		.url = API_TIME_URL,
		.host = API_TIME_HOST,
		.auth_type = HTTP_AUTH_TYPE_NONE,
		.event_handler = NULL,
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);
	ESP_LOGI(TAG_HTTP, "HTTP client init");
	esp_http_client_set_timeout_ms(client, 10000); // Set timeout to 10 seconds

	esp_err_t err;
	int retry_count = 0;
	const int max_retries = 5;

	do {
		err = esp_http_client_perform(client);
		if (err == ESP_OK) {
			int status_code = esp_http_client_get_status_code(client);
			if (status_code == 200) {
				// save data in struct
				data_time data;
				data.memory = malloc(1); // Will be grown as needed by the realloc
				data.size = 0; // no data at this point
				// get data from API
				esp_http_client_fetch_headers(client);
				//read full data
				int content_length = esp_http_client_get_content_length(client);
				char *buffer = malloc(content_length + 1);
				if (buffer) {
					int total_read_len = 0, read_len;
					while (total_read_len < content_length) {
						read_len = esp_http_client_read(client, buffer + total_read_len, content_length - total_read_len);
						if (read_len <= 0) {
							ESP_LOGE(TAG_HTTP, "Error reading data");
							break;
						}
						total_read_len += read_len;
					}
					buffer[total_read_len] = '\0'; // Null-terminate the buffer

					// Save the data in the struct
					data.memory = realloc(data.memory, total_read_len + 1);
					if (data.memory) {
						memcpy(data.memory, buffer, total_read_len + 1);
						data.size = total_read_len;
						ESP_LOGI(TAG_HTTP, "Received data: %s", data.memory);
					} else {
						ESP_LOGE(TAG_HTTP, "Failed to allocate memory");
					}
					free(buffer);
				} else {
					ESP_LOGE(TAG_HTTP, "Failed to allocate buffer");
				}


				break; // Exit the loop if successful
			} else {
				ESP_LOGE(TAG, "HTTP request error, status code: %d", status_code);
			}
		} else {
			ESP_LOGE(TAG, "HTTP request error: %s", esp_err_to_name(err));
		}
		retry_count++;
		ESP_LOGI(TAG_HTTP, "Retrying... (%d/%d)", retry_count, max_retries);
		vTaskDelay(pdMS_TO_TICKS(2000)); // Wait for 2 seconds before retrying
	} while (err != ESP_OK && retry_count < max_retries);

	esp_http_client_cleanup(client);
}

static void initialize_sntp(void)
{
	ESP_LOGI(TAG_SNTP, "Initializing SNTP");
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org");
	sntp_init();
}

static void obtain_time(void)
{
	initialize_sntp();

	// wait for time to be set
	time_t now = 0;
	struct tm timeinfo = {0};
	int retry = 0;
	const int retry_count = 10;
	while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
		ESP_LOGI(TAG_SNTP, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
		vTaskDelay(pdMS_TO_TICKS(2000));
		time(&now);
		localtime_r(&now, &timeinfo);
	}

	// Set timezone to Colombia
	setenv("TZ", "COT5", 1);
	tzset();
}


static void print_current_time(void)
{
	time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	ESP_LOGI(TAG_SNTP, "Current time: %s", asctime(&timeinfo));
}

/**
 * Main task for the WiFi application
 * @param pvParameters parameter which can be passed to the task
 */
static void wifi_app_task(void *pvParameters)
{
	wifi_app_queue_message_t msg;

	// Initialize the event handler
	wifi_app_event_handler_init();

	// Initialize the TCP/IP stack and WiFi config
	wifi_app_default_wifi_init();

	// Soft STA config
	wifi_app_sta_config();

	// Start WiFi
	ESP_ERROR_CHECK(esp_wifi_start());

	// Send first event message
	wifi_app_send_message(WIFI_APP_MSG_START_HTTP_SERVER);

	for (;;)
	{
		if (xQueueReceive(wifi_app_queue_handle, &msg, portMAX_DELAY))
		{
			switch (msg.msgID)
			{
			case WIFI_APP_MSG_START_HTTP_SERVER:
				ESP_LOGI(TAG, "WIFI_APP_MSG_START_HTTP_SERVER");
				


				break;

			case WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER:
				ESP_LOGI(TAG, "WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER");

				break;

			case WIFI_APP_MSG_STA_CONNECTED_GOT_IP:
				ESP_LOGI(TAG, "WIFI_APP_MSG_STA_CONNECTED_GOT_IP");
				//get_time_from_api();
				obtain_time();
				print_current_time();
				//rgb_led_wifi_connected();

				break;
			

			default:
				break;
			}
		}
	}
}

BaseType_t wifi_app_send_message(wifi_app_message_e msgID)
{
	wifi_app_queue_message_t msg;
	msg.msgID = msgID;
	return xQueueSend(wifi_app_queue_handle, &msg, portMAX_DELAY);
}



void wifi_app_start(void)
{
	ESP_LOGI(TAG, "STARTING WIFI APPLICATION");

	// Disable default WiFi logging messages
	esp_log_level_set("wifi", ESP_LOG_NONE);

	// Create message queue
	wifi_app_queue_handle = xQueueCreate(3, sizeof(wifi_app_queue_message_t));

	// Start the WiFi application task
	xTaskCreatePinnedToCore(&wifi_app_task, "wifi_app_task", WIFI_APP_TASK_STACK_SIZE, NULL, WIFI_APP_TASK_PRIORITY, NULL, WIFI_APP_TASK_CORE_ID);
}