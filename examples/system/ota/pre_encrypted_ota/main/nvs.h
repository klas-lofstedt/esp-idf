/*
 * nvs.h
 *
 *  Created on: 20 jan 2023
 *      Author: klaslofstedt
 */

#ifndef _NVS_H_
#define _NVS_H_

#include <stdbool.h>

bool nvs_wifi_is_provisioned(void);
bool nvs_wifi_set_pwd(const char* str);
bool nvs_wifi_get_pwd(uint8_t *buffer);
bool nvs_wifi_set_default(void);
void nvs_init(void);

#endif /* _NVS_H_ */
