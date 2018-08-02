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

#include "driver/i2s.h"
#include "esp_system.h"
#include <math.h>

#define SAMPLE_RATE     (44100)
#define I2S_NUM         (I2S_NUM_0)
#define WAVE_FREQ_HZ    (330)
#define PI 3.14159265
#define SAMPLE_PER_CYCLE (SAMPLE_RATE/WAVE_FREQ_HZ)


Player* Player::singleton=NULL;

Player* Player::get(){
	if(singleton==NULL)singleton = new Player();
	return singleton;
}

Player::Player() {
	static const i2s_config_t i2s_config = {
	     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
	     .sample_rate = 44100,
	     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, /* the DAC module will only take the 8bits from MSB */
	     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
	     .communication_format = I2S_COMM_FORMAT_I2S_MSB,
	     .intr_alloc_flags = 0, // default interrupt priority
	     .dma_buf_count = 8,
	     .dma_buf_len = 64,
	     .use_apll = true,
		 .fixed_mclk = 44100
	};

	i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);   //install and start i2s driver

	i2s_set_pin(I2S_NUM_0, NULL); //for internal DAC, this will enable both of the internal channels

	//You can call i2s_set_dac_mode to set built-in DAC output mode.
	i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);

	i2s_set_sample_rates(I2S_NUM_0, 44100); //set sample rates
    i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);

    //i2s_driver_uninstall(i2s_num); //stop & destroy i2s driver

}

void Player::play(){
	setup_triangle_sine_waves(test_bits);
	vTaskDelay(5000);
	//test_bits += 8;
	if(test_bits > 32)
		test_bits = 16;
}

void Player::setup_triangle_sine_waves(int bits){
	i2s_bits_per_sample_t bps;
    short samples_data[2*SAMPLE_PER_CYCLE*4];
    unsigned int i, sample_val;
    double sin_float, triangle_float, triangle_step = (double) pow(2, bits) / SAMPLE_PER_CYCLE;

    printf("\r\nTest bits=%d free mem=%d, written data=%d\n", bits, esp_get_free_heap_size(), ((bits+8)/16)*SAMPLE_PER_CYCLE*4);

    triangle_float = -(pow(2, bits)/2 - 1);

    for(i = 0; i < SAMPLE_PER_CYCLE*4; i++) {
        sin_float = (2<<(bits-2))*(1.0+sin(i *2* PI / (SAMPLE_PER_CYCLE)));
        if(sin_float >= 0)
            triangle_float += triangle_step;
        else
            triangle_float -= triangle_step;


		samples_data[i*2+0] = (short)(sin_float);
		samples_data[i*2+1] = (short)(sin_float);

    }

    switch(bits){ //11.2ms escritos, 3ms cheio, 8.2ms vazio
    case 8:bps = I2S_BITS_PER_SAMPLE_8BIT;break;
    case 16:bps = I2S_BITS_PER_SAMPLE_16BIT;break;
    case 24:bps = I2S_BITS_PER_SAMPLE_24BIT;break;
    case 32:bps = I2S_BITS_PER_SAMPLE_32BIT;break;
    default:bps = I2S_BITS_PER_SAMPLE_16BIT;
    }

    //i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, bps, I2S_CHANNEL_STEREO);
    //Using push
    // for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
    //     if (bits == 16)
    //         i2s_push_sample(0, &samples_data[i], 100);
    //     else
    //         i2s_push_sample(0, &samples_data[i*2], 100);
    // }
    // or write
    i2s_zero_dma_buffer(I2S_NUM_0);
    size_t written=0;
    i2s_write(I2S_NUM_0, (const void *)samples_data, sizeof(samples_data), 2048, (TickType_t)portMAX_DELAY);
    printf("%d wr\n",written);

}
