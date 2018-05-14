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
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

#include "esp32/ulp.h"

extern uint32_t ulp_m_count, ulp_entry;
extern const uint8_t bin_start[] asm("_binary_ulp_audio_bin_start");
extern const uint8_t bin_end[]   asm("_binary_ulp_audio_bin_end");

#include "soc/timer_group_struct.h"


#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL_SEC    (1.0/10000) // sample test interval for the first timer
#define TIMER_INDEX           TIMER_1

AdcReader* AdcReader::instance = 0;
AdcReader* AdcReader::get(){
	if (instance == 0)instance = new AdcReader();
	return instance;
}
AdcReader::AdcReader() {
    timer_queue = xQueueCreate(10, sizeof(int));
    counter = 0;
    ulp_addr = 0;
    xTaskCreate(task, "AdcReader", 2048, NULL, 1, &handle);
}

void AdcReader::task(void *arg){
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = 1;
    timer_init(TIMER_GROUP_0, TIMER_INDEX, &config);

    timer_set_counter_value(TIMER_GROUP_0, TIMER_INDEX, 0x00000000ULL);

    timer_set_alarm_value(TIMER_GROUP_0, TIMER_INDEX, TIMER_INTERVAL_SEC * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, TIMER_INDEX);
    timer_isr_register(TIMER_GROUP_0, TIMER_INDEX, timerCallback,
        (void *) TIMER_INDEX, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, TIMER_INDEX);

    //Init ULP executable
    ESP_ERROR_CHECK( ulp_load_binary(
            0,
            bin_start,
            (bin_end - bin_start) / sizeof(uint32_t)) );
    get()->ulp_addr = (&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t);

	while(1){
        //int evt;
        vTaskDelay(1000);
        //xQueueReceive(get()->timer_queue, &evt, portMAX_DELAY);
        printf("Total: %d, ulp=(%d,%d)\n",get()->counter,ulp_m_count>>16, ulp_m_count&0xFFFF);
	}
}

void IRAM_ATTR AdcReader::timerCallback(void *arg){

	ulp_run(get()->ulp_addr);

    get()->counter++;

    TIMERG0.int_clr_timers.t1 = 1;
    TIMERG0.hw_timer[TIMER_INDEX].config.alarm_en = TIMER_ALARM_EN;
}
