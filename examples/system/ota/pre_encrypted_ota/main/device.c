#include "device.h"
#include <cJSON.h>
#include "gpio.h"
#include <string.h>
#include "esp_log.h"
#include "misc.h"
#include "nvs.h"
#include "switch.h"

static const char *TAG = "DEVICE";

void device_set(const char* json_data)
{
    uint32_t device_type = nvs_get_device_type();
    switch(device_type){
        case DEVICE_TYPE_SWITCH: return switch_set(json_data);
        default: return;
    }
}

// Remember to free returned json_str after it has been used
char *device_get(void)
{
    uint32_t device_type = nvs_get_device_type();
    switch(device_type){
        case DEVICE_TYPE_SWITCH: return switch_get();
        default: return "TYPE_UNKNOWN";
    }
}
