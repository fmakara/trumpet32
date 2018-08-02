/*
 * WifiHandler.h
 *
 *  Created on: 18 de mai de 2018
 *      Author: makara
 */

#ifndef MAIN_CONFIG_WIFIHANDLER_H_
#define MAIN_CONFIG_WIFIHANDLER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"

class WifiHandler {
public:
  static WifiHandler* get();
  void updateAP();
  void updateSTA();
  wifi_ap_record_t *front_buffer;
  void getWifiScan(wifi_ap_record_t **records, int *count);
  void freeWifiScan();
protected:
  WifiHandler();
  static void task(void *arg);
  wifi_ap_record_t *records;
  int records_count;
  int records_manual_semaphore;
  TaskHandle_t handle;
  EventGroupHandle_t wifi_event_group;
  static WifiHandler* singleton;
  static esp_err_t event_handler(void *ctx, system_event_t *event);
};

#endif /* MAIN_CONFIG_WIFIHANDLER_H_ */
