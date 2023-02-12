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


// WiFi
#define EXAMPLE_ESP_WIFI_SSID       "WiFi-113E7F"
#define EXAMPLE_ESP_WIFI_PASS       "54285231"
#define EXAMPLE_ESP_MAXIMUM_RETRY   10 // TODO


static const char *TAG = "WIFI";

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);
/* FreeRTOS event group to signal when we are connected*/
//static EventGroupHandle_t s_wifi_event_group;
//static SemaphoreHandle_t wifi_connect_lock;


/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
// #define WIFI_CONNECTED_BIT BIT0
// #define WIFI_FAIL_BIT      BIT1
// #define WIFI_SCAN_DONE_BIT BIT2

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    ESP_LOGI(TAG,"DEBUG EVENT!!!!!! %ld", event_id);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Connect WiFi...");
        //if (xSemaphoreTake(wifi_connect_lock, 0) == pdTRUE){
        //esp_wifi_connect();
        //} else {
        //    ESP_LOGI(TAG, "Failed to connect WiFi... semaphore already taken 1");
        //}
        xEventGroupSetBits(app_event_group, APP_EVENT_WIFI_START);

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        //ESP_LOGI(TAG, "Reconnecting WiFi...");
        //xSemaphoreGive(wifi_connect_lock);
        //if (xSemaphoreTake(wifi_connect_lock, 0) == pdTRUE){
        //esp_wifi_connect();
        //} else {
        //    ESP_LOGI(TAG, "Failed to reconnect WiFi... semaphore already taken 2");
        //}
        xEventGroupSetBits(app_event_group, APP_EVENT_WIFI_DISCONNECTED);

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        //xSemaphoreGive(wifi_connect_lock);
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(app_event_group, APP_EVENT_WIFI_CONNECTED);

        //xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE){
        //xEventGroupSetBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT);
        xEventGroupSetBits(app_event_group, APP_EVENT_WIFI_SCAN_DONE);
    }
}

void wifi_start_scan(void)
{
    esp_wifi_disconnect();
    wifi_scan_config_t scan_config = {
        .ssid = 0,
        .bssid = 0,
        .channel = 0,
        .show_hidden = true
    };

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
}

void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
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
    // esp_wifi_disconnect();
    // esp_wifi_stop();
    // esp_wifi_deinit();
    // wifi_init();
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
    //ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_connect();
}

void wifi_connect(void)
{
    esp_wifi_connect();
    // uint8_t wifi_ssid[WIFI_SSID_MAX_LEN];
    // uint8_t wifi_pwd[WIFI_PWD_MAX_LEN];
    // if (nvs_wifi_is_provisioned()){
    //     nvs_wifi_get_ssid(wifi_ssid);
    //     nvs_wifi_get_pwd(wifi_pwd);

    //     wifi_config_t wifi_config = {
    //         .sta = {
    //             .ssid = "",
    //             .password = "",
    //             /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
    //             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
    //             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
    //             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
    //             */
    //             .threshold.authmode = WIFI_AUTH_OPEN,
    //             .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
    //         },
    //     };

    //     memcpy(wifi_config.sta.ssid, wifi_ssid, WIFI_SSID_MAX_LEN);
    //     memcpy(wifi_config.sta.password, wifi_pwd, WIFI_PWD_MAX_LEN);
    //     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    // }

    // ESP_ERROR_CHECK(esp_wifi_start()); // TODO not sure if esp_wifi_start() or esp_wifi_connect();
}

void wifi_connect_propeller(void)
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
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

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "klaswifi",
            .password = "klasklasklas",
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
            * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
            * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
            * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
            */
            .threshold.authmode = WIFI_AUTH_OPEN,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));


    ESP_ERROR_CHECK(esp_wifi_start());
}

