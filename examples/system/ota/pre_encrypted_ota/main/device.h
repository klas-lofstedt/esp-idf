/*
 * device.h
 *
 *  Created on: 20 feb 2023
 *      Author: klaslofstedt
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define DEVICE_ID_LEN 15

#define DEVICE_TYPE_SWITCH      BIT0
#define DEVICE_TYPE_RGB_LIGHT   BIT1

void device_set(const char *json_data);
char *device_get(void);

#endif /* _DEVICE_H_ */
