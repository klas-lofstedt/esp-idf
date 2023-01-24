/*
 * misc.h
 *
 *  Created on: 31 dec 2021
 *      Author: klaslofstedt
 */

#ifndef _MISC_H_
#define _MISC_H_

#include <stdbool.h>

#define DEVICE_TYPE ("SWITCH")
#define DEVICE_ID_LEN 15

void get_device_id(char *device_id);
bool is_device_id_valid();

#endif /* _MISC_H_ */