/*
 * nvs.h
 *
 *  Created on: 20 jan 2023
 *      Author: klaslofstedt
 */

#ifndef _NVS_H_
#define _NVS_H_

#include <stdbool.h>

void nvs_set_ble_pop(void);
void nvs_set_aes_key(void);
void nvs_get_ble_pop_arr(uint8_t *pop);
void nvs_get_ble_pop_str(char *pop);
void nvs_get_aes_key_arr(uint8_t *aes);
void nvs_get_aes_key_str(char *aes);
bool nvs_wifi_set_provisioned(bool set_provision);
bool nvs_wifi_is_provisioned(void);
bool nvs_wifi_set_pwd(const char* str);
bool nvs_wifi_get_pwd(uint8_t *buffer);
bool nvs_wifi_set_ssid(const char* str);
bool nvs_wifi_get_ssid(uint8_t *buffer);
bool nvs_wifi_set_default(void);
void nvs_init(void);

#endif /* _NVS_H_ */
