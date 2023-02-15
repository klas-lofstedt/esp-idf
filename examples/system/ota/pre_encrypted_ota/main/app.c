#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "ota.h"
#include "nvs.h"
#include "wifi.h"
#include "ble.h"
#include "app.h"
#include "misc.h"
#include "aes.h"
#include <string.h>
#include <stdio.h>
#include <cJSON.h>


static const char *TAG = "APP";

EventGroupHandle_t app_event_group;

static uint32_t app_awaiting_event(void);
static const char* app_event_string(app_events_t event);
static const char* app_state_string(app_states_t state);

//static void set_production_keys(void);
//static void set_provisioning_keys(void);

static uint32_t app_awaiting_event(void)
{
    EventBits_t bits = xEventGroupWaitBits(app_event_group,
        APP_EVENT_BLE_GAP_CONNECT |
        APP_EVENT_BLE_GAP_ADV_COMPLETE |
        APP_EVENT_BLE_GAP_DISCONNECT |
        APP_EVENT_BLE_RECEIVE_POP_DONE |
        APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE |
        APP_EVENT_BLE_RECEIVE_WIFI_CREDS_FAIL |
        APP_EVENT_BLE_RECEIVE_CA_CERT_DONE |
        APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE |
        APP_EVENT_BLE_NOTIFY_WIFI_CREDS_DONE |
        APP_EVENT_BLE_NOTIFY_CA_CERT_DONE |
        APP_EVENT_WIFI_START |
        APP_EVENT_WIFI_DISCONNECTED |
        APP_EVENT_WIFI_CONNECTED |
        APP_EVENT_WIFI_SCAN_DONE |
        APP_EVENT_UNINITIALISED,
        //APP_EVENT_BLE_NOTIFY_DONE,

        pdTRUE, pdFALSE, portMAX_DELAY);

    uint32_t event = (uint32_t)bits;

    return event;
}

static const char* app_event_string(app_events_t event)
{
    switch (event) {
        case APP_EVENT_BLE_GAP_CONNECT: return "APP_EVENT_BLE_GAP_CONNECT";
        case APP_EVENT_BLE_GAP_ADV_COMPLETE: return "APP_EVENT_BLE_GAP_ADV_COMPLETE";
        case APP_EVENT_BLE_GAP_DISCONNECT: return "APP_EVENT_BLE_GAP_DISCONNECT";
        case APP_EVENT_BLE_RECEIVE_POP_DONE: return "APP_EVENT_BLE_RECEIVE_POP_DONE";
        case APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE: return "APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE";
        case APP_EVENT_BLE_RECEIVE_WIFI_CREDS_FAIL: return "APP_EVENT_BLE_RECEIVE_WIFI_CREDS_FAIL";
        case APP_EVENT_BLE_RECEIVE_CA_CERT_DONE: return "APP_EVENT_BLE_RECEIVE_CA_CERT_DONE";
        case APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE: return "APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE";
        case APP_EVENT_BLE_NOTIFY_WIFI_CREDS_DONE: return "APP_EVENT_BLE_NOTIFY_WIFI_CREDS_DONE";
        case APP_EVENT_BLE_NOTIFY_CA_CERT_DONE: return "APP_EVENT_BLE_NOTIFY_CA_CERT_DONE";
        case APP_EVENT_WIFI_START: return "APP_EVENT_WIFI_START";
        case APP_EVENT_WIFI_DISCONNECTED: return "APP_EVENT_WIFI_DISCONNECTED";
        case APP_EVENT_WIFI_CONNECTED: return "APP_EVENT_WIFI_CONNECTED";
        case APP_EVENT_WIFI_SCAN_DONE: return "APP_EVENT_WIFI_SCAN_DONE";
        case APP_EVENT_UNINITIALISED: return "APP_EVENT_UNINITIALISED";
        default: return "UNKNOWN_APP_EVENT";
    }
}

static const char* app_state_string(app_states_t state)
{
    switch (state) {
        case APP_STATE_AWAITING_INIT: return "APP_STATE_AWAITING_INIT";
        case APP_STATE_AWAITING_BLE_CONNECTION: return "APP_STATE_AWAITING_BLE_CONNECTION";
        case APP_STATE_AWAITING_BLE_RECEIVE_POP: return "APP_STATE_AWAITING_BLE_RECEIVE_POP";
        case APP_STATE_AWAITING_WIFI_SCAN_DONE: return "APP_STATE_AWAITING_WIFI_SCAN_DONE";
        case APP_STATE_AWAITING_BLE_NOTIFY_WIFI_SCAN_DONE: return "APP_STATE_AWAITING_BLE_NOTIFY_WIFI_SCAN_DONE";
        case APP_STATE_AWAITING_BLE_RECEIVE_WIFI_CREDS: return "APP_STATE_AWAITING_BLE_RECEIVE_WIFI_CREDS";
        case APP_STATE_AWAITING_BLE_NOTIFY_WIFI_CREDS_DONE: return "APP_STATE_AWAITING_BLE_NOTIFY_WIFI_CREDS_DONE";
        case APP_STATE_AWAITING_BLE_RECEIVE_CA_CERT: return "APP_STATE_AWAITING_BLE_RECEIVE_CA_CERT";
        case APP_STATE_AWAITING_BLE_NOTIFY_CA_CERT_DONE: return "APP_STATE_AWAITING_BLE_NOTIFY_CA_CERT_DONE";
        case APP_STATE_AWAITING_WIFI_CONNECTED: return "APP_STATE_AWAITING_WIFI_CONNECTED";
        case APP_STATE_AWAITING_WIFI_DISCONNECTED: return "APP_STATE_AWAITING_WIFI_DISCONNECTED";
        case APP_STATE_NORMAL_OPERATION: return "APP_STATE_NORMAL_OPERATION";
        case APP_STATE_AWAITING_WIFI_SCAN_READY: return "APP_STATE_AWAITING_WIFI_SCAN_READY";
        case APP_STATE_DO_PRODUCTION: return "APP_STATE_DO_PRODUCTION";
        case APP_STATE_DO_PROVISION: return "APP_STATE_DO_PROVISION";
        case APP_STATE_UNINITIALISED: return "APP_STATE_UNINITIALISED";
        default: return "UNKNOWN_APP_STATE";
    }
}

// TODO: these should be sent to the RPi jig. To prevent that -someone- reads them out
// on their own product, these should be sent AES encrypted with a key that is hardcoded
// in esp32 FW and RPi SW. Create a JSON blob that is base64 encoded and send to RPi over uart
// static void set_production_keys(void)
// {
//     // BLE pop
//     nvs_set_ble_pop();
//     uint8_t pop[16];
//     nvs_get_ble_pop_arr(pop);
//     ESP_LOG_BUFFER_HEX(TAG, pop, 16);
//     char popstr[2* 16];
//     nvs_get_ble_pop_str(popstr);
//     ESP_LOGI(TAG, "%s", popstr);

//     // AES key
//     nvs_set_aes_key();
//     uint8_t aes[16];
//     nvs_get_aes_key_arr(aes);
//     ESP_LOG_BUFFER_HEX(TAG, aes, 16);
//     char aesstr[2* 16];
//     nvs_get_aes_key_str(aesstr);
//     ESP_LOGI(TAG, "%s", aesstr);
// }



// static void set_provisioning_keys(void)
// {

// }


void app_main(void)
{
    app_event_group = xEventGroupCreate();
    app_states_t app_state = APP_STATE_UNINITIALISED;
    app_events_t app_event_received = APP_EVENT_UNINITIALISED;
    app_events_t app_event_expected = APP_EVENT_UNINITIALISED;

    nvs_init();
    aes_ctr_init();
    wifi_init();
    ble_init();

    if (!nvs_is_production_done()){
        app_state = APP_STATE_DO_PRODUCTION;
        //app_event_expected = APP_EVENT_UART;
    }else if (!nvs_is_provision_done()){
        app_state = APP_STATE_DO_PROVISION;
        app_event_expected = APP_EVENT_BLE_GAP_CONNECT;
    } else {
        app_state = APP_STATE_NORMAL_OPERATION;
        app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_START;
    }

    while (true){
        ESP_LOGI(TAG, "Waiting for events...");

        app_event_received = app_awaiting_event();

        ESP_LOGI(TAG, "STATE: %s: 0x%lx", app_state_string(app_state), (uint32_t)app_state);
        ESP_LOGI(TAG, "EVENT: %s: 0x%lx", app_event_string(app_event_received), (uint32_t)app_event_received);

        if (app_state == APP_STATE_DO_PRODUCTION){

        }
        if (app_state == APP_STATE_DO_PROVISION){
            switch(app_event_received & app_event_expected){
                case APP_EVENT_BLE_GAP_CONNECT:
                    app_event_expected = APP_EVENT_BLE_RECEIVE_POP_DONE;
                    break;
                case APP_EVENT_BLE_RECEIVE_POP_DONE:
                    app_event_expected = APP_EVENT_WIFI_SCAN_DONE;
                    wifi_start_scan();
                    break;
                case APP_EVENT_WIFI_SCAN_DONE:
                    app_event_expected = APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE;
                    ble_notify_wifi_scan();
                    break;
                case APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE:
                    app_event_expected = APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE;
                    break;
                case APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE:
                    app_event_expected = APP_EVENT_WIFI_START;
                    wifi_reinit();
                    break;
                case APP_EVENT_WIFI_START:
                    app_event_expected = APP_EVENT_WIFI_CONNECTED | APP_EVENT_WIFI_DISCONNECTED;
                    wifi_connect();
                    break;
                case APP_EVENT_WIFI_CONNECTED:
                    app_event_expected = APP_EVENT_BLE_NOTIFY_WIFI_CREDS_DONE;
                    ble_notify_provisioning_status(true);
                    break;
                case APP_EVENT_WIFI_DISCONNECTED:
                    app_event_expected = APP_EVENT_BLE_NOTIFY_WIFI_CREDS_DONE;
                    ble_notify_provisioning_status(false);
                    break;
                case APP_EVENT_BLE_NOTIFY_WIFI_CREDS_DONE:
                    app_state = APP_STATE_NORMAL_OPERATION;
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_START;
                    break;
                default:
                    // TODO: print that event was not handled or reboot?
                    break;
            }
        }
        if (app_state == APP_STATE_NORMAL_OPERATION){
            switch(app_event_received & app_event_expected){
                case APP_EVENT_BLE_GAP_CONNECT:
                    app_state = APP_STATE_DO_PROVISION;
                    app_event_expected = APP_EVENT_BLE_RECEIVE_POP_DONE;
                    break;
                case APP_EVENT_WIFI_START:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_CONNECTED | APP_EVENT_WIFI_DISCONNECTED;
                    wifi_connect();
                    break;
                case APP_EVENT_WIFI_CONNECTED:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_DISCONNECTED;
                    // TODO: do nothing? or expect mqtt or OTA etc
                    break;
                case APP_EVENT_WIFI_DISCONNECTED:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_CONNECTED | APP_EVENT_WIFI_DISCONNECTED;
                    wifi_connect();
                    break;
                default:
                    // TODO: print that event was not handled or reboot?
                    break;
            }
        }
    }
        //    /*
    //    Antingen ha en thread som gor en freertos delay i 12h och bara skickar ett APP_OTA_DO_UPDATE till xEventGroupWaitBits
    //    som sedan startar OTA function call (inte startar en thread)
    //    eller ha en MQTT topic som subscribar pa topic/new_fw
    //    eller bada?
    //    */
    // }
}
