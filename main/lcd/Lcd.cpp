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
#include "../config/ConfigManager.h"
#include <string.h>
#include <stdio.h>

#ifndef CONFIG_GLOBAL_LCD_PRIORITY
#define CONFIG_GLOBAL_LCD_PRIORITY 1
#endif

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
void lcd_spi_pre_transfer_callback(spi_transaction_t *t){
  int dc=((int)t->user)&1;
  gpio_num_t pin = (gpio_num_t)(((int)t->user)>>8);
  if(pin)gpio_set_level(pin, dc);
}
}

Lcd* Lcd::singleton = 0;
Lcd* Lcd::get(){
  if(singleton == 0)singleton = new Lcd(GPIO_NUM_5,GPIO_NUM_19,GPIO_NUM_23,GPIO_NUM_18,GPIO_NUM_21,GPIO_NUM_22);
  return singleton;
}

Lcd::Lcd(gpio_num_t cs, gpio_num_t miso, gpio_num_t mosi,
    gpio_num_t clk , gpio_num_t rst, gpio_num_t dc) : Sprite(mem,84,48,6) {

  lastButtonRefresh = 0;
  lastButtons = 0;
  buttonOverride = 0;

  sem = xSemaphoreCreateBinary();
  xSemaphoreGive( sem );

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

  root = ScreenElement::createList(NULL);

  setup();
}

void Lcd::spi_send(uint8_t byte, uint8_t dc){
  spi_transaction_t t;
  xSemaphoreTake( sem, portMAX_DELAY );
  gpio_set_level(cs_pin, 0);
  memset(&t, 0, sizeof(t));       //Zero out the transaction
  t.length=8;                     //Command is 8 bits
  t.rxlength=8;
  t.tx_buffer=&byte;              //The data is the cmd itself
  t.rx_buffer=&lastButtons;
  t.user=(void*)(dc|(dc_pin<<8)); //Sends the d/c level
  lastButtonRefresh = esp_timer_get_time();
  spi_device_transmit(spi, &t);  //Transmit!
  gpio_set_level(cs_pin, 1);
  xSemaphoreGive( sem );
}

uint8_t Lcd::spi_receive_buttons(){
  spi_transaction_t t;
  uint8_t sent=0, received = 0xAA;
  xSemaphoreTake( sem, portMAX_DELAY );
  gpio_set_level(cs_pin, 0);
  memset(&t, 0, sizeof(t));
  t.length=8;
  t.rxlength=8;
  t.tx_buffer = &sent;
  t.rx_buffer=&lastButtons;
  t.user = (void*)(0|(dc_pin<<8));
  lastButtonRefresh = esp_timer_get_time();
  spi_device_transmit(spi, &t);
  gpio_set_level(cs_pin, 1);
  xSemaphoreGive( sem );
  return received;
}

void Lcd::spi_sendFullData(){
  spi_transaction_t t;
  xSemaphoreTake( sem, portMAX_DELAY );
  gpio_set_level(cs_pin, 0);
  memset(&t, 0, sizeof(t));
  t.length=84*48;
  t.rxlength=8;
  t.tx_buffer=memory();
  t.rx_buffer=&lastButtons;
  t.user=(void*)(1|(dc_pin<<8));
  lastButtonRefresh = esp_timer_get_time();
  spi_device_transmit(spi, &t);
  gpio_set_level(cs_pin, 1);
  xSemaphoreGive( sem );
}


void Lcd::setContrast(int32_t bias, int32_t vop){
  if(bias>=8 || bias<0)bias = 3;
  if(vop>=128 || vop<0)vop = 66;
  spi_send(LCD_FUNCTION|LCD_FUNCTION_EXTEN|LCD_FUNCTION_VADDR,0);//Enter extended mode
  spi_send(LCD_EXT_VOP|vop,0);
  spi_send(LCD_EXT_BIAS|bias,0);
  spi_send(LCD_FUNCTION|LCD_FUNCTION_VADDR,0);//Back to default mode, display on, vertical addr
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
  spi_send(LCD_FUNCTION|LCD_FUNCTION_VADDR,0);//Back to default mode, display on, vertical addr
  spi_send(LCD_DISPLAY_NORMAL,0);
  setContrast();

  refresh();

  xTaskCreate(task, "LcdRefresher", 2048, this, CONFIG_GLOBAL_LCD_PRIORITY, &t_handle);
}

void Lcd::task(void* arg){
  Lcd *lcd = (Lcd*) arg;
  while(true){
    lcd->clear();
    lcd->root->renderInto(lcd);
    lcd->refresh();
    vTaskDelay(20);
  }
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

uint8_t Lcd::buttons_get(uint8_t buttons){
  if(lastButtonRefresh+1000*1000<esp_timer_get_time())spi_receive_buttons();
  if(buttonOverride&(~(uint32_t)BUTTONS_USED)){
    return buttonOverride & buttons;
  }else{
    return lastButtons & buttons;
  }
}

uint8_t Lcd::buttons_waitPress(uint8_t buttons, uint32_t timeout){
  uint8_t lastRead=0xFF, read=0xFF, op;
  int64_t end = (timeout>0)?(timeout*1000+esp_timer_get_time()):(INT64_MAX);
  while(end>esp_timer_get_time()){
    read = buttons_get();
    op = read & (~lastRead);
    if(op)return op;
    lastRead = read;
    vTaskDelay(5);
  }
  return 0;
}

uint8_t Lcd::buttons_waitRelease(uint8_t buttons, uint32_t timeout){
  uint8_t lastRead=0, read=0, op;
  int64_t end = (timeout>0)?(timeout*1000+esp_timer_get_time()):(INT64_MAX);
  while(end>esp_timer_get_time()){
    read = buttons_get();
    op = lastRead & (~read);
    if(op)return op;
    lastRead = read;
    vTaskDelay(5);
  }
  return 0;
}

