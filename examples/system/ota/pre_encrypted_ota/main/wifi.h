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


char* wifi_get_scan(void);
void wifi_start_scan(void);
//void wifi_connect_to_network(void);
void wifi_init(void);
void wifi_connect(void);
void wifi_connect_propeller(void);

#endif /* _WIFI_H_ */
