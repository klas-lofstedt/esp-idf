/*
 * gpio.h
 *
 *  Created on: 31 dec 2021
 *      Author: klaslofstedt
 */

#ifndef _GPIO_H_
#define _GPIO_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

extern QueueHandle_t gpio_evt_queue;

void gpio_init(void);

#endif /* _GPIO_H_ */