


#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include "Commun_lib.h"
#include "mqtt_client.h"




#define MQTT_BROKER_URI "mqtt://51.75.141.46:1883"
#define MQTT_BROKER_USERNAME "mounir"
#define MQTT_BROKER_PASSWORD "mounirsfm"
// Global MQTT client handle
extern esp_mqtt_client_handle_t client;

static const char *TAG_MQTT = "MQTT";
#define MQTT_RECONNECT_DELAY pdMS_TO_TICKS(5000)

#define TIMER_PERIOD pdMS_TO_TICKS(5000) // 1 minute in ticks



void mqtt_app_start(esp_mqtt_client_handle_t *client) {
    ESP_LOGI(TAG_MQTT, "mqtt_app_start");
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.username=MQTT_BROKER_USERNAME,
        .credentials.authentication.password=MQTT_BROKER_PASSWORD,
    };

    *client = esp_mqtt_client_init(&mqtt_cfg);
   
    esp_mqtt_client_start(*client);
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
        char *json_str = "massage";   

        esp_mqtt_client_publish(client, "droppy", json_str, 0, 1, 0);
        free(json_str);

                    break;
            case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");

                    vTaskDelay(MQTT_RECONNECT_DELAY);
                    esp_mqtt_client_reconnect(event->client); 
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
            break;
    }
}



#endif /* MQTT_MANAGER_H */
