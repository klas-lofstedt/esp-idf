/*
 * ble.h
 *
 *  Created on: 20 jan 2023
 *      Author: klaslofstedt
 */

#ifndef _BLE_H_
#define _BLE_H_

#define BLE_POP_SIZE            16

void ble_init(void);
void ble_start(void);
bool ble_notify_wifi_scan(void);
bool ble_notify_provisioning_status(bool status);
bool ble_notify_ca_cert(bool status);
void parse_ble_json_message(char *ble_message);



#endif /* _BLE_H_ */
