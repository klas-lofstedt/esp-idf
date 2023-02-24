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
#include "gpio.h"
#include "mqtt.h"
#include "device.h"


static const char *TAG = "APP";

EventGroupHandle_t app_event_group;

static app_events_t app_awaiting_event(app_events_t event_expected);
static const char* app_event_string(app_events_t event_received);
static const char* app_state_string(app_states_t state);

//static void set_production_keys(void);
//static void set_provisioning_keys(void);

static app_events_t app_awaiting_event(app_events_t event_expected)
{
    EventBits_t bits = xEventGroupWaitBits(app_event_group,
        APP_EVENT_BLE_GAP_CONNECT |
        APP_EVENT_BLE_GAP_ADV_COMPLETE |
        APP_EVENT_BLE_GAP_DISCONNECTED |
        APP_EVENT_BLE_RECEIVE_POP_DONE |
        APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE |
        APP_EVENT_BLE_RECEIVE_WIFI_CREDS_FAIL |
        APP_EVENT_BLE_RECEIVE_CA_CERT_DONE |
        APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE |
        APP_EVENT_BLE_NOTIFY_WIFI_CREDS_OK_DONE |
        APP_EVENT_BLE_NOTIFY_CA_CERT_DONE |
        APP_EVENT_WIFI_START |
        APP_EVENT_WIFI_DISCONNECTED |
        APP_EVENT_WIFI_CONNECTED |
        APP_EVENT_WIFI_SCAN_DONE |
        APP_EVENT_UNINITIALISED |
        APP_EVENT_BLE_NOTIFY_WIFI_CREDS_ERROR_DONE |
        APP_EVENT_MQTT_DATA_SENT |
        APP_EVENT_MQTT_DATA_RECEIVED |
        APP_EVENT_MQTT_SUBSCRIBED,

        pdTRUE, pdFALSE, portMAX_DELAY);

    app_events_t event_received = (app_events_t)bits;
    app_events_t event = event_received & event_expected;
    ESP_LOGI(TAG, "EVENT RECEIVED: %s", app_event_string(event_received));
    ESP_LOGI(TAG, "-> DO: %s", app_event_string(event));

    return event;
}

static const char* app_event_string(app_events_t event_received)
{
    const char* event_text = "";
    if (event_received & APP_EVENT_BLE_GAP_CONNECT) event_text = "APP_EVENT_BLE_GAP_CONNECT ";
    if (event_received & APP_EVENT_BLE_GAP_ADV_COMPLETE) event_text = "APP_EVENT_BLE_GAP_ADV_COMPLETE ";
    if (event_received & APP_EVENT_BLE_GAP_DISCONNECTED) event_text = "APP_EVENT_BLE_GAP_DISCONNECTED ";
    if (event_received & APP_EVENT_BLE_RECEIVE_POP_DONE) event_text = "APP_EVENT_BLE_RECEIVE_POP_DONE ";
    if (event_received & APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE) event_text = "APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE ";
    if (event_received & APP_EVENT_BLE_RECEIVE_WIFI_CREDS_FAIL) event_text = "APP_EVENT_BLE_RECEIVE_WIFI_CREDS_FAIL ";
    if (event_received & APP_EVENT_BLE_RECEIVE_CA_CERT_DONE) event_text = "APP_EVENT_BLE_RECEIVE_CA_CERT_DONE ";
    if (event_received & APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE) event_text = "APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE ";
    if (event_received & APP_EVENT_BLE_NOTIFY_WIFI_CREDS_OK_DONE) event_text = "APP_EVENT_BLE_NOTIFY_WIFI_CREDS_OK_DONE ";
    if (event_received & APP_EVENT_BLE_NOTIFY_CA_CERT_DONE) event_text = "APP_EVENT_BLE_NOTIFY_CA_CERT_DONE ";
    if (event_received & APP_EVENT_WIFI_START) event_text = "APP_EVENT_WIFI_START ";
    if (event_received & APP_EVENT_WIFI_DISCONNECTED) event_text = "APP_EVENT_WIFI_DISCONNECTED ";
    if (event_received & APP_EVENT_WIFI_CONNECTED) event_text = "APP_EVENT_WIFI_CONNECTED ";
    if (event_received & APP_EVENT_WIFI_SCAN_DONE) event_text = "APP_EVENT_WIFI_SCAN_DONE ";
    if (event_received & APP_EVENT_UNINITIALISED) event_text = "APP_EVENT_UNINITIALISED ";
    if (event_received & APP_EVENT_BLE_NOTIFY_WIFI_CREDS_ERROR_DONE) event_text = "APP_EVENT_BLE_NOTIFY_WIFI_CREDS_ERROR_DONE ";
    if (event_received & APP_EVENT_MQTT_DATA_SENT) event_text = "APP_EVENT_MQTT_DATA_SENT ";
    if (event_received & APP_EVENT_MQTT_DATA_RECEIVED) event_text = "APP_EVENT_MQTT_DATA_RECEIVED ";
    if (event_received & APP_EVENT_MQTT_SUBSCRIBED) event_text = "APP_EVENT_MQTT_SUBSCRIBED ";


    return event_text;
}

static const char* app_state_string(app_states_t state)
{
    switch (state) {
        case APP_STATE_NORMAL_OPERATION: return "APP_STATE_NORMAL_OPERATION";
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
    gpio_init();
    aes_ctr_init();
    wifi_init();
    ble_init();
    mqtt_init();

    if (!nvs_is_production_done()){
        app_state = APP_STATE_DO_PRODUCTION;
        nvs_set_device_type(DEVICE_TYPE_SWITCH);
        //app_event_expected = APP_EVENT_UART;

        app_state = APP_STATE_NORMAL_OPERATION;
        app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_START;
    } else if (!nvs_is_provision_done()){
        app_state = APP_STATE_DO_PROVISION;
        app_event_expected = APP_EVENT_BLE_GAP_CONNECT;
    } else {
        app_state = APP_STATE_NORMAL_OPERATION;
        app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_START;
    }

    while (true){
        ESP_LOGI(TAG, "Waiting for events...");
        app_event_received = app_awaiting_event(app_event_expected);

        ESP_LOGI(TAG, "STATE: %s: 0x%lx", app_state_string(app_state), (uint32_t)app_state);

        if (app_state == APP_STATE_DO_PRODUCTION){

        }

        if (app_state == APP_STATE_DO_PROVISION){
            switch(app_event_received){
                case APP_EVENT_BLE_GAP_CONNECT:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_BLE_RECEIVE_POP_DONE;
                    break;
                case APP_EVENT_BLE_RECEIVE_POP_DONE:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_SCAN_DONE;
                    wifi_start_scan();
                    break;
                case APP_EVENT_WIFI_SCAN_DONE:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE;
                    ble_notify_wifi_scan();
                    break;
                case APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE;
                    break;
                case APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_START;
                    wifi_reinit();
                    break;
                case APP_EVENT_WIFI_START:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_CONNECTED | APP_EVENT_WIFI_DISCONNECTED;
                    wifi_connect();
                    break;
                case APP_EVENT_WIFI_CONNECTED:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_BLE_NOTIFY_WIFI_CREDS_OK_DONE;
                    ble_notify_provisioning_status(true);
                    break;
                case APP_EVENT_WIFI_DISCONNECTED:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_BLE_NOTIFY_WIFI_CREDS_ERROR_DONE;
                    ble_notify_provisioning_status(false);
                    break;
                case APP_EVENT_BLE_NOTIFY_WIFI_CREDS_OK_DONE:
                    vTaskDelay(2000 / portTICK_PERIOD_MS);
                    esp_restart();
                    break;
                case APP_EVENT_BLE_NOTIFY_WIFI_CREDS_ERROR_DONE:
                    vTaskDelay(2000 / portTICK_PERIOD_MS);
                    esp_restart();
                    break;
                default:
                    ESP_LOGI(TAG, "ERROR: Event broke provision state machine :-)");
                    //esp_restart();
                    break;
            }
        }
        if (app_state == APP_STATE_NORMAL_OPERATION){
            switch(app_event_received){
                case APP_EVENT_BLE_GAP_CONNECT:
                    app_state = APP_STATE_DO_PROVISION;
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_BLE_RECEIVE_POP_DONE;
                    break;
                case APP_EVENT_WIFI_START:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_CONNECTED | APP_EVENT_WIFI_DISCONNECTED;
                    wifi_connect();
                    break;
                case APP_EVENT_WIFI_CONNECTED:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_DISCONNECTED | APP_EVENT_MQTT_SUBSCRIBED;
                    break;
                case APP_EVENT_WIFI_DISCONNECTED:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_CONNECTED | APP_EVENT_WIFI_DISCONNECTED;
                    wifi_connect();
                    break;
                case APP_EVENT_MQTT_DATA_RECEIVED:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_DISCONNECTED | APP_EVENT_MQTT_DATA_SENT | APP_EVENT_MQTT_DATA_RECEIVED | APP_EVENT_MQTT_SUBSCRIBED;
                    device_set(mqtt_data_received());
                    mqtt_data_send(device_get()); // Do this in order to update state on phone
                    break;
                case APP_EVENT_MQTT_DATA_SENT:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_DISCONNECTED | APP_EVENT_MQTT_DATA_SENT | APP_EVENT_MQTT_DATA_RECEIVED | APP_EVENT_MQTT_SUBSCRIBED;
                    break;
                case APP_EVENT_MQTT_SUBSCRIBED:
                    app_event_expected = APP_EVENT_BLE_GAP_CONNECT | APP_EVENT_WIFI_DISCONNECTED | APP_EVENT_MQTT_DATA_SENT | APP_EVENT_MQTT_DATA_RECEIVED | APP_EVENT_MQTT_SUBSCRIBED;
                    mqtt_data_send(device_get());
                    break;
                default:
                    ESP_LOGI(TAG, "ERROR: Event broke normal operation state machine :-)");
                    //esp_restart();
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
