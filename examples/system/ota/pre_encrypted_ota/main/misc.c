// C
#include <string.h>
#include <stdint.h>
// FreeRTOS
// ESP32
#include <esp_wifi.h>
#include "esp_system.h"
// App
#include "misc.h"

void get_device_id(char *device_id)
{
    uint8_t mac[6];
    const char *prefix = "id";
    esp_efuse_mac_get_default(mac);
    snprintf(device_id, DEVICE_ID_LEN, "%s%02X%02X%02X%02X%02X%02X",
             prefix, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

bool is_device_id_valid()
{
    char device_id[DEVICE_ID_LEN];
    get_device_id(device_id);

    // Return false if the mac address is all zeros
    if (strncmp(device_id, "id000000000000", DEVICE_ID_LEN) == 0){
        return false;
    }
    return true;
}