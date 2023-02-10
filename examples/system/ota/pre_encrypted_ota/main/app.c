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
#include "aes.h"
#include <string.h>
#include <stdio.h>
#include <cJSON.h>


static const char *TAG = "APP";

EventGroupHandle_t app_event_group;

static uint32_t awaiting_event(void);
static void set_production_keys(void);
static void set_provisioning_keys(void);

static uint32_t awaiting_event(void)
{
    EventBits_t bits = xEventGroupWaitBits(app_event_group,
        APP_EVENT_BLE_GAP_CONNECT |
        APP_EVENT_BLE_GAP_ADV_COMPLETE |
        APP_EVENT_BLE_GAP_DISCONNECT |
        APP_EVENT_BLE_RECEIVE_POP_DONE |
        APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE |
        APP_EVENT_BLE_RECEIVE_CA_CERT_DONE |
        APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE |
        APP_EVENT_BLE_NOTIFY_WIFI_CREDS_DONE |
        APP_EVENT_BLE_NOTIFY_CA_CERT_DONE |
        APP_EVENT_WIFI_START |
        APP_EVENT_WIFI_DISCONNECTED |
        APP_EVENT_WIFI_CONNECTED |
        APP_EVENT_WIFI_SCAN_DONE,
        //APP_EVENT_BLE_NOTIFY_DONE,

        pdTRUE, pdFALSE, portMAX_DELAY);

    uint32_t event = (uint32_t)bits;

    return event;
}

// TODO: these should be sent to the RPi jig. To prevent that -someone- reads them out
// on their own product, these should be sent AES encrypted with a key that is hardcoded
// in esp32 FW and RPi SW. Create a JSON blob that is base64 encoded and send to RPi over uart
static void set_production_keys(void)
{
    // BLE pop
    nvs_set_ble_pop();
    uint8_t pop[16];
    nvs_get_ble_pop_arr(pop);
    ESP_LOG_BUFFER_HEX(TAG, pop, 16);
    char popstr[2* 16];
    nvs_get_ble_pop_str(popstr);
    ESP_LOGI(TAG, "%s", popstr);

    // AES key
    nvs_set_aes_key();
    uint8_t aes[16];
    nvs_get_aes_key_arr(aes);
    ESP_LOG_BUFFER_HEX(TAG, aes, 16);
    char aesstr[2* 16];
    nvs_get_aes_key_str(aesstr);
    ESP_LOGI(TAG, "%s", aesstr);
}



static void set_provisioning_keys(void)
{

}


void app_main(void)
{
    app_event_group = xEventGroupCreate();
    app_states_t app_state = APP_STATE_AWAITING_INIT;

    nvs_init();

    //nvs_wifi_set_default();

    wifi_init();
    //wifi_connect_propeller();
    //set_production_keys();


    // char input1[] = {0xeb, 0xc0, 0xb6, 0x22};
    // unsigned char output1[sizeof(input1)];

    aes_ctr_init();
    // aes_ctr_crypto(input1, output1);
    // ESP_LOGI(TAG, "AES input1: %s, len: %d", input1, sizeof(input1));
    // ESP_LOGI(TAG, "AES output1: %c%c%c%c, len: %d", output1[0], output1[1], output1[2], output1[3], sizeof(output1));
    // ESP_LOG_BUFFER_HEX(TAG, output1, sizeof(output1));
    //char stringoutput[2*sizeof(output1)];
    //string_from_u8_array(output1, stringoutput, sizeof(output1));
    //ESP_LOGI(TAG, "stringoutput: %s, len: %d", stringoutput, sizeof(stringoutput));

    // //const unsigned char *input2 = (const unsigned char *)output1;
    // aes_ctr_crypto(output1, output2);
    // //ESP_LOGI(TAG, "AES input2: %s, len: %d", input2, sizeof(input2));
    // ESP_LOGI(TAG, "AES output2: %s, len: %d", output2, sizeof(output2));
    // ESP_LOG_BUFFER_HEX(TAG, output1, sizeof(output1));



    char aes_key[32];
    nvs_get_aes_key_str(aes_key);
    ESP_LOGI(TAG, "AES KEY: %s", aes_key);

    uint8_t aes[16];
    nvs_get_aes_key_arr(aes);
    ESP_LOG_BUFFER_HEX(TAG, aes, 16);



    //ESP_LOGI(TAG, "AES encrypt output: %s", output);
    // char output2[strlen(input)];
    // aes_decrypt(output, output2);
    // ESP_LOGI(TAG, "AES decrypt output: %s", output2);



    //wifi_scan_networks();

    ble_init();

// const char *input = "{\"type\":\"creds\",\"ssid\":\"WiFi-113E7F\",\"password\":\"hej\"}";

//     parse_and_print_json(input);


    app_state = APP_STATE_NORMAL_OPERATION;
    while (true){
        ESP_LOGI(TAG, "Waiting for events...");

        uint32_t app_event = awaiting_event();

        switch (app_event){
            case APP_EVENT_BLE_RECEIVE_POP_DONE:
                ESP_LOGI(TAG, "APP_EVENT_BLE_RECEIVE_POP_DONE");
                if (app_state == APP_STATE_AWAITING_BLE_RECEIVE_POP){
                    wifi_start_scan();
                    ESP_LOGI(TAG, "new state -> APP_STATE_AWAITING_WIFI_SCAN_DONE");
                    app_state = APP_STATE_AWAITING_WIFI_SCAN_DONE;
                }
                break;

            case APP_EVENT_WIFI_SCAN_DONE:
                ESP_LOGI(TAG, "APP_EVENT_WIFI_SCAN_DONE");
                if (app_state == APP_STATE_AWAITING_WIFI_SCAN_DONE){
                    if (ble_notify_wifi_scan()){
                        ESP_LOGI(TAG, "new state -> APP_STATE_AWAITING_BLE_NOTIFY_WIFI_SCAN_DONE");
                        app_state = APP_STATE_AWAITING_BLE_NOTIFY_WIFI_SCAN_DONE;
                    } else {
                        // TODO
                    }
                }
                break;

            case APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE:
                ESP_LOGI(TAG, "APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE");
                if (app_state == APP_STATE_AWAITING_BLE_NOTIFY_WIFI_SCAN_DONE){
                    app_state = APP_STATE_AWAITING_BLE_RECEIVE_WIFI_CREDS;
                }
                break;

            case APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE:
                ESP_LOGI(TAG, "APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE");
                if (app_state == APP_STATE_AWAITING_BLE_RECEIVE_WIFI_CREDS){
                    // if (ble_notify_wifi_creds(true)) {
                    //     app_state = APP_STATE_AWAITING_BLE_NOTIFY_WIFI_CREDS_DONE;
                    // }
                }
                break;

            case APP_EVENT_BLE_NOTIFY_WIFI_CREDS_DONE:
                ESP_LOGI(TAG, "APP_EVENT_BLE_NOTIFY_WIFI_CREDS_DONE");
                if (app_state == APP_STATE_AWAITING_BLE_NOTIFY_WIFI_CREDS_DONE){
                    app_state = APP_STATE_AWAITING_BLE_RECEIVE_CA_CERT;
                }
                break;

            case APP_EVENT_BLE_RECEIVE_CA_CERT_DONE:
                ESP_LOGI(TAG, "APP_EVENT_BLE_RECEIVE_CA_CERT_DONE");
                if (app_state == APP_STATE_AWAITING_BLE_RECEIVE_CA_CERT){
                    // if (ble_notify_ca_cert(true)) {
                    //     app_state = APP_STATE_AWAITING_BLE_NOTIFY_CA_CERT_DONE;
                    // }

                }
                break;

            case APP_EVENT_BLE_NOTIFY_CA_CERT_DONE:
                ESP_LOGI(TAG, "APP_EVENT_BLE_NOTIFY_CA_CERT_DONE");
                if (app_state == APP_STATE_AWAITING_BLE_NOTIFY_CA_CERT_DONE){
                    app_state = APP_STATE_AWAITING_WIFI_CONNECTED;
                    //app_state = APP_STATE_AWAITING_WIFI_CONNECTED;
                    //wifi_connect();
                }
                break;

            case APP_EVENT_WIFI_CONNECTED:
                ESP_LOGI(TAG, "APP_EVENT_WIFI_CONNECTED");
                app_state = APP_STATE_NORMAL_OPERATION;

                break;

            case APP_EVENT_WIFI_DISCONNECTED:
                if (app_state == APP_STATE_NORMAL_OPERATION){
                    ESP_LOGI(TAG, "APP_EVENT_WIFI_STA_DISCONNECTED");
                    wifi_connect();
                }
                if (app_state == APP_STATE_AWAITING_WIFI_CONNECTED){
                    ESP_LOGI(TAG, "APP_EVENT_WIFI_STA_DISCONNECTED");
                    wifi_connect();
                }
                break;

            case APP_EVENT_BLE_GAP_CONNECT:
                ESP_LOGI(TAG, "APP_EVENT_BLE_GAP_CONNECT");
                if (app_state == APP_STATE_NORMAL_OPERATION){
                    app_state = APP_STATE_AWAITING_BLE_RECEIVE_POP;
                }
                if (app_state == APP_STATE_AWAITING_WIFI_CONNECTED){
                    app_state = APP_STATE_AWAITING_BLE_RECEIVE_POP;
                }
                break;

            case APP_EVENT_BLE_GAP_DISCONNECT:
                break;


            default:
                ESP_LOGI(TAG, "default");
                break;
        }


       /*
       Antingen ha en thread som gor en freertos delay i 12h och bara skickar ett APP_OTA_DO_UPDATE till xEventGroupWaitBits
       som sedan startar OTA function call (inte startar en thread)
       eller ha en MQTT topic som subscribar pa topic/new_fw
       eller bada?
       */
    }
    //xTaskCreate(&pre_encrypted_ota_task, "pre_encrypted_ota_task", 1024 * 8, NULL, 5, NULL);

    // while(true){
    //     ESP_LOGI(TAG, "Waiting forever");

    //     vTaskDelay(2000 / portTICK_PERIOD_MS);

    // }
}
