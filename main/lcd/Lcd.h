/*
 * Lcd.h
 *
 *  Created on: 28 de abr de 2018
 *      Author: makara
 */

#ifndef MAIN_LCD_LCD_H_
#define MAIN_LCD_LCD_H_

#include "Sprite.h"
#include "driver/spi_master.h"

class Lcd: public Sprite {
public:
	Lcd(gpio_num_t cs, gpio_num_t miso, gpio_num_t mosi,
			gpio_num_t clk , gpio_num_t rst, gpio_num_t dc);
	void setup();
	void refresh();
	void uart_refresh();
private:
	void spi_send(uint8_t byte, uint8_t dc);
	void spi_sendFullData();
	uint8_t mem[(48/8)*(84)];
	spi_device_handle_t spi;
	gpio_num_t rst_pin, dc_pin, cs_pin, miso_pin, mosi_pin, clk_pin;
};

#endif /* MAIN_LCD_LCD_H_ */
