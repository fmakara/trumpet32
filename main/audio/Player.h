/*
 * Player.h
 *
 *  Created on: 27 de jul de 2018
 *      Author: makara
 */

#ifndef MAIN_AUDIO_PLAYER_H_
#define MAIN_AUDIO_PLAYER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

class Player {
public:
  static Player* get();
  void startup_sine_waves();
  static const uint32_t SAMPLE_RATE = 44100;
  static const uint32_t REFRESH_RATE_HZ = 100;
  static const uint32_t INTERNAL_BUFFER_SIZE = SAMPLE_RATE/REFRESH_RATE_HZ;
protected:
  int test_bits=16;
  static Player* singleton;
  Player();
  static void task(void *arg);
  void play(void *arg);
  TaskHandle_t t_handle;
  int8_t *sampleSourceFF;
  int8_t *sampleSourcePP;
  uint32_t sampleCount;
  uint32_t *sampleBuffer;
  SemaphoreHandle_t samples_sem;
};

#endif /* MAIN_AUDIO_PLAYER_H_ */
