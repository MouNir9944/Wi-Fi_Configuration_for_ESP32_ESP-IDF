#ifndef NVS_CONFIG_H
#define NVS_CONFIG_H
#include "commun_lib.h"
#include "nvs_flash.h"
#include "nvs.h"

esp_err_t init_nvs(){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    return ret;
}
char* get_saved_data_from_flash(const char* namespace, const char* key) {
          
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        printf("Error (%s) nvs_flash_init!\n", esp_err_to_name( ret));
        ret = nvs_flash_init();
    }
        nvs_handle_t nvs_handle;

    // Open NVS
    esp_err_t err = nvs_open(namespace, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
         printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        // saved_data_in_flash(namespace,key, "") ;
        return NULL;
    }

    // Read data from NVS
    char* value = NULL;
    size_t required_size;

    err = nvs_get_str(nvs_handle, key, NULL, &required_size);
    if (err == ESP_OK) {
        value = malloc(required_size);
        if (value) {
            err = nvs_get_str(nvs_handle, key, value, &required_size);
            if (err == ESP_OK) {
                printf("Value for key '%s' in namespace '%s': %s\n", key, namespace, value);
            } else {
                printf("Error (%s) reading from NVS!\n", esp_err_to_name(err));
                free(value);
               return NULL;
            }
        } else {
            printf("Error allocating memory!\n");
        }
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        printf("Key '%s' not found in namespace '%s'!\n", key, namespace);
         return NULL;
    } else {
        printf("Error (%s) reading from NVS!\n", esp_err_to_name(err));
          return NULL;
    }

    // Close NVS
    nvs_close(nvs_handle);

    return value;
}
esp_err_t saved_data_in_flash(const char* namespace, const char* key, const char* value) {
      
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        printf("Error (%s) nvs_flash_init!\n", esp_err_to_name( ret));
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    printf("nvs_flash_init= (%s)\n", esp_err_to_name( ret));
    nvs_handle_t nvs_handle;

    // Open NVS
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    
    }

    // Write data to NVS
    err = nvs_set_str(nvs_handle, key, value);
    if (err != ESP_OK) {
        printf("Error (%s) writing to NVS!\n", esp_err_to_name(err));
    }

    // Commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        printf("Error (%s) committing changes to NVS!\n", esp_err_to_name(err));
    }

    // Close NVS
    nvs_close(nvs_handle);
    return err;
}
uint32_t get_saved_data_32_from_flash(const char* namespace, const char* key) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        printf("Error (%s) nvs_flash_init!\n", esp_err_to_name(ret));
        return 0;  // Return 0 in case of error, can be handled differently
    }

    nvs_handle_t nvs_handle;

    // Open NVS
    esp_err_t err = nvs_open(namespace, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return 0;  // Return 0 in case of error, can be handled differently
    }

    // Read data from NVS
    uint32_t value = 0;
    err = nvs_get_u32(nvs_handle, key, &value);
    if (err == ESP_OK) {
        printf("Value for key '%s' in namespace '%s': %lu\n", key, namespace, value);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        printf("Key '%s' not found in namespace '%s'!\n", key, namespace);
        value = 0;  // Key not found, return 0
    } else {
        printf("Error (%s) reading from NVS!\n", esp_err_to_name(err));
        value = 0;  // Return 0 in case of error
    }

    // Close NVS
    nvs_close(nvs_handle);

    return value;
}
esp_err_t save_data_u32_in_flash(const char* namespace, const char* key, uint32_t value) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        printf("Error (%s) nvs_flash_init! Erasing flash and reinitializing.\n", esp_err_to_name(ret));
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        printf("Error (%s) nvs_flash_init failed!\n", esp_err_to_name(ret));
        return ret;
    }

    nvs_handle_t nvs_handle;
    // Open NVS
    esp_err_t err = nvs_open(namespace, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return err;
    }

    // Write data to NVS
    err = nvs_set_u32(nvs_handle, key, value);
    if (err != ESP_OK) {
        printf("Error (%s) writing to NVS!\n", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        printf("Error (%s) committing changes to NVS!\n", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Close NVS
    nvs_close(nvs_handle);
    return ESP_OK;
}

#endif /* NVS_CONFIG_H */