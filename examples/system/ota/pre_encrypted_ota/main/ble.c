#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "ble.h"
#include "wifi.h"
#include "nvs.h"
#include "app.h"
#include <cJSON.h>
#include "esp_wifi.h"
#include "misc.h"
#include "aes.h"


#define SERVICE_UUID                0xFFFF
// Write to esp32
#define CHAR_WRITE_POP_UUID         0xFF01
#define CHAR_WRITE_CREDS_UUID       0xFF02
#define CHAR_WRITE_CERT_UUID        0xFF03
// Notify from esp32
#define CHAR_NOTIFY_SCAN_UUID       0xFF04
//#define CHAR_NOTIFY_STATUS_UUID     0xFF05
// Read from esp32
#define CHAR_READ_UUID              0xFF06



static const char *TAG = "BLE";

uint8_t ble_addr_type;
uint16_t control_notif_handle;
uint16_t conn_handle;
char rx_data[255];

static bool handle_json_string(char *json_input);
static int ble_write_creds_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int ble_device_read_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int ble_notify_dummy_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int ble_event_handler(struct ble_gap_event *event, void *arg);
//static int ble_device_notify(int16_t conn_handle, uint32_t data);

bool handle_json_string(char *json_input)//, char *ssid, char *password)
{
    cJSON *root = cJSON_Parse(json_input);
    if (!root) {
        ESP_LOGI(TAG, "Error parsing JSON input");
        return false;
    }

    cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "type");

    if (!cJSON_IsString(type)) {
        ESP_LOGI(TAG, "Error: JSON type is not a string");
        cJSON_Delete(root);
        return false;
    }

    if (strcmp(type->valuestring, "creds") == 0){
        char *ssid = cJSON_GetObjectItemCaseSensitive(root, "ssid")->valuestring;
        char *password = cJSON_GetObjectItemCaseSensitive(root, "password")->valuestring;
        if (!nvs_wifi_set_ssid(ssid)){
            cJSON_Delete(root);
            return false;
        }
        if (!nvs_wifi_set_pwd(password)){
            cJSON_Delete(root);
            return false;
        }
        if (!nvs_wifi_set_provisioned(true)){
            cJSON_Delete(root);
            return false;
        }
        // char ssid[WIFI_SSID_MAX_LEN];
        // char password[WIFI_PWD_MAX_LEN];
        // memset(ssid, 0, sizeof(ssid));
        // memset(password, 0, sizeof(password));
        // memcpy(ssid, ssid_temp, strlen(ssid_temp));
        // memcpy(password, password_temp, strlen(password_temp));
    } else if (strcmp(type->valuestring, "cert") == 0){
        //char *cert = cJSON_GetObjectItemCaseSensitive(root, "line")->valuestring;
        //int *count = cJSON_GetObjectItemCaseSensitive(root, "count")->valueint;
    } else {
        ESP_LOGI(TAG, "Error: JSON has unknown subtype ");
        cJSON_Delete(root);
        return false;
    }

    //printf("Received: %s, %s, %s\n", type, ssid, password);
    //ESP_LOGI(TAG, "Received: %s, %s, %s", type, ssid_temp, password_temp);

    cJSON_Delete(root); // DONT move this before the print!!!
    return true;
}
// Write data to ESP32 defined as server
static int ble_write_pop_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    memset(rx_data, 0, sizeof(rx_data));

    //printf("Data from the client: %d, %d\n", ctxt->om->om_len, ctxt->om->om_data);
    memcpy(rx_data,ctxt->om->om_data,ctxt->om->om_len);
    ESP_LOGI(TAG, "POP: %s, conn_handle %d", rx_data, conn_handle);
    char pop_ble[BLE_POP_SIZE * 2];
    memcpy(pop_ble, rx_data, BLE_POP_SIZE * 2);
    //aes_ctr_crypto(rx_data, pop);
    //ESP_LOGI(TAG, "Unencrypted POP: %s", pop);
    char pop_nvs[32];
    nvs_get_ble_pop_str(pop_nvs);
    ESP_LOGI(TAG, "NVS POP: %s", pop_nvs);
    if (strncmp(pop_nvs, pop_ble, BLE_POP_SIZE * 2) == 0){
        ESP_LOGI(TAG, "POP are equal");
    } else {
        ESP_LOGI(TAG, "POP error");
    }

    //printf("Receive Data =  %.*s, conn_handle = %d\n", WIFI_PWD_MAX_LEN, rx_data, conn_handle);
    xEventGroupSetBits(app_event_group, APP_EVENT_BLE_RECEIVE_POP_DONE);

    //ble_device_notify(0, "hej");

    // if (nvs_wifi_set_pwd(rx_data)){
    //     ESP_LOGI(TAG, "Write WiFi PWD successful");
    //     ESP_LOGI(TAG, "Rebooting...");
    //     esp_restart();
    // } else {
    //     ESP_LOGI(TAG, "Write WiFi PWD failed");
    // }
    return 0;
}

static int ble_write_creds_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    memset(rx_data, 0, sizeof(rx_data));
    memcpy(rx_data,ctxt->om->om_data,ctxt->om->om_len);

    char decrypted_string[ctxt->om->om_len / 2];
    aes_ctr_crypto(rx_data, decrypted_string, ctxt->om->om_len / 2 );

    // char ssid[WIFI_SSID_MAX_LEN];
    // char password[WIFI_PWD_MAX_LEN];
    // memset(ssid, 0, sizeof(ssid));
    // memset(password, 0, sizeof(password));

    if (handle_json_string(decrypted_string)) {
        xEventGroupSetBits(app_event_group, APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE);
    } else {
        xEventGroupSetBits(app_event_group, APP_EVENT_BLE_RECEIVE_WIFI_CREDS_DONE);
    }
    // ESP_LOGI(TAG, "Received in BLE: %s, %s", ssid, password);

    // ESP_LOGI(TAG, "SSID: ");
    // ESP_LOG_BUFFER_HEX(TAG, ssid, strlen(ssid));
    // ESP_LOGI(TAG, "PropellerAero: ");
    // ESP_LOG_BUFFER_HEX(TAG, "PropellerAero", strlen("PropellerAero"));
    // ESP_LOGI(TAG, "PWD: ");
    // ESP_LOG_BUFFER_HEX(TAG, password, strlen(password));
    // ESP_LOGI(TAG, "hejklas1234: ");
    // ESP_LOG_BUFFER_HEX(TAG, "hejklas1234", strlen("hejklas1234"));




    //ble_device_notify(0, "hej");

    // if (nvs_wifi_set_pwd(rx_data)){
    //     ESP_LOGI(TAG, "Write WiFi PWD successful");
    //     ESP_LOGI(TAG, "Rebooting...");
    //     esp_restart();
    // } else {
    //     ESP_LOGI(TAG, "Write WiFi PWD failed");
    // }
    return 0;
}


static int ble_write_cert_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    //printf("Data from the client: %d, %d\n", ctxt->om->om_len, ctxt->om->om_data);
    memcpy(rx_data,ctxt->om->om_data,ctxt->om->om_len);
    ESP_LOGI(TAG, "cert: %s", rx_data);

    //printf("Receive Data =  %.*s, conn_handle = %d\n", WIFI_PWD_MAX_LEN, rx_data, conn_handle);
    xEventGroupSetBits(app_event_group, APP_EVENT_BLE_RECEIVE_CA_CERT_DONE);

    //ble_device_notify(0, "hej");

    // if (nvs_wifi_set_pwd(rx_data)){
    //     ESP_LOGI(TAG, "Write WiFi PWD successful");
    //     ESP_LOGI(TAG, "Rebooting...");
    //     esp_restart();
    // } else {
    //     ESP_LOGI(TAG, "Write WiFi PWD failed");
    // }
    return 0;
}

// void parse_json_ble_message(char *message)
// {
//     cJSON *message_root = NULL;
//     message_root = cJSON_Parse(message);
//     if(message_root == NULL){
//         ESP_LOGI(TAG, "JSON parse fail");
//         return;
//     }

//     char *ssid = cJSON_GetObjectItem(message_root, "ssid") ->valuestring;
//     char *pwd = cJSON_GetObjectItem(message_root, "pwd") ->valuestring;
//     char *id = cJSON_GetObjectItem(message_root, "id") ->valuestring;

//     // Update global device struct
//     if (!strcmp(ping, "online")){
//         device_status.ping = true;
//     }
//     if (!strcmp(ping, "offline")){
//         device_status.ping = false;
//     }
//     if (!strcmp(status, "on")){
//         device_status.status = true;
//         gpio_set_level(PIN_LED, true);
//     }
//     if (!strcmp(status, "off")){
//         device_status.status = false;
//         gpio_set_level(PIN_LED, false);
//     }
//     cJSON_Delete(message_root);

//     ESP_LOGI(TAG, "MQTT Receive: status %d, ping %d", device_status.status, device_status.ping);
// }

// Read data from ESP32 defined as server
static int ble_device_read_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, "OK", strlen("OK"));
    return 0;
}

bool ble_notify_wifi_scan(void)
{
    uint16_t apCount = WIFI_MAX_SCAN_RESULTS;
    wifi_ap_record_t ap_list[WIFI_MAX_SCAN_RESULTS];

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, ap_list));

    for (int i = 0; i < apCount; i++){
        ESP_LOGI(TAG, "SSID: %s, RSSI: %d", ap_list[i].ssid, ap_list[i].rssi);
    }

    esp_wifi_scan_stop();

    for(int i = 0; i < apCount; i++) {
        char *json_str = NULL;
        cJSON *message_root = cJSON_CreateObject();

        cJSON_AddStringToObject(message_root, "type", "wifi_scan");
        cJSON_AddStringToObject(message_root, "ssid", (char *)ap_list[i].ssid);
        cJSON_AddNumberToObject(message_root, "rssi", ap_list[i].rssi);
        cJSON_AddNumberToObject(message_root, "count", apCount - i - 1);

        json_str = cJSON_Print(message_root);
        cJSON_Delete(message_root);

        struct os_mbuf *om;
        om = ble_hs_mbuf_from_flat(json_str, strlen(json_str));
        // TODO: come up with a better way to handle conn_handle
        conn_handle = 0;
        ESP_LOGI(TAG, "Notifying conn=%d", conn_handle);
        int rc = ble_gatts_notify_custom(conn_handle, control_notif_handle, om);
        if (rc != 0) {
            ESP_LOGE(TAG, "error notifying; rc=%d", rc);
            return false;
        }
        cJSON_free(json_str);
    }
    xEventGroupSetBits(app_event_group, APP_EVENT_BLE_NOTIFY_WIFI_SCAN_DONE);

    return true;
}

bool ble_notify_provisioning_status(bool status)
{
    char *json_str = NULL;
    cJSON *message_root = cJSON_CreateObject();

    cJSON_AddStringToObject(message_root, "type", "provision");
    if (status){
        cJSON_AddStringToObject(message_root, "status", "ok");
    } else {
        cJSON_AddStringToObject(message_root, "status", "error");
    }
    json_str = cJSON_Print(message_root);
    cJSON_Delete(message_root);

    struct os_mbuf *om;
    om = ble_hs_mbuf_from_flat(json_str, strlen(json_str));
    // TODO: come up with a better way to handle conn_handle
    conn_handle = 0;
    ESP_LOGI(TAG, "Notifying conn=%d", conn_handle);
    int rc = ble_gatts_notify_custom(conn_handle, control_notif_handle, om);
    if (rc != 0) {
        ESP_LOGE(TAG, "error notifying; rc=%d", rc);
        return false;
    }
    cJSON_free(json_str);

    xEventGroupSetBits(app_event_group, APP_EVENT_BLE_NOTIFY_WIFI_CREDS_DONE);

    return true;
}

bool ble_notify_ca_cert(bool status)
{
    char *json_str = NULL;
    cJSON *message_root = cJSON_CreateObject();

    cJSON_AddStringToObject(message_root, "type", "wifi_cert");
    if (status){
        cJSON_AddStringToObject(message_root, "status", "ok");
    } else {
        cJSON_AddStringToObject(message_root, "status", "error");
    }
    cJSON_AddNumberToObject(message_root, "count", 0);

    json_str = cJSON_Print(message_root);
    cJSON_Delete(message_root);

    struct os_mbuf *om;
    om = ble_hs_mbuf_from_flat(json_str, strlen(json_str));
    // TODO: come up with a better way to handle conn_handle
    conn_handle = 0;
    ESP_LOGI(TAG, "Notifying conn=%d", conn_handle);
    int rc = ble_gatts_notify_custom(conn_handle, control_notif_handle, om);
    if (rc != 0) {
        ESP_LOGE(TAG, "error notifying; rc=%d", rc);
        return false;
    }
    cJSON_free(json_str);

    xEventGroupSetBits(app_event_group, APP_EVENT_BLE_NOTIFY_CA_CERT_DONE);

    return true;
}

static int ble_notify_dummy_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    ESP_LOGI(TAG, "ble_device_notify_dummy_cb");

    return 0;
}


// Array of pointers to other service definitions
// UUID - Universal Unique Identifier
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {   .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(SERVICE_UUID),                 // Define UUID for device type
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_WRITE_POP_UUID),           // Define UUID for writing
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = ble_write_pop_cb
            },
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_WRITE_CREDS_UUID),           // Define UUID for writing
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = ble_write_creds_cb
            },
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_WRITE_CERT_UUID),           // Define UUID for writing
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = ble_write_cert_cb
            },
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_NOTIFY_SCAN_UUID),
                .access_cb = ble_notify_dummy_cb,
                .val_handle = &control_notif_handle,
                .flags = BLE_GATT_CHR_F_NOTIFY
            },
            // {
            //     .uuid = BLE_UUID16_DECLARE(CHAR_NOTIFY_STATUS_UUID),
            //     .access_cb = ble_notify_dummy_cb,
            //     .val_handle = &control_notif_handle,
            //     .flags = BLE_GATT_CHR_F_NOTIFY
            // },
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_READ_UUID),           // Define UUID for reading
                .flags = BLE_GATT_CHR_F_READ,
                .access_cb = ble_device_read_cb
            },
            {0}
        }
    },
    {0}
};

// BLE event handling
static int ble_event_handler(struct ble_gap_event *event, void *arg)
{
    ESP_LOGI(TAG, "DEBUG BLE EVENT: %d", event->type);
    //ESP_LOGI(TAG, "conn_handle: %d", event->connect.conn_handle);
    conn_handle = event->connect.conn_handle;

    switch (event->type)
    {
    // Advertise if connected
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        // char* wifi_scan_char_array = wifi_scan_networks();
        // ESP_LOGI(TAG, "SCAN: %s", wifi_scan_char_array);
        if (event->connect.status != 0)
        {
            ble_start();
        } else {
            xEventGroupSetBits(app_event_group, APP_EVENT_BLE_GAP_CONNECT);
        }
        break;
    // Advertise again after completion of the event
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "BLE GAP EVENT ADV COMPLETE");
        ble_start();
        xEventGroupSetBits(app_event_group, APP_EVENT_BLE_GAP_ADV_COMPLETE);
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "BLE GAP EVENT DISCONNECT");
        ble_start();
        xEventGroupSetBits(app_event_group, APP_EVENT_BLE_GAP_DISCONNECT);
        break;
    case BLE_GAP_EVENT_NOTIFY_TX:
        ESP_LOGI(TAG, "BLE_GAP_EVENT_NOTIFY_TX");

        break;
    default:
        break;
    }
    return 0;
}

// Define the BLE connection
void ble_start(void)
{
    ESP_LOGI(TAG, "Start advertising!");
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); // Read the BLE device name
    ESP_LOGI(TAG, "Device name: %s", device_name);

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_event_handler, NULL);
}

// The application
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
    ble_start();                     // Define the BLE connection
}

// The infinite task
void ble_host_task(void *param)
{
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
}

void ble_init(void)
{
    char device_name[DEVICE_ID_LEN];
    get_device_id(device_name);

    nimble_port_init();                        // 3 - Initialize the host stack
    ble_svc_gap_device_name_set(device_name); // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application
    nimble_port_freertos_init(ble_host_task);  // 6 - Run the thread
}
