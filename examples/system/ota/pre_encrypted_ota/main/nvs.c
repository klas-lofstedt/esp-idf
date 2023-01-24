// NVS
#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include <string.h>
#include "wifi.h"

// NVS storage
#define WIFI_IS_PROVISIONED     1
#define WIFI_NOT_PROVISIONED    0
#define NVS_APP_NAMESPACE       "storage"
#define NVS_WIFI_SSID_KEY       "wifi_ssid"
#define NVS_WIFI_PWD_KEY        "wifi_pwd"
#define NVS_WIFI_PROVISION_KEY  "wifi_is_prov"

static const char *TAG = "NVS";


bool nvs_wifi_is_provisioned(void)
{
    // Create nvs handle
    nvs_handle_t handle_wifi;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_wifi));
    // We only erase the default production app (OTA won't have this for persistance)
    int32_t is_provisioned = -1;
    ESP_ERROR_CHECK(nvs_get_i32(handle_wifi, NVS_WIFI_PROVISION_KEY, &is_provisioned));
    // Close
    nvs_close(handle_wifi);

    ESP_LOGI(TAG, "NVS is_provisioned: %ld", is_provisioned);

    if (is_provisioned == WIFI_IS_PROVISIONED){
        return true;
    }
    return false;
}

bool nvs_wifi_set_pwd(const char* str)
{
    ESP_LOGI(TAG, "NVS wifi pwd input: %s", str);

    // Create nvs handle
    nvs_handle_t handle_wifi;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_wifi));
    // Create data to store
    //const char* str = "hejhej test klas";
    ESP_ERROR_CHECK(nvs_set_str(handle_wifi, NVS_WIFI_PWD_KEY, str));
    // Commit data to nvs
    ESP_ERROR_CHECK(nvs_commit(handle_wifi));
    // Read nvs back into char buffer
    char buf[WIFI_PWD_MAX_LEN + 1];
    size_t buf_len = sizeof(buf);
    ESP_ERROR_CHECK(nvs_get_str(handle_wifi, NVS_WIFI_PWD_KEY, buf, &buf_len));

    ESP_LOGI(TAG, "NVS wifi pwd read: %s", buf);

    if (!strcmp(str, buf)){
        ESP_ERROR_CHECK(nvs_set_i32(handle_wifi, NVS_WIFI_PROVISION_KEY, WIFI_IS_PROVISIONED));
        // Commit data to nvs
        ESP_ERROR_CHECK(nvs_commit(handle_wifi));
        // Read nvs back
        int32_t is_provisioned = -1;
        ESP_ERROR_CHECK(nvs_get_i32(handle_wifi, NVS_WIFI_PROVISION_KEY, &is_provisioned));

        // Close
        nvs_close(handle_wifi);
        return is_provisioned;
    }

    // Close
    nvs_close(handle_wifi);
    return false;
}

bool nvs_wifi_get_pwd(uint8_t *buffer)
{
    // Create nvs handle
    nvs_handle_t handle_wifi;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_wifi));
    // Read nvs back into char buffer
    char char_buffer[WIFI_PWD_MAX_LEN];
    size_t char_buffer_len = sizeof(char_buffer);
    ESP_ERROR_CHECK(nvs_get_str(handle_wifi, NVS_WIFI_PWD_KEY, char_buffer, &char_buffer_len));

    memcpy(buffer, char_buffer, WIFI_PWD_MAX_LEN);
    ESP_LOGI(TAG, "NVS wifi pwd read: %s", buffer);

    nvs_close(handle_wifi);
    return true;// TODO
}


bool nvs_wifi_set_default(void)
{
    // Create nvs handle
    nvs_handle_t handle_wifi;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_wifi));
    // We only erase the default production app (OTA won't have this for persistance)
    ESP_ERROR_CHECK(nvs_erase_all(handle_wifi));
    // Set key is_provisioned = 0 (not provisioned)
    ESP_ERROR_CHECK(nvs_set_i32(handle_wifi, NVS_WIFI_PROVISION_KEY, WIFI_NOT_PROVISIONED));
    // Commit data to nvs
    ESP_ERROR_CHECK(nvs_commit(handle_wifi));
    // Read nvs back
    int32_t is_provisioned = -1;
    ESP_ERROR_CHECK(nvs_get_i32(handle_wifi, NVS_WIFI_PROVISION_KEY, &is_provisioned));
    // Close
    nvs_close(handle_wifi);

    ESP_LOGI(TAG, "NVS set provisioned: %ld", is_provisioned);

    if (is_provisioned == WIFI_NOT_PROVISIONED){
        return true;
    }
    return false;
}

void nvs_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}
