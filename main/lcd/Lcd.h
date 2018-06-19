/*
 * Lcd.h
 *
 *  Created on: 28 de abr de 2018
 *      Author: makara
 */

#ifndef MAIN_LCD_LCD_H_
#define MAIN_LCD_LCD_H_

#include "Sprite.h"
#include "ScreenElement.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

class Lcd: public Sprite {
public:
	static Lcd* get();
	void setup();
	void refresh();
	void uart_refresh();

	static const uint8_t BUTTON_OK=(1<<2);
	static const uint8_t BUTTON_RIGHT=(1<<3);
	static const uint8_t BUTTON_BACK=(1<<4);
	static const uint8_t BUTTON_LEFT=(1<<5);
	static const uint8_t BUTTON_UP=(1<<6);
	static const uint8_t BUTTON_DOWN=(1<<7);
	static const uint8_t BUTTONS_USED=0xFC;

	uint8_t buttons_get(uint8_t buttons=BUTTONS_USED);
	uint8_t buttons_waitPress(uint8_t buttons, uint32_t timeout=0);
	uint8_t buttons_waitRelease(uint8_t buttons, uint32_t timeout=0);

	void setContrast(int32_t bias=-1, int32_t vop=-1);

	ScreenElement *root;
	uint32_t buttonOverride;

private:
	Lcd(gpio_num_t cs, gpio_num_t miso, gpio_num_t mosi,
			gpio_num_t clk , gpio_num_t rst, gpio_num_t dc);
	void spi_send(uint8_t byte, uint8_t dc);
	void spi_sendFullData();
	uint8_t mem[(48/8)*(84)];
	spi_device_handle_t spi;
	gpio_num_t rst_pin, dc_pin, cs_pin, miso_pin, mosi_pin, clk_pin;
	static Lcd *singleton;
	SemaphoreHandle_t sem;
	int64_t lastButtonRefresh;
	uint8_t lastButtons;
	uint8_t spi_receive_buttons();
	static void task(void *arg);
	TaskHandle_t t_handle;
};

#endif /* MAIN_LCD_LCD_H_ */
