#include "commun_lib.h"
#include "mqtt_manager.h"

#include "wifi_manager.h"
#include "config_mode.h"
#include "nvs_manager.h"
#include "spiffs_manager.h"
#include "Setup_mode_button.h"


static const char *TAG_MAIN = "MAIN"; // Tag for logging
uint8_t status=0;
httpd_handle_t server = NULL;
esp_mqtt_client_handle_t client = NULL;  // Define client as a global variable
uint32_t saved_data ;


void app_main(void) {
    // Initialize NVS
    init_nvs();
    // Initialize SPIFFS
    init_spiffs();
    // Initialize WIFI
    wifi_init();
    // Initialize GPIO 
    gpio_button_mode_init(setup_mode_button);
    // Retrieve Gateway mode from NVS
    saved_data = get_saved_data_32_from_flash(NVS_NAMESPACE,Gateway_mode);
    ESP_LOGI(TAG_MAIN, "Gateway_mode: %d", saved_data);

    /*char* Gateway_mode = NULL;
    Gateway_mode = get_saved_data_from_flash(NVS_NAMESPACE, KEY_NAME);
    if (Gateway_mode == NULL) {
        // Save default Gateway mode if not found
        saved_data_in_flash(NVS_NAMESPACE, KEY_NAME, Gateway_mode_nvs);
        Gateway_mode = get_saved_data_from_flash(NVS_NAMESPACE, KEY_NAME); 
        ESP_LOGI(TAG_MAIN, "Save default Gateway_mode: %s", Gateway_mode);  
    }
    status=atoi(Gateway_mode);
    free(Gateway_mode);
    char *Config= read_data("/spiffs/Config.txt") ;
     ESP_LOGI(TAG_MAIN, "Save Config: %s", Config);  
     cJSON* config_1=cJSON_Parse(Config);
    free(Config);*/
    switch (saved_data)
    {
    case 0:        
        ESP_LOGI(TAG_MAIN, "Mode : SETUP Mode!");
        wifi_start_ap();
        break;
    case 1:                
        ESP_LOGI(TAG_MAIN, "Mode : Operation Mode");
        //wifi_start_sta(cJSON_GetObjectItem(config_1, "ssid")->valuestring,cJSON_GetObjectItem(config_1, "password")->valuestring);
        break;
    default:
        break;
    }
}
