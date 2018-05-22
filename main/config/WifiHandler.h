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

protected:
	WifiHandler();
	EventGroupHandle_t wifi_event_group;
	static WifiHandler* singleton;
	static esp_err_t event_handler(void *ctx, system_event_t *event);
};

#endif /* MAIN_CONFIG_WIFIHANDLER_H_ */
