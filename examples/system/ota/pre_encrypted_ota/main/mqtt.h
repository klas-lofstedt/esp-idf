/*
 * mqtt.h
 *
 *  Created on: 15 sep 2022
 *      Author: klaslofstedt
 */
#ifndef _MQTT_H_
#define _MQTT_H_

#include "mqtt_client.h"


void mqtt_task(void *arg);
void mqtt_publish_status(esp_mqtt_client_handle_t client);

#endif /* _MQTT_H_ */
