/*
 * mqtt.h
 *
 *  Created on: 15 sep 2022
 *      Author: klaslofstedt
 */
#ifndef _MQTT_H_
#define _MQTT_H_


//void mqtt_task(void *arg);
void mqtt_init(void);
void mqtt_data_send(char *data);
const char* mqtt_data_received(void);


#endif /* _MQTT_H_ */
