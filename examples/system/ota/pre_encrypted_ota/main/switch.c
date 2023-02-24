#include "switch.h"
#include <cJSON.h>
#include <stdbool.h>
#include "device.h"
#include "nvs.h"
#include <string.h>
#include "gpio.h"
#include "misc.h"
#include "esp_log.h"


static const char *TAG = "SWITCH";

typedef struct SwitchStatus {
    bool status;
    bool ping;
} SwitchStatus;

SwitchStatus switch_status = { .status = false, .ping = true };

char *switch_get(void)
{
    char *json_str = NULL;

    cJSON *message_root = cJSON_CreateObject();
    cJSON *message_sub = cJSON_CreateObject();

    char device_id[DEVICE_ID_LEN];
    get_device_id(device_id);

    cJSON_AddStringToObject(message_root, "id", device_id);
    cJSON_AddStringToObject(message_root, "type", "SWITCH");
    cJSON_AddStringToObject(message_sub, "ping", "online");
    if (switch_status.status){
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

void switch_set(const char *message_mqtt)
{
    ESP_LOGI(TAG, "Raw message: %s", message_mqtt);

    cJSON *message_root = NULL;
    message_root = cJSON_Parse(message_mqtt);
    if(message_root == NULL){
        ESP_LOGI(TAG, "MQTT subscription message parse fail");
        cJSON_Delete(message_root);
        return;
    }

    char *status = cJSON_GetObjectItem(message_root, "status") ->valuestring;
    char *ping = cJSON_GetObjectItem(message_root, "ping") ->valuestring;
    // Update global device struct
    if (!strcmp(ping, "online")){
        switch_status.ping = true;
    }
    if (!strcmp(ping, "offline")){
        switch_status.ping = false;
    }
    if (!strcmp(status, "on")){
        switch_status.status = true;
    }
    if (!strcmp(status, "off")){
        switch_status.status = false;
    }
    cJSON_Delete(message_root);

    ESP_LOGI(TAG, "Switch update: status %d, ping %d", switch_status.status, switch_status.ping);

    gpio_toggle_led(switch_status.status);
}
