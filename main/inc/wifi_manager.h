#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "commun_lib.h"
#include "config_mode.h"
#include "mqtt_manager.h"
#include "esp_wifi.h"


extern httpd_handle_t server ;
#define MAX_AP_COUNT 20
#define DEFAULT_SCAN_LIST_SIZE 20
extern uint8_t status;
static const char *TAG_AP_WIFI = "AP_WIFI";

#define WIFI_SSID_AP       "esp32_ap"
#define WIFI_PASS_AP       "123456789"
#define MAX_STA_CONN       4



// Define MACSTR and MAC2STR
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2STR(mac) (mac)[0], (mac)[1], (mac)[2], (mac)[3], (mac)[4], (mac)[5]

extern esp_mqtt_client_handle_t client;


static const char *TAG_WIFI = "wifi_manager";

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_STACONNECTED: {
                wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
                ESP_LOGI(TAG_WIFI, "Station " MACSTR " join, AID=%d",
                         MAC2STR(event->mac), event->aid);
                         if(server==NULL){
                            server = start_webserver();
                         } 
                break;
            }
            case WIFI_EVENT_AP_STADISCONNECTED: {
                wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
                ESP_LOGI(TAG_WIFI, "Station " MACSTR " leave, AID=%d",
                         MAC2STR(event->mac), event->aid);
                //stop_webserver(server);
                break;
            }
            case WIFI_EVENT_STA_START: {
                ESP_LOGI(TAG_WIFI, "STA_START");
                esp_err_t err =esp_wifi_connect();
                break;
            }
            case WIFI_EVENT_STA_DISCONNECTED: {
                
                ESP_LOGI(TAG_WIFI, "Retry");
                esp_wifi_connect();
                    if (client != NULL && status==1) {
                        ESP_LOGI(TAG_WIFI, "WiFi disconnected, stopping MQTT client");
                        esp_mqtt_client_stop(client);
                        esp_mqtt_client_disconnect(client);
                        esp_mqtt_client_unregister_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler);
                        client = NULL;
                    }
                break;
            }
            case WIFI_EVENT_STA_CONNECTED: {
                    ESP_LOGI(TAG_WIFI, "STA_CONNECTED");
                    if (status==1) {
                        vTaskDelay(100);
                    mqtt_app_start(&client);
                    esp_err_t errr = esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
                        if (errr != ESP_OK) {
                            ESP_LOGE(TAG_WIFI, "Failed to register MQTT event handler: %s", esp_err_to_name(errr));
                        }

                    }
                break;
            }
            default:
                break;
        }
    }
}
/// @brief 
/// @return 
cJSON* wifi_scan_ap()
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "state", 1);
    cJSON *ap_array = cJSON_CreateArray();
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));

    cJSON *ap_item1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(ap_item1, "n_station", ap_count);
      cJSON_AddItemToArray(ap_array, ap_item1);
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        cJSON *ap_item = cJSON_CreateObject();
        cJSON_AddStringToObject(ap_item, "ssid", (const char*)ap_info[i].ssid);
        cJSON_AddNumberToObject(ap_item, "rssi", (double)ap_info[i].rssi);
        if (ap_info[i].rssi > -30) {
        cJSON_AddStringToObject(ap_item, "state_wifi", "Good");
        } else if (ap_info[i].rssi  >-70)  {
        cJSON_AddStringToObject(ap_item, "state_wifi", "Not bad");
        }else{
        cJSON_AddStringToObject(ap_item, "state_wifi", "bad"); 
        }
        if (ap_info[i].primary < 14) {
        cJSON_AddStringToObject(ap_item, "Band", "2.4GHz");
        } else {
        cJSON_AddStringToObject(ap_item, "Band", "5GHz");
        }
     cJSON_AddItemToArray(ap_array, ap_item);
    }
    cJSON_AddItemToObject(root, "wifi_networks", ap_array);
    ESP_ERROR_CHECK(esp_wifi_scan_stop());
    ESP_ERROR_CHECK(esp_wifi_clear_ap_list());
    return root;
}
void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
}
void wifi_start_ap(void)
{
    esp_netif_create_default_wifi_ap();
 esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID_AP,
            .ssid_len = strlen(WIFI_SSID_AP),
            .password = WIFI_PASS_AP,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(WIFI_PASS_AP) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }


    // STA configuration
    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = "",
            .password = "",
            .threshold.authmode = WIFI_AUTH_WPA2_WPA3_PSK,
           .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    // Configure the WiFi interfaces
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));

    ESP_ERROR_CHECK(esp_wifi_start());

   ESP_LOGI(TAG_WIFI, "AP mode started. AP SSID:%s password:%s",
             wifi_config.ap.ssid, wifi_config.ap.password);

}
void wifi_stop_ap(void)
{
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_LOGI(TAG_WIFI, "AP mode stopped.");
}
void wifi_start_sta(char *_ssid,char *_password)
{
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            //.ssid = _ssid,
            //.password = _password
        },
    };
   strlcpy((char *)wifi_config.sta.ssid, _ssid, sizeof(wifi_config.sta.ssid));
   strlcpy((char *)wifi_config.sta.password, _password, sizeof(wifi_config.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG_WIFI, "STA mode started. SSID:%s password:%s",
             (char *)wifi_config.sta.ssid,(char *)wifi_config.sta.password);
}
void wifi_stop_sta(void)
{
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_LOGI(TAG_WIFI, "STA mode stopped.");
}
esp_err_t setup_wifi_connect(char* _ssid,char* _password)
{
   /* Set WiFi configuration */
   wifi_config_t wifi_config = {
       .sta = {

           .bssid_set = false
       },
   };
   strlcpy((char *)wifi_config.sta.ssid, _ssid, sizeof(wifi_config.sta.ssid));
   strlcpy((char *)wifi_config.sta.password, _password, sizeof(wifi_config.sta.password));
   ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  // ESP_ERROR_CHECK(esp_wifi_start());
   return esp_wifi_connect();

}
char* get_ap_info() {
    wifi_ap_record_t ap_info;
   // memset(ap_info, 0, sizeof(ap_info));
    esp_err_t err=esp_wifi_sta_get_ap_info(&ap_info);
    if (err== ESP_OK) {
 size_t max_buffer_size =200;  // Adjust as needed
        char* info_buffer = (char*)malloc(max_buffer_size);

        if (info_buffer != NULL) {
            // Use snprintf to format information into the buffer
            int offset = 0;
            offset += snprintf(info_buffer + offset, max_buffer_size - offset, "      Connected AP Info:\n");
            offset += snprintf(info_buffer + offset, max_buffer_size - offset, "SSID: %s\n", ap_info.ssid);
            offset += snprintf(info_buffer + offset, max_buffer_size - offset, "MAC: %0X:%0X:%0X:%0X:%0X:%0X\n",ap_info.bssid[0], ap_info.bssid[1], ap_info.bssid[2], ap_info.bssid[3],ap_info.bssid[4], ap_info.bssid[5]);
            offset += snprintf(info_buffer + offset, max_buffer_size - offset, "Signal: %d -dbm\n", ap_info.rssi);
            offset += snprintf(info_buffer + offset, max_buffer_size - offset, "Primary: %d\n ", ap_info.primary);      
            //offset += snprintf(info_buffer + offset, max_buffer_size - offset, "authmode: %d\n", ap_info.authmode);

            return info_buffer;
        } else {
            ESP_LOGE(TAG_AP_WIFI, "Memory allocation failed");
            return "";
        }
    }else {
       ESP_LOGE(TAG_AP_WIFI, "Failed to get AP info");
        return "";
    }
    
}
void clear_sta_config() {
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
}               
void disconnect_ST(){
      esp_wifi_disconnect();
}
#endif /*WIFI_MANAGER_H*/