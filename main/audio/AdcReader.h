/*
 * AdcReader.h
 *
 *  Created on: 11 de mai de 2018
 *      Author: makara
 */

#ifndef MAIN_AUDIO_ADCREADER_H_
#define MAIN_AUDIO_ADCREADER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

class AdcReader {
public:
	static AdcReader* get();
	uint32_t counter;
protected:
	AdcReader();
	QueueHandle_t timer_queue;
	TaskHandle_t handle;
	uint32_t ulp_addr;
	static void task(void *arg);
	static void timerCallback(void *arg);
	static AdcReader* singleton;
};

#endif /* MAIN_AUDIO_ADCREADER_H_ */
