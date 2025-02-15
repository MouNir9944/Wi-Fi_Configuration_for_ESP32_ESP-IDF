#ifndef CONFIG_MODE_H
#define CONFIG_MODE_H

#include "commun_lib.h"
#include "wifi_manager.h"
#include "nvs_manager.h"
#include "Setup_mode_button.h"
//#include "protocol_examples_common.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_http_server.h"
#include "spiffs_manager.h"

TaskHandle_t ir_rx_task_handle=NULL;
static const char *TAG_server = "server";
extern httpd_handle_t server ;
cJSON* wifi_scan_ap();
esp_err_t setup_wifi_connect(char* _ssid,char* _password);
char* get_ap_info();
void clear_sta_config();
void disconnect_ST();
void delete_files(const char* path);
static esp_err_t Config_get_handler(httpd_req_t *req)
{
    FILE* file = fopen("/spiffs/Configurationpage.html", "r");
    if (file == NULL) {
        ESP_LOGE("SPIFFS", "Failed to open file : %s", "/spiffs/Configurationpage.html");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        httpd_resp_send_chunk(req, line, HTTPD_RESP_USE_STRLEN);
    }
    httpd_resp_send_chunk(req, NULL, 0); // Indicate end of response
    fclose(file);
    return ESP_OK;
}

static httpd_uri_t Config_page = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = Config_get_handler,
    .user_ctx  = NULL
};


void send_websocket_message(httpd_req_t *req, const char *message) {
    // Create a WebSocket frame for the response
    httpd_ws_frame_t ws_response = {
        .payload =message,
        .len = strlen(message),
        .type = HTTPD_WS_TYPE_TEXT
    };

    // Send the message back to the client
    httpd_ws_send_frame(req, &ws_response);
}
static void websocket_callback(httpd_handle_t hd, httpd_req_t *req, httpd_ws_frame_t *ws_pkt)
{
    if (ws_pkt->type == HTTPD_WS_TYPE_TEXT) {
    char *buffer = malloc(ws_pkt->len + 1);
            if (buffer == NULL) {
                ESP_LOGE(TAG_server, "Failed to allocate memory for WebSocket message");
                return;
            }
            
            // Copy the payload into the buffer and null-terminate it
            memcpy(buffer, ws_pkt->payload, ws_pkt->len);
            buffer[ws_pkt->len] = '\0'; // Null-terminate the string

            // Log the received message
            ESP_LOGI(TAG_server, "Received WebSocket message: %s", buffer);
    

        // Check if buffer contains valid data
        if (buffer[0] != '\0') {
            // Example: Send a new message back to the client
            cJSON *root = cJSON_Parse((char*)buffer);
            char *path = malloc(23); // Example allocation, adjust as needed
            if (root != NULL) {   
            int state=cJSON_GetObjectItem(root, "state")->valueint;
                if(state==0 )
                {
                    cJSON_Delete(root);     
                    const char *new_message=read_data("/spiffs/Config.txt");

                    send_websocket_message(req, new_message);
                }
                else if(state==1)
                {
                    cJSON_Delete(root);
                    const char *new_message=cJSON_Print(wifi_scan_ap());
                    send_websocket_message(req, new_message);
                }
                else if(state==2)
                {
                    esp_err_t err=setup_wifi_connect(cJSON_GetObjectItem(root, "ssid")->valuestring,cJSON_GetObjectItem(root, "password")->valuestring);
                    vTaskDelay(1000);
                    cJSON_Delete(root);
                    cJSON *root_send = cJSON_CreateObject();
                    cJSON_AddNumberToObject(root_send, "state", 2);
                    if(err==ESP_OK){
                    cJSON_AddStringToObject(root_send, "Wifi_state", get_ap_info());
                    }else{
                    cJSON_AddStringToObject(root_send, "Wifi_state", "Wifi Not Connect");
                    }
                    const char *new_message=cJSON_Print(root_send);
                      disconnect_ST();
                      clear_sta_config();
                    cJSON_Delete(root_send);
                    send_websocket_message(req, new_message);
                }
                else if(state==3)
                {

                }else if(state==4)
                {

                }else if(state==5)
                {  
                    cJSON *root_send = cJSON_CreateObject();
                    cJSON_AddNumberToObject(root_send, "state", 5);
                    esp_err_t err= store_data("/spiffs/Config.txt",cJSON_Print(root));
                    if(err==ESP_OK){
                    cJSON_AddStringToObject(root_send, "store_data", "ok");
                    }else{
                    cJSON_AddStringToObject(root_send, "store_data", "notok");
                    }  
                    const char *new_message =cJSON_Print(root_send);
                    cJSON_Delete(root_send);
                    cJSON_Delete(root);  
                    send_websocket_message(req, new_message);
                } else if(state==6)
                {
                    cJSON_Delete(root);  
                    cJSON *root_send = cJSON_CreateObject();
                    cJSON_AddNumberToObject(root_send, "state", 6);
                    esp_err_t err= store_data("/spiffs/Config.txt","{}");
                     err= save_data_u32_in_flash(NVS_NAMESPACE, KEY_cons, 0);
                    if(err==ESP_OK){
                    cJSON_AddStringToObject(root_send, "ren_data", "ok");
                    }else{
                    cJSON_AddStringToObject(root_send, "ren_data", "notok");
                    }  
                    const char *new_message =cJSON_Print(root_send);
                    cJSON_Delete(root_send);
                    send_websocket_message(req, new_message);
                }else if(state==7)
                {

                    saved_data_in_flash(NVS_NAMESPACE, KEY_NAME,"1");
                    char *Gateway_mode = get_saved_data_from_flash(NVS_NAMESPACE,KEY_NAME); 
                    ESP_LOGI(TAG_server,"Mode!=%s", Gateway_mode );
                    vTaskDelay(1000);   
                    esp_restart();
                     
                }  
                else if(state==8)
                {
                 
                     char *path1=cJSON_GetObjectItem(root, "readpath")->valuestring;                   
                     snprintf(path, 23, path1); 
                     delete_files(path);
                     cJSON_Delete(root); 
                     //free(path1);
                }  
            }
        }
        free(buffer);
    }
    
}

static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG_server, "Handshake done, new connection opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t *)malloc(8192);
    httpd_ws_recv_frame(req, &ws_pkt, 8192);

    websocket_callback(req->handle, req, &ws_pkt);
    free(ws_pkt.payload);

    return ESP_OK;
}

static httpd_uri_t ws = {
    .uri       = "/ws",
    .method    = HTTP_GET,
    .handler   = ws_handler,
    .is_websocket = true
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG_server, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG_server, "Registering URI handlers");
        httpd_register_uri_handler(server, &Config_page);
        httpd_register_uri_handler(server, &ws);
        return server;
    }

    ESP_LOGI(TAG_server, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{

    esp_err_t err=httpd_stop(server);
    if(err==ESP_OK){
    ESP_LOGI(TAG_server, "Server Stop ");
    }else{
    ESP_LOGI(TAG_server, "error Stop Server!");
    }
}


#endif /* CONFIG_MODE_H */