#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

//#include <cJSON.h>

#include "ota.h"
#include "nvs.h"
#include "wifi.h"
#include "ble.h"
#include "app.h"
#include "misc.h"


static const char *TAG = "APP";

EventGroupHandle_t app_event_group;

static uint32_t awaiting_event(void);


static uint32_t awaiting_event(void)
{
    EventBits_t bits = xEventGroupWaitBits(app_event_group,
        APP_EVENT_BLE_GAP_CONNECT |
        APP_EVENT_BLE_GAP_ADV_COMPLETE |
        APP_EVENT_BLE_GAP_DISCONNECT |
        APP_EVENT_BLE_POP_DONE |
        APP_EVENT_BLE_CREDS_DONE |
        APP_EVENT_BLE_CERT_DONE |
        APP_EVENT_BLE_NOTIFY_SCAN_DONE |
        APP_EVENT_BLE_NOTIFY_STATUS_DONE |
        APP_EVENT_WIFI_STA_START |
        APP_EVENT_WIFI_STA_DISCONNECTED |
        APP_EVENT_WIFI_STA_CONNECTED |
        APP_EVENT_WIFI_SCAN_DONE,

        pdTRUE, pdFALSE, portMAX_DELAY);

    uint32_t event = (uint32_t)bits;

    return event;
}

void app_main(void)
{
    app_event_group = xEventGroupCreate();
    app_states_t app_state = APP_STATE_AWAITING_INIT;

    nvs_init();

    //nvs_wifi_set_default();

    //wifi_init();
    wifi_connect_propeller();

    //wifi_scan_networks();

    //ble_init();


    app_state = APP_STATE_NORMAL_OPERATION;

    // while (true){
    //     ESP_LOGI(TAG, "Waiting for events...");

    //     uint32_t app_event = awaiting_event();

    //     switch (app_event){
    //         case APP_EVENT_BLE_POP_DONE:
    //             if (app_state == APP_STATE_AWAITING_BLE_POP){
    //                 ESP_LOGI(TAG, "APP_EVENT_BLE_POP_DONE");
    //                 wifi_start_scan();
    //                 app_state = APP_STATE_AWAITING_SCAN_RESULTS;
    //             }
    //             break;

    //         case APP_EVENT_WIFI_SCAN_DONE:
    //             if (app_state == APP_STATE_AWAITING_SCAN_RESULTS){
    //                 ESP_LOGI(TAG, "APP_EVENT_WIFI_SCAN_DONE");
    //                 app_state = APP_STATE_AWAITING_BLE_CREDS;
    //                 ble_notify_scan(0);
    //             }
    //             break;

    //         case APP_EVENT_BLE_CREDS_DONE:
    //             if (app_state == APP_STATE_AWAITING_BLE_CREDS){
    //                 ESP_LOGI(TAG, "APP_EVENT_BLE_CREDS_DONE");
    //                 app_state = APP_STATE_AWAITING_BLE_CERT;
    //             }
    //             break;

    //         case APP_EVENT_BLE_CERT_DONE:
    //             if (app_state == APP_STATE_AWAITING_BLE_CERT){
    //                 ESP_LOGI(TAG, "APP_EVENT_BLE_CERT_DONE");
    //                 app_state = APP_STATE_AWAITING_WIFI;
    //                 wifi_connect();
    //             }
    //             break;

    //         case APP_EVENT_WIFI_STA_CONNECTED:
    //             app_state = APP_STATE_NORMAL_OPERATION;
    //             ESP_LOGI(TAG, "APP_EVENT_WIFI_STA_CONNECTED");
    //             break;

    //         case APP_EVENT_WIFI_STA_DISCONNECTED:
    //             if (app_state == APP_STATE_NORMAL_OPERATION){
    //                 ESP_LOGI(TAG, "APP_EVENT_WIFI_STA_DISCONNECTED");
    //                 wifi_connect();
    //             }
    //             if (app_state == APP_STATE_AWAITING_WIFI){
    //                 ESP_LOGI(TAG, "APP_EVENT_WIFI_STA_DISCONNECTED");
    //                 wifi_connect();
    //             }
    //             break;

    //         case APP_EVENT_BLE_GAP_CONNECT:
    //             ESP_LOGI(TAG, "APP_EVENT_BLE_GAP_CONNECT");
    //             if (app_state == APP_STATE_NORMAL_OPERATION){
    //                 app_state = APP_STATE_AWAITING_BLE_POP;
    //             }
    //             if (app_state == APP_STATE_AWAITING_WIFI){
    //                 app_state = APP_STATE_AWAITING_BLE_POP;
    //             }
    //             break;

    //         case APP_EVENT_BLE_GAP_DISCONNECT:
    //             break;
    //     }


    //    /*
    //    Antingen ha en thread som gor en freertos delay i 12h och bara skickar ett APP_OTA_DO_UPDATE till xEventGroupWaitBits
    //    som sedan startar OTA function call (inte startar en thread)
    //    eller ha en MQTT topic som subscribar pa topic/new_fw
    //    eller bada?
    //    */
    // }
    xTaskCreate(&pre_encrypted_ota_task, "pre_encrypted_ota_task", 1024 * 8, NULL, 5, NULL);

    while(true){
        ESP_LOGI(TAG, "Waiting forever");

        vTaskDelay(2000 / portTICK_PERIOD_MS);

    }
}
