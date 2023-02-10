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

void hex_string_from_u8_array(const uint8_t *in, char *out, size_t size);
bool u8_array_from_hex_string(char *in, char *out, size_t size);
void ascii_string_from_u8_array(const uint8_t *bytes, char *ascii_string, size_t len);
void generate_random_u8_array(uint8_t *out, size_t size);
void get_device_id(char *device_id);
bool is_device_id_valid(void);

#endif /* _MISC_H_ */
