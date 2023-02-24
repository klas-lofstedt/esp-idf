// C
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#include "device.h"

// FreeRTOS
// ESP32
#include <esp_wifi.h>
#include "esp_mac.h"
// App
#include "misc.h"

static const char *TAG = "MISC";

#define HEX_TO_INT(c) ((c) >= '0' && (c) <= '9' ? (c) - '0' : ((c) >= 'a' && (c) <= 'f' ? (c) - 'a' + 10 : ((c) >= 'A' && (c) <= 'F' ? (c) - 'A' + 10 : -1)))


void hex_string_from_u8_array(const uint8_t *in, char *out, size_t size)
{
    for (int i = 0; i < size; i++) {
        sprintf(out + 2 * i, "%02x", in[i]);
    }
}

bool u8_array_from_hex_string(char *in, char *out, size_t size)
{
    size_t i;
    for (i = 0; i < size; ++i) {
        int high_nibble = HEX_TO_INT(in[2 * i]);
        int low_nibble = HEX_TO_INT(in[2 * i + 1]);
        if (high_nibble < 0 || low_nibble < 0) {
            return false;
        }
        out[i] = (high_nibble << 4) | low_nibble;
    }
    return true;
}

void ascii_string_from_u8_array(const uint8_t *bytes, char *ascii_string, size_t len)
{
    for (int i = 0; i < len; i++) {
        ascii_string[i] = (char)bytes[i];
    }
    //ascii_string[len] = '\0';
}

// POP: a8af2c56b75273f245b3e7caf7c447d3
// AES: 98e3639932d8b6e23bd67f42557d1c0c
void generate_random_u8_array(uint8_t *out, size_t size)
{
    uint8_t mac8[6];
    esp_efuse_mac_get_default(mac8);
    int64_t mac64 = 0;
    memcpy(&mac64, mac8, 4);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "TIME1(): %llu", time(NULL));
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "TIME2(): %llu", time(NULL));
    srand(time(NULL) ^ mac64);
    //srand(time(NULL));
    for (int i = 0; i < size; i++) {
        uint8_t r = rand() & 0xff;
        while (r == 0) {
            r = rand() & 0xff;
        }
        out[i] = r;
    }
}

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
