/*
 * AdcReader.cpp
 *
 *  Created on: 11 de mai de 2018
 *      Author: makara
 */

#include "AdcReader.h"

#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
//#include "driver/periph_ctrl.h"
//#include "driver/timer.h"

#include "driver/adc.h"
#include "esp32/ulp.h"

#define ULP_BUFFER_SIZE 32
#define ULP_BUFFER_MASK (ULP_BUFFER_SIZE-1)

extern uint32_t ulp_adcreader_current_pos;
extern uint32_t ulp_adcreader_readings_mic1;
extern uint32_t ulp_adcreader_readings_mic2;
extern uint32_t ulp_entry;
extern const uint8_t bin_start[] asm("_binary_ulp_audio_bin_start");
extern const uint8_t bin_end[]   asm("_binary_ulp_audio_bin_end");

const uint32_t *adcReaderMic1 = &ulp_adcreader_readings_mic1;
const uint32_t *adcReaderMic2 = &ulp_adcreader_readings_mic2;

#include "soc/timer_group_struct.h"

#ifndef CONFIG_GLOBAL_ADCREADER_PRIORITY
#define CONFIG_GLOBAL_ADCREADER_PRIORITY 1
#endif

AdcReader* AdcReader::singleton = 0;
AdcReader* AdcReader::get(){
  if (singleton == 0)singleton = new AdcReader();
  return singleton;
}
AdcReader::AdcReader() {
  timer_queue = xQueueCreate(10, sizeof(int));
  counter = 0;
  ulp_addr = 0;
  bufferUsed = 0;
  bufferPosition = 0;
  for(uint32_t i=0; i<MAX_BUFFER_SIZE; i++){
    buffers[0][0][i] = 0xAA;
    buffers[0][1][i] = 0xAA;
    buffers[1][0][i] = 0xAA;
    buffers[1][1][i] = 0xAA;
  }
  xTaskCreate(task, "AdcReader", 2048, NULL, CONFIG_GLOBAL_ADCREADER_PRIORITY, &handle);
}

void AdcReader::task(void *arg){
  //Init ULP executable


  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);
  adc1_config_channel_atten(ADC1_CHANNEL_1,ADC_ATTEN_DB_0);
  adc1_ulp_enable();

  ESP_ERROR_CHECK( ulp_load_binary(
      0,
      bin_start,
      (bin_end - bin_start) / sizeof(uint32_t)) );
  get()->ulp_addr = (&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t);
  ulp_run(get()->ulp_addr);

  uint16_t lastPos = 0;
  while(1){
    while(lastPos!=(ulp_adcreader_current_pos & ULP_BUFFER_MASK)){
      get()->buffers[0][get()->bufferUsed][get()->bufferPosition] = adcReaderMic1[lastPos]&0xFF;
      get()->buffers[1][get()->bufferUsed][get()->bufferPosition] = adcReaderMic2[lastPos]&0xFF;
      get()->bufferPosition++;
      if(get()->bufferPosition>=MAX_BUFFER_SIZE){
        get()->bufferPosition = 0;
        get()->bufferUsed = (get()->bufferUsed)?0:1;
      }
      lastPos = (lastPos+1)&ULP_BUFFER_MASK;
    }
    vTaskDelay(1);

    //taskYIELD();
  }
}
