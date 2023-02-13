#include "lwip/err.h"
#include "lwip/sys.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "wifi.h"
#include "nvs.h"
#include "app.h"
#include <cJSON.h>

static const char *TAG = "WIFI";

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);
static bool wifi_take_lock(int timeout_ms);
static void wifi_give_lock(void);

static SemaphoreHandle_t wifi_connect_lock;
static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;
static esp_netif_t *wifi_sta = NULL;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT){
        ESP_LOGI(TAG, "DEBUG WIFI EVENT: %ld", event_id);
    }
    if (event_base == IP_EVENT){
        ESP_LOGI(TAG, "DEBUG IP EVENT: %ld", event_id);
    }
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "STA start...");
        xEventGroupSetBits(app_event_group, APP_EVENT_WIFI_START);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_give_lock();
        xEventGroupSetBits(app_event_group, APP_EVENT_WIFI_DISCONNECTED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_give_lock();
        xEventGroupSetBits(app_event_group, APP_EVENT_WIFI_CONNECTED);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        wifi_give_lock();
        xEventGroupSetBits(app_event_group, APP_EVENT_WIFI_CONNECTED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE){
        xEventGroupSetBits(app_event_group, APP_EVENT_WIFI_SCAN_DONE);
    }
}

static bool wifi_take_lock(int timeout_ms)
{
    if (!(xSemaphoreTake(wifi_connect_lock, timeout_ms / portTICK_PERIOD_MS) == pdTRUE)){
        ESP_LOGI(TAG, "Failed to take mutex ");
        return false;
    }
    ESP_LOGI(TAG, "Take mutex ");
    return true;
}

static void wifi_give_lock(void)
{
    xSemaphoreGive(wifi_connect_lock);
    ESP_LOGI(TAG, "Give mutex ");
}

bool wifi_start_scan(void)
{
    ESP_LOGI(TAG, "Trying to start scan");
    if (!wifi_take_lock(5000)){
        return false;
    }
    wifi_scan_config_t scan_config = {
        .ssid = 0,
        .bssid = 0,
        .channel = 0,
        .show_hidden = true
    };
    ESP_LOGI(TAG, "Starting scan...");
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    return true;
}

void wifi_stop_scan(void)
{
    esp_wifi_scan_stop();
    wifi_give_lock();
}


void wifi_init(void)
{
    wifi_connect_lock = xSemaphoreCreateBinary();
    wifi_give_lock();

    ESP_LOGI(TAG, "Init");

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_sta = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));


    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    uint8_t wifi_ssid[WIFI_SSID_MAX_LEN];
    uint8_t wifi_pwd[WIFI_PWD_MAX_LEN];
    if (nvs_wifi_is_provisioned()){
        nvs_wifi_get_ssid(wifi_ssid);
        nvs_wifi_get_pwd(wifi_pwd);

        wifi_config_t wifi_config = {
            .sta = {
                .ssid = "",
                .password = "",
                /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
                * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
                * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
                * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
                */
                .threshold.authmode = WIFI_AUTH_OPEN,
                .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            },
        };

        memcpy(wifi_config.sta.ssid, wifi_ssid, WIFI_SSID_MAX_LEN);
        memcpy(wifi_config.sta.password, wifi_pwd, WIFI_PWD_MAX_LEN);
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    }
    ESP_ERROR_CHECK(esp_wifi_start());
}

void wifi_reinit(void)
{
    ESP_LOGI(TAG, "Deinitialising");
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
    esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    esp_netif_destroy(esp_netif_get_handle_from_ifkey("WIFI_STA"));
    esp_event_loop_delete_default();
    esp_netif_deinit();
    esp_netif_destroy(wifi_sta);
    vSemaphoreDelete(wifi_connect_lock);

    ESP_LOGI(TAG, "Initialising");
    wifi_init();
}

bool wifi_connect(void)
{
    ESP_LOGI(TAG, "Trying to connect");
    if (!wifi_take_lock(0)){
        return false;
    }
    ESP_LOGI(TAG, "Connecting...");
    esp_wifi_connect();
    return true;
}
