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
#include "misc.h"
#include "aes.h"
#include "ble.h"

// NVS storage
#define WIFI_IS_PROVISIONED     1
#define WIFI_NOT_PROVISIONED    0

#define NVS_APP_NAMESPACE       "storage"

#define NVS_WIFI_SSID_KEY       "wifi_ssid"
#define NVS_WIFI_PWD_KEY        "wifi_pwd"
#define NVS_WIFI_PROVISION_KEY  "wifi_is_prov"
#define NVS_BLE_POP_KEY         "ble_pop"
#define NVS_AES_KEY             "aes_key"
#define NVS_DEVICE_TYPE         "device_type"

static const char *TAG = "NVS";

bool nvs_set_device_type(uint32_t device_type)
{
    ESP_LOGI(TAG, "NVS device type: %ld", device_type);

    // Create nvs handle
    nvs_handle_t handle_device_type;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_device_type));
    // Create data to store
    ESP_ERROR_CHECK(nvs_set_u32(handle_device_type, NVS_DEVICE_TYPE, device_type));
    // Commit data to nvs
    ESP_ERROR_CHECK(nvs_commit(handle_device_type));
    // Read nvs back into char buffer
    uint32_t is_device_type = 0;
    ESP_ERROR_CHECK(nvs_get_u32(handle_device_type, NVS_DEVICE_TYPE, &is_device_type));

    nvs_close(handle_device_type);

    if (device_type == is_device_type){
        ESP_LOGI(TAG, "NVS device type OK");
        return true;
    }
    ESP_LOGI(TAG, "NVS device type failed");
    return false;
}

uint32_t nvs_get_device_type(void)
{
    // Create nvs handle
    nvs_handle_t handle_device_type;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_device_type));
    // We only erase the default production app (OTA won't have this for persistance)
    uint32_t device_type = 0;
    ESP_ERROR_CHECK(nvs_get_u32(handle_device_type, NVS_DEVICE_TYPE, &device_type));
    // Close
    nvs_close(handle_device_type);

    return device_type;
}

void nvs_set_ble_pop(void)
{
    uint8_t pop[BLE_POP_SIZE];
    generate_random_u8_array(pop, BLE_POP_SIZE);
    ESP_LOGI(TAG, "Set BLE pop in NVS");
    ESP_LOG_BUFFER_HEX(TAG, pop, 16);

    // Create nvs handle
    nvs_handle_t handle_ble_pop;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_ble_pop));
    // Create data to store
    size_t len = BLE_POP_SIZE;
    ESP_ERROR_CHECK(nvs_set_blob(handle_ble_pop, NVS_BLE_POP_KEY, pop, len));
    // Commit data to nvs
    ESP_ERROR_CHECK(nvs_commit(handle_ble_pop));
    // Close
    nvs_close(handle_ble_pop);
}

void nvs_get_ble_pop_arr(uint8_t *pop)
{
    ESP_LOGI(TAG, "Get BLE pop in NVS");
    // Create nvs handle
    nvs_handle_t handle_ble_pop;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_ble_pop));
    // Create data to store
    size_t len = BLE_POP_SIZE;
    ESP_ERROR_CHECK(nvs_get_blob(handle_ble_pop, NVS_BLE_POP_KEY, pop, &len));
    // Close
    nvs_close(handle_ble_pop);
}

void nvs_get_ble_pop_str(char *pop)
{
    ESP_LOGI(TAG, "Get BLE pop in NVS");
    uint8_t temp[BLE_POP_SIZE];
    // Create nvs handle
    nvs_handle_t handle_ble_pop;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_ble_pop));
    // Create data to store
    size_t len = BLE_POP_SIZE;
    ESP_ERROR_CHECK(nvs_get_blob(handle_ble_pop, NVS_BLE_POP_KEY, temp, &len));
    hex_string_from_u8_array(temp, pop, BLE_POP_SIZE);
    // Close
    nvs_close(handle_ble_pop);
}

void nvs_set_aes_key(void)
{
    uint8_t aes[AES_KEY_SIZE];
    generate_random_u8_array(aes, AES_KEY_SIZE);
    ESP_LOGI(TAG, "Set AES key in NVS");
    ESP_LOG_BUFFER_HEX(TAG, aes, 16);
    // Create nvs handle
    nvs_handle_t handle_aes_key;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_aes_key));
    // Create data to store
    size_t len = AES_KEY_SIZE;
    ESP_ERROR_CHECK(nvs_set_blob(handle_aes_key, NVS_AES_KEY, aes, len));
    // Commit data to nvs
    ESP_ERROR_CHECK(nvs_commit(handle_aes_key));
    // Close
    nvs_close(handle_aes_key);
}

void nvs_get_aes_key_arr(uint8_t *aes)
{
    ESP_LOGI(TAG, "Get AES key in NVS");
    // Create nvs handle
    nvs_handle_t handle_aes_key;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_aes_key));
    // Create data to store
    size_t len = AES_KEY_SIZE;
    ESP_ERROR_CHECK(nvs_get_blob(handle_aes_key, NVS_AES_KEY, aes, &len));
    // Close
    nvs_close(handle_aes_key);
}

void nvs_get_aes_key_str(char *aes)
{
    ESP_LOGI(TAG, "Get AES key in NVS");
    uint8_t temp[AES_KEY_SIZE];
    // Create nvs handle
    nvs_handle_t handle_aes_key;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_aes_key));
    // Create data to store
    size_t len = AES_KEY_SIZE;
    ESP_ERROR_CHECK(nvs_get_blob(handle_aes_key, NVS_AES_KEY, temp, &len));
    hex_string_from_u8_array(temp, aes, AES_KEY_SIZE);
    // Close
    nvs_close(handle_aes_key);
}

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
    ESP_LOGI(TAG, "NVS wifi set pwd input: %s", str);

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

    ESP_LOGI(TAG, "NVS wifi set pwd read: %s", buf);

    nvs_close(handle_wifi);

    if (strcmp(str, buf) == 0){
        ESP_LOGI(TAG, "NVS wifi password OK");
        return true;
    }
    ESP_LOGI(TAG, "NVS wifi password failed");
    return false;
}

bool nvs_is_production_done(void)
{
    return true;
}

bool nvs_is_provision_done(void)
{
    return true;
}

bool nvs_wifi_set_provisioned(bool set_provision)
{
    ESP_LOGI(TAG, "NVS wifi provisioned input: %d", set_provision);

    // Create nvs handle
    nvs_handle_t handle_wifi;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_wifi));
    // Create data to store
    //const char* str = "hejhej test klas";
    ESP_ERROR_CHECK(nvs_set_i32(handle_wifi, NVS_WIFI_PROVISION_KEY, (int32_t)set_provision));
    // Commit data to nvs
    ESP_ERROR_CHECK(nvs_commit(handle_wifi));
    // Read nvs back into char buffer
    int32_t is_provisioned = -1;
    ESP_ERROR_CHECK(nvs_get_i32(handle_wifi, NVS_WIFI_PROVISION_KEY, &is_provisioned));

    nvs_close(handle_wifi);

    if ((int32_t)set_provision == is_provisioned){
        ESP_LOGI(TAG, "NVS wifi provision OK");
        return true;
    }
    ESP_LOGI(TAG, "NVS wifi provision failed");
    return false;
}

bool nvs_wifi_set_ssid(const char* str)
{
    ESP_LOGI(TAG, "NVS wifi ssid input: %s", str);

    // Create nvs handle
    nvs_handle_t handle_wifi;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_wifi));
    // Create data to store
    //const char* str = "hejhej test klas";
    ESP_ERROR_CHECK(nvs_set_str(handle_wifi, NVS_WIFI_SSID_KEY, str));
    // Commit data to nvs
    ESP_ERROR_CHECK(nvs_commit(handle_wifi));
    // Read nvs back into char buffer
    char buf[WIFI_SSID_MAX_LEN + 1];
    size_t buf_len = sizeof(buf);
    ESP_ERROR_CHECK(nvs_get_str(handle_wifi, NVS_WIFI_SSID_KEY, buf, &buf_len));

    ESP_LOGI(TAG, "NVS wifi ssid read: %s", buf);

    nvs_close(handle_wifi);

    if (strcmp(str, buf) == 0){
        ESP_LOGI(TAG, "NVS wifi ssid OK");
        return true;
    }
    ESP_LOGI(TAG, "NVS wifi ssid failed");
    return false;
}

bool nvs_wifi_get_ssid(uint8_t *buffer)
{
    // Create nvs handle
    nvs_handle_t handle_wifi;
    // Open nvs
    ESP_ERROR_CHECK(nvs_open(NVS_APP_NAMESPACE, NVS_READWRITE, &handle_wifi));
    // Read nvs back into char buffer
    char char_buffer[WIFI_SSID_MAX_LEN];
    size_t char_buffer_len = sizeof(char_buffer);
    ESP_ERROR_CHECK(nvs_get_str(handle_wifi, NVS_WIFI_SSID_KEY, char_buffer, &char_buffer_len));

    memcpy(buffer, char_buffer, WIFI_SSID_MAX_LEN);
    ESP_LOGI(TAG, "NVS wifi ssid read: %s", buffer);

    nvs_close(handle_wifi);
    return true;// TODO
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
    ESP_LOGI(TAG, "NVS wifi get pwd read: %s", buffer);

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
