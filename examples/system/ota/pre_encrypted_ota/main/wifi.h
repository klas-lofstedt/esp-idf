/*
 * wifi.h
 *
 *  Created on: 20 jan 2023
 *      Author: klaslofstedt
 */

#ifndef _WIFI_H_
#define _WIFI_H_

#define WIFI_SSID_MAX_LEN 32
#define WIFI_PWD_MAX_LEN 64
#define WIFI_MAX_SCAN_RESULTS 20 // TODO test with different sizes

bool wifi_start_scan(void);
void wifi_stop_scan(void);
void wifi_init(void);
void wifi_reinit(void);
bool wifi_connect(void);

#endif /* _WIFI_H_ */
