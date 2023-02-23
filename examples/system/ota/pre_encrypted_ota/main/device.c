#include "device.h"
#include <cJSON.h>
#include "gpio.h"
#include <string.h>
#include "esp_log.h"
#include "misc.h"

static const char *TAG = "DEVICE";

static void parse_json_mqtt_message(const char *message_mqtt);

typedef struct DeviceStatus {
    bool status;
    bool ping;
} DeviceStatus;

DeviceStatus device_status = { .status = false, .ping = true };

void device_set(const char* json_data)
{
    parse_json_mqtt_message(json_data);
    gpio_toggle_led(device_status.status);
}

static void parse_json_mqtt_message(const char *message_mqtt)
{
    ESP_LOGI(TAG, "Raw message: %s", message_mqtt);

    cJSON *message_root = NULL;
    message_root = cJSON_Parse(message_mqtt);
    if(message_root == NULL){
        ESP_LOGI(TAG, "MQTT subscription message parse fail");
        return;
    }

    char *status = cJSON_GetObjectItem(message_root, "status") ->valuestring;
    char *ping = cJSON_GetObjectItem(message_root, "ping") ->valuestring;
    // Update global device struct
    if (!strcmp(ping, "online")){
        device_status.ping = true;
    }
    if (!strcmp(ping, "offline")){
        device_status.ping = false;
    }
    if (!strcmp(status, "on")){
        device_status.status = true;
    }
    if (!strcmp(status, "off")){
        device_status.status = false;
    }
    cJSON_Delete(message_root);

    ESP_LOGI(TAG, "Switch update: status %d, ping %d", device_status.status, device_status.ping);
}

const char *device_get(void)
{
    char *json_str = NULL;

    cJSON *message_root = cJSON_CreateObject();
    cJSON *message_sub = cJSON_CreateObject();

    char device_id[DEVICE_ID_LEN];
    get_device_id(device_id);

    cJSON_AddStringToObject(message_root, "id", device_id);
    cJSON_AddStringToObject(message_root, "type", DEVICE_TYPE);
    cJSON_AddStringToObject(message_sub, "ping", "online");
    if (device_status.status){
        cJSON_AddStringToObject(message_sub, "status", "on");
    } else {
        cJSON_AddStringToObject(message_sub, "status", "off");
    }
    cJSON_AddItemToObject(message_root, "settings", message_sub);


    json_str = cJSON_Print(message_root);

    cJSON_Delete(message_root);

    ESP_LOGI(TAG, "Create JSON struct of size: %d, %s", strlen(json_str), json_str);

    return json_str;
}
