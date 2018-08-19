/*
 * Player.cpp
 *
 *  Created on: 27 de jul de 2018
 *      Author: makara
 */

#include "Player.h"
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "driver/i2s.h"
#include <math.h>
#include "../config/ConfigManager.h"

#define I2S_NUM         (I2S_NUM_0)
#define WAVE_FREQ_HZ    (330)
#define PI 3.14159265
#define SAMPLE_PER_CYCLE (SAMPLE_RATE/WAVE_FREQ_HZ)

#ifndef CONFIG_GLOBAL_PLAYER_PRIORITY
#define CONFIG_GLOBAL_PLAYER_PRIORITY 2
#endif

Player* Player::singleton=NULL;

Player* Player::get(){
  if(singleton==NULL)singleton = new Player();
  return singleton;
}

Player::Player() {
  static const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, /* the DAC module will only take the 8bits from MSB */
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_I2S_MSB,
      .intr_alloc_flags = 0, // default interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = true,
      .fixed_mclk = SAMPLE_RATE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);   //install and start i2s driver

  i2s_set_pin(I2S_NUM_0, NULL); //for internal DAC, this will enable both of the internal channels

  //You can call i2s_set_dac_mode to set built-in DAC output mode.
  i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);

  i2s_set_sample_rates(I2S_NUM_0, 44100); //set sample rates
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);
  i2s_zero_dma_buffer(I2S_NUM_0);

  //i2s_driver_uninstall(i2s_num); //stop & destroy i2s driver
  sampleSourceFF = NULL;
  sampleSourcePP = NULL;
  sampleCount = 0;;
  samples_sem = xSemaphoreCreateBinary();
  sampleBuffer = (uint32_t*)malloc(4*INTERNAL_BUFFER_SIZE);

  if(samples_sem==NULL || sampleBuffer==NULL){
    printf("Fail to allocate resources for Player\n");
  }else{
    xTaskCreate(task, "Player", 2048, this, CONFIG_GLOBAL_PLAYER_PRIORITY, &t_handle);
  }
}

void Player::task(void *arg){
  Player::get()->play(arg);
}
void Player::play(void *arg){
  int32_t freqVelha_10=0, freqNova_10=0;
  int32_t ampliVelha=0, ampliNova=0;
  int32_t inPosMax=0, inPosMax_FXP16=0, inPos_FXP16=0;
  int32_t freq_FXP16=0, stepFreq_FXP16=0;
  int32_t ampliFF_FXP16=0, ampliPP_FXP16=0;
  int32_t stepAmpliFF_FXP16=0, stepAmpliPP_FXP16=0;

  float pVar;
  CM *cm = CM::get();
  while(true){
    freqNova_10 = cm->get(CM::PLAYR_FREQ);
    ampliNova = cm->get(CM::PLAYR_VOLUME);
    if(freqVelha_10==0){
      freqVelha_10 = freqNova_10;
    }
    if(ampliNova==0 || freqNova_10==0){
      ampliNova = 0;
      freqNova_10 = freqVelha_10;
    }
    for(int i=0;i<INTERNAL_BUFFER_SIZE;i++){
      sampleBuffer[i] = 0x80008000;//Hold lines at middle
    }
    bool taken=xSemaphoreTake(samples_sem,0);
    if( !taken ||
        sampleCount==0 ||
        sampleSourceFF==NULL ||
        sampleSourcePP==NULL ||
        (ampliVelha==0 && ampliNova==0)){
      if(taken)xSemaphoreGive(samples_sem);
    }else{
      if(inPosMax!=sampleCount){
        inPosMax = sampleCount;
        inPosMax_FXP16 = sampleCount<<16;
        inPos_FXP16 = 0;
      }
      pVar = ((float)freqVelha_10)*((float)inPosMax_FXP16);
      pVar /= (float)20*SAMPLE_RATE;
      freq_FXP16 = pVar;

      pVar = ((float)freqNova_10-freqVelha_10)*((float)(1<<16));
      pVar /= (float)20*SAMPLE_RATE*INTERNAL_BUFFER_SIZE;
      stepFreq_FXP16 = pVar;

      ampliFF_FXP16 = ampliVelha<<16;
      pVar = (float)((ampliNova-ampliVelha)<<16);
      pVar /= INTERNAL_BUFFER_SIZE;
      stepAmpliFF_FXP16 = pVar;

      ampliPP_FXP16 = ((255-ampliVelha)<<14);
      pVar = (float)((ampliVelha-ampliNova)<<14);
      pVar /= INTERNAL_BUFFER_SIZE;
      stepAmpliPP_FXP16 = pVar;

      for(int i=0;i<INTERNAL_BUFFER_SIZE;i++){
        uint32_t pos = inPos_FXP16>>16;
        uint16_t *p = (uint16_t*)(sampleBuffer+i);
        int16_t audio = ((int32_t)sampleSourceFF[pos]*ampliFF_FXP16)>>16;
        //For now, play at both channels
        p[0] += audio;
        p[1] += audio;
        //Increment the deltas
        inPos_FXP16 = (inPos_FXP16+freq_FXP16)%inPosMax_FXP16;
        ampliFF_FXP16 += stepAmpliFF_FXP16;
        ampliPP_FXP16 += stepAmpliPP_FXP16;
        freq_FXP16 += stepFreq_FXP16;
      }
      xSemaphoreGive(samples_sem);
    }
    freqVelha_10 = freqNova_10;
    ampliVelha = ampliNova;
    //TODO: Calculate the position and add the metronome here
    //
    size_t written = 0;
    i2s_write(I2S_NUM_0, (const void *)sampleBuffer, 4*INTERNAL_BUFFER_SIZE, &written, portMAX_DELAY); // @suppress("Function cannot be resolved")
  }
}

void Player::startup_sine_waves(){
  xSemaphoreTake(samples_sem,portMAX_DELAY);
  if(sampleSourceFF!=NULL)free(sampleSourceFF);
  if(sampleSourcePP!=NULL)free(sampleSourcePP);
  sampleSourceFF = (int8_t*)malloc(360*sizeof(int8_t));
  sampleSourcePP = (int8_t*)malloc(360*sizeof(int8_t));
  sampleCount = 360;
  for(int i=0;i<360;i++){
    float v = 127*sin(i*2*PI/360);
    sampleSourceFF[i] = v;
    sampleSourcePP[i] = v;
  }
  xSemaphoreGive(samples_sem);
}
