/*
 * Copyright (c) 2024 <[SFM Technologies]>
 */

#ifndef SETUP_MODE_BUTTON_H
#define SETUP_MODE_BUTTON_H

#include "Commun_Lib.h"
#include "nvs_Manager.h"

#define setup_mode_button GPIO_NUM_0

#define NVS_NAMESPACE "storage"
#define KEY_NAME "mode"
#define Gateway_mode "CONFIG"



static const char *TAG_BUTTON = "Button mode";
static QueueHandle_t gpio_evt_queue = NULL;
void gpio_button_mode_init(gpio_num_t gpio_switchmode);
bool state_mode(gpio_num_t gpio_num);
static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}
static void gpio_task(void* arg) {
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            //saved_data_in_flash(NVS_NAMESPACE, KEY_NAME,Gateway_mode);
            save_data_u32_in_flash(NVS_NAMESPACE, Gateway_mode, 0);
            if (io_num == setup_mode_button) {

                save_data_u32_in_flash(NVS_NAMESPACE, Gateway_mode, 0);
                ESP_LOGI(TAG_BUTTON, "Restarting ESP32...");
                esp_restart();
            }
        }
    }
}/*
static void gpio_task(void* arg) {
    uint32_t io_num;
    TickType_t press_start_time = 0;
    const TickType_t timeout_5s = 5000 / portTICK_PERIOD_MS;  // 5 seconds
    const TickType_t timeout_10s = 10000 / portTICK_PERIOD_MS; // 10 seconds

    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            if (io_num == setup_mode_button) {
                ESP_LOGI(TAG_BUTTON, "Setup button pressed, waiting for timeout...");

                // Record the start time of the press
                press_start_time = xTaskGetTickCount();

                // Loop to monitor button press duration
                while (gpio_get_level(setup_mode_button) == 0) { // Button is held down
                    TickType_t elapsed_time = xTaskGetTickCount() - press_start_time;

                    // Check for 5-second timeout
                    if (elapsed_time >= timeout_5s && elapsed_time < timeout_10s) {
                        ESP_LOGI(TAG_BUTTON, "Button pressed for 5 seconds, saving 1 to NVS.");
                        save_data_u32_in_flash(NVS_NAMESPACE, Gateway_mode, 1);
                    }

                    // Check for 10-second timeout
                    if (elapsed_time >= timeout_10s) {
                        ESP_LOGI(TAG_BUTTON, "Button pressed for 10 seconds, saving 0 to NVS.");
                        save_data_u32_in_flash(NVS_NAMESPACE, Gateway_mode, 0);
                        break;
                    }

                    // Add a small delay to avoid blocking the task
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }

                // Restart ESP if button is released after any timeout
                ESP_LOGI(TAG_BUTTON, "Restarting ESP32...");
                esp_restart();
            }
        }
    }
}
*/
bool state_mode(gpio_num_t _gpio_num)
{
return gpio_get_level(_gpio_num);
}
void gpio_button_mode_init(gpio_num_t gpio_switchmode)
{
    // Configuration du GPIO
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_POSEDGE;  // Interruption sur front montant
    io_conf.mode = GPIO_MODE_INPUT;         // Mode entrée
    io_conf.pin_bit_mask = (1ULL << gpio_switchmode);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;                 // Activation du pull-up interne
    gpio_config(&io_conf);

    // Créer une file d'attente pour les événements GPIO
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    // Créer une tâche pour gérer les interruptions GPIO
    xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);

    // Installer le gestionnaire d'interruption
    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio_switchmode, gpio_isr_handler, (void*) gpio_switchmode);

    ESP_LOGI(TAG_BUTTON, "GPIO interrupt example initialized.");
}

#endif /*  SETUP_MODE_BUTTON_H */