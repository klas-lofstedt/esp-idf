/*
 * ble.h
 *
 *  Created on: 20 jan 2023
 *      Author: klaslofstedt
 */

#ifndef _BLE_H_
#define _BLE_H_

typedef enum
{
    WAITING_CONNECTION = 0,
    WAITING_POP = 1,
    WAITING_CRED = 2,
    WAITING_CERT = 3,
} ble_status_t;

void ble_init(void);
void ble_start(void);
int ble_notify_scan(int16_t conn_handle);//, char* ble_data);

#endif /* _BLE_H_ */
