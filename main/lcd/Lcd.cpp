/*
 * Lcd.cpp
 *
 *  Created on: 28 de abr de 2018
 *      Author: makara
 */

#include "Lcd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/spi_master.h"
#include <string.h>
#include <stdio.h>

#define LCD_FUNCTION         (1<<5)
#define LCD_FUNCTION_PDOWN   (1<<2)
#define LCD_FUNCTION_VADDR   (1<<1)
#define LCD_FUNCTION_EXTEN   (1<<0)
#define LCD_DISPLAY_BLANK    (1<<3)
#define LCD_DISPLAY_NORMAL   ((1<<3)|(1<<2))
#define LCD_DISPLAY_BLACK    ((1<<3)|(1<<0))
#define LCD_DISPLAY_INVERT   ((1<<3)|(1<<2)|(1<<0))
#define LCD_SETY  (1<<6)
#define LCD_SETX  (1<<7)

#define LCD_EXT_TC    (1<<2)
#define LCD_EXT_BIAS  (1<<4)
#define LCD_EXT_VOP   (1<<7)

extern "C" {
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=((int)t->user)&1;
    gpio_num_t pin = (gpio_num_t)(((int)t->user)>>8);
    gpio_set_level(pin, dc);
}
}

Lcd* Lcd::singleton = 0;
Lcd* Lcd::get(){
	if(singleton == 0)singleton = new Lcd(GPIO_NUM_5,GPIO_NUM_19,GPIO_NUM_23,GPIO_NUM_18,GPIO_NUM_21,GPIO_NUM_22);
	return singleton;
}

Lcd::Lcd(gpio_num_t cs, gpio_num_t miso, gpio_num_t mosi,
		gpio_num_t clk , gpio_num_t rst, gpio_num_t dc) : Sprite(mem,84,48,6) {
	clear();

	cs_pin = cs;
	miso_pin = miso;
	mosi_pin = mosi;
	clk_pin = clk;
	rst_pin = rst;
	dc_pin = dc;

    esp_err_t ret;
    spi_bus_config_t buscfg;
    spi_device_interface_config_t devcfg;

    memset(&buscfg, 0, sizeof(buscfg));
    memset(&devcfg, 0, sizeof(devcfg));

    buscfg.miso_io_num=miso_pin;
    buscfg.mosi_io_num=mosi_pin;
    buscfg.sclk_io_num=clk_pin;
    buscfg.quadwp_io_num=-1;
    buscfg.quadhd_io_num=-1;
    buscfg.max_transfer_sz=84*6*2;

    devcfg.clock_speed_hz=4*1000*1000;           //Clock out at 4 MHz
    devcfg.mode=0;                                //SPI mode 0
    devcfg.spics_io_num=-1;//cs_pin;               //CS pin
    devcfg.queue_size=7;                          //We want to be able to queue 7 transactions at a time
    devcfg.pre_cb=lcd_spi_pre_transfer_callback;  //Specify pre-transfer callback to handle D/C line

    //SPI Init
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);

    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}

void Lcd::spi_send(uint8_t byte, uint8_t dc){
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&byte;               //The data is the cmd itself
    t.user=(void*)(dc|(dc_pin<<8)); //Sends the d/c level
    spi_device_transmit(spi, &t);  //Transmit!
}
void Lcd::spi_sendFullData(){
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=84*48;                     //Command is 8 bits
    t.tx_buffer=memory();               //The data is the cmd itself
    t.user=(void*)(1|(dc_pin<<8)); //Sends the d/c level
    spi_device_transmit(spi, &t);  //Transmit!
}

void Lcd::setup(){
    //GPIO Init
    gpio_pad_select_gpio(rst_pin);
    gpio_pad_select_gpio(dc_pin);
    gpio_pad_select_gpio(cs_pin);
    gpio_set_direction(rst_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(dc_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(cs_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dc_pin, 0);
    gpio_set_level(rst_pin, 0);
    gpio_set_level(cs_pin, 1);
    vTaskDelay(10);
    gpio_set_level(rst_pin, 1);
    vTaskDelay(10);
    gpio_set_level(cs_pin, 0);
    //LCD init
    spi_send(0,0);
    spi_send(LCD_FUNCTION|LCD_FUNCTION_EXTEN|LCD_FUNCTION_VADDR,0);//Enter extended mode
    spi_send(LCD_EXT_VOP|66,0);
    spi_send(LCD_EXT_BIAS|3,0);
    spi_send(LCD_FUNCTION|LCD_FUNCTION_VADDR,0);//Back to default mode, display on, vertical addr
    spi_send(LCD_DISPLAY_NORMAL,0);

    refresh();
}

void Lcd::refresh(){
    spi_send(LCD_SETY,0);
    spi_send(LCD_SETX,0);
    spi_sendFullData();
}

void Lcd::uart_refresh(){
	int x, y;
	printf("\n");
	for(x=0;x<width();x++)printf("-");
	printf("\n");
	for(y=0;y<height();y++){
		for(x=0;x<width();x++){
			if(at(x,y))printf("%%");
			else printf(" ");
		}
		printf("\n");
	}
	for(x=0;x<width();x++)printf("-");
	printf("\n");
}
