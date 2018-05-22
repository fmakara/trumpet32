/*
 * WifiHandler.cpp
 *
 *  Created on: 18 de mai de 2018
 *      Author: makara
 */

#include "WifiHandler.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mdns.h"
#include "driver/gpio.h"
#include <sys/socket.h>
#include <netdb.h>

#include "lwip/err.h"
#include "lwip/sys.h"
#include "ConfigManager.h"


extern void tcpip_adapter_init();

WifiHandler* WifiHandler::singleton = 0;
WifiHandler* WifiHandler::get(){
	if(singleton==0)singleton = new WifiHandler();
	return singleton;
}

WifiHandler::WifiHandler() {
	wifi_init_config_t configs = WIFI_INIT_CONFIG_DEFAULT();
	char buff[60] = "Trumpet32   ";

	// Begin of mDNS (only available to STA mode)
	CM::get()->get(CM::WIFI_DEVICENAME,buff+10);
    ESP_ERROR_CHECK( mdns_init() );
    ESP_ERROR_CHECK( mdns_hostname_set(buff+10) );
    ESP_ERROR_CHECK( mdns_instance_name_set(buff) );

	//structure with TXT records
	mdns_txt_item_t serviceTxtData[3] = {
			{"board","esp32"},
			{"u","user"},
			{"p","password"}
	};

	//initialize service
	ESP_ERROR_CHECK( mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData, 3) );
	//add another TXT item
	ESP_ERROR_CHECK( mdns_service_txt_item_set("_http", "_tcp", "path", "/foobar") );
	//change TXT item value
	ESP_ERROR_CHECK( mdns_service_txt_item_set("_http", "_tcp", "u", "admin") );
	// End of mDNS

    wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
	nvs_flash_init();

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	ESP_ERROR_CHECK(esp_wifi_init(&configs));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_start());
}

void WifiHandler::updateAP(){
	wifi_config_t wifi_config;
	memset(&wifi_config,0,sizeof(wifi_config));
	CM::get()->get(CM::WIFI_AP_SSID,(char*)wifi_config.ap.ssid);
	CM::get()->get(CM::WIFI_AP_PASSWD,(char*)wifi_config.ap.password);
	wifi_config.ap.channel = CM::get()->get(CM::WIFI_AP_CHANNEL);
	if(strlen((char*)wifi_config.ap.password)==0){
		wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	}else{
		wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
	}
	if((((int)CM::get()->get(CM::WIFI_MODE))&2)!=2)wifi_config.ap.ssid_hidden=1;
	wifi_config.ap.max_connection = 4;
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
}

const int WIFI_CONNECTED_BIT = BIT0;
esp_err_t WifiHandler::event_handler(void *ctx, system_event_t *event){
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI("wifi", "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(get()->wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI("wifi", "station:" MACSTR " join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI("wifi", "station:" MACSTR "leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(get()->wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
        break;
    }
    mdns_handle_system_event(ctx, event);
    return ESP_OK;
}
