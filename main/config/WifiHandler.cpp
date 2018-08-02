/*
 * WifiHandler.cpp
 *
 *  Created on: 18 de mai de 2018
 *      Author: makara
 */

#include "WifiHandler.h"

#include <stdio.h>
#include <string.h>
#include "esp_types.h"
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
#include "../ui/PopUpper.h"


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

  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_init(&configs));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  ESP_ERROR_CHECK(esp_wifi_start());

  records_count = 0;
  records = NULL;
  records_manual_semaphore = 0;
  xTaskCreate(task, "WifiHandler", 2048, this, 1, &handle);
}



void WifiHandler::getWifiScan(wifi_ap_record_t **_records, int *_count){
  while(records_manual_semaphore<0)vTaskDelay(1);
  records_manual_semaphore++;
  *_records = records;
  *_count = records_count;
}

void WifiHandler::freeWifiScan(){
  if(records_manual_semaphore>0)records_manual_semaphore--;
}

void WifiHandler::task(void *arg){
  WifiHandler *wh = (WifiHandler*) arg;
  while(1){
    int times = CM::get()->get(CM::WIFI_SCAN_TIMES);
    if(times>0){
      uint16_t num_found=0;
      wifi_scan_config_t config;
      wifi_ap_record_t *records;
      memset(&config,0,sizeof(config));
      config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
      config.scan_time.active.min = 100;
      config.scan_time.active.max = 1500;
      config.show_hidden = false;
      esp_wifi_scan_start(&config, true);
      ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&num_found));
      records = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t)*num_found);
      if(records==NULL)continue;
      ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&num_found, records));
      printf("%d stations found\n",num_found);
      while(wh->records_manual_semaphore>0)vTaskDelay(1);
      wh->records_manual_semaphore = -1;
      free(wh->records);
      wh->records = records;
      wh->records_count = num_found;
      wh->records_manual_semaphore = 0;
      CM::get()->set(CM::WIFI_SCAN_TIMES, times-1);
    }else{
      vTaskDelay(100);
    }
  }
}

void WifiHandler::updateAP(){
  wifi_config_t wifi_config;
  memset(&wifi_config,0,sizeof(wifi_config));
  CM::get()->get(CM::WIFI_AP_SSID,(char*)wifi_config.ap.ssid);
  if(wifi_config.ap.ssid[0]==0)return;
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
  printf("AP reset\n");
}

void WifiHandler::updateSTA(){
  wifi_config_t wifi_config;
  char buff[33];
  memset(&wifi_config,0,sizeof(wifi_config));
  CM::get()->get(CM::WIFI_STA_SSID,(char*)wifi_config.sta.ssid);
  if(wifi_config.sta.ssid[0]==0){
    esp_wifi_disconnect();
    return;
  }
  CM::get()->get(CM::WIFI_STA_PASSWD,(char*)wifi_config.sta.password);
  if(strlen((char*)wifi_config.sta.password)==0){
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WEP;
  }else{
    wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
  }
  wifi_config.sta.scan_method = WIFI_FAST_SCAN;
  wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
  wifi_config.sta.threshold.rssi = -127;


  // Begin of mDNS (only available to STA mode)
  CM::get()->get(CM::WIFI_DEVICENAME,buff);
  tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, buff);

  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_connect());
  printf("STA reset\n");
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
    CM::get()->set(CM::WIFI_STA_IP,(int32_t)event->event_info.got_ip.ip_info.ip.addr);
    CM::get()->set(CM::WIFI_STA_GW,(int32_t)event->event_info.got_ip.ip_info.gw.addr);
    CM::get()->set(CM::WIFI_STA_NM,(int32_t)event->event_info.got_ip.ip_info.netmask.addr);
    xEventGroupSetBits(get()->wifi_event_group, WIFI_CONNECTED_BIT);
    PopUpper::get()->popup(10000,0,"My IP is %s\n",ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
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
    //esp_wifi_connect();
    xEventGroupClearBits(get()->wifi_event_group, WIFI_CONNECTED_BIT);
    break;
  default:
    break;
  }
  mdns_handle_system_event(ctx, event);
  return ESP_OK;
}
