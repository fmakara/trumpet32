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
  static const uint32_t MAX_BUFFER_SIZE = 5000;
  uint8_t bufferUsed;
  uint8_t buffers[2][2][MAX_BUFFER_SIZE];
protected:
  uint32_t bufferPosition;
  AdcReader();
  QueueHandle_t timer_queue;
  TaskHandle_t handle;
  uint32_t ulp_addr;
  static void task(void *arg);
  static AdcReader* singleton;
};

#endif /* MAIN_AUDIO_ADCREADER_H_ */
