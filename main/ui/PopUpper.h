/*
 * PopUpper.h
 *
 *  Created on: 26 de jun de 2018
 *      Author: makara
 */

#ifndef MAIN_UI_POPUPPER_H_
#define MAIN_UI_POPUPPER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "../lcd/ScreenElement.h"

class PopUpper {
public:
  static PopUpper* get();
  void setZ(int _z);
  int getZ();
  void popup(const char *msg, uint32_t time=0, uint8_t size=0);
  void popup(uint32_t time, uint8_t size, const char *fmt, ...);

  uint32_t DefaultTime;
  uint8_t DefaultSize;
protected:
  struct POPUP {
    char msg[128];
    uint32_t time;
    uint8_t size;
  };
  PopUpper();
  QueueHandle_t queue;
  ScreenElement screen;
  static void task(void *arg);
  TaskHandle_t t_handle;
  static PopUpper *singleton;
};

#endif /* MAIN_UI_POPUPPER_H_ */
