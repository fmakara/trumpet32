/*
 * Webpages.cpp
 *
 *  Created on: 28 de mai de 2018
 *      Author: makara
 */

#include "Webpages.h"

#include <stdio.h>
#include "ConfigManager.h"
#include "WifiHandler.h"
#include "esp_err.h"

void init_cgi(WebService *ws){
	ws->addCgi("/cgi/config.cgi", config_cgi);
	ws->addCgi("/cgi/stations.cgi", stations_cgi);
	ws->addCgi("/cgi/save.cgi", save_cgi);
	ws->addCgi("/cgi/reboot.cgi", reboot_cgi);
}

static const char *reply_fmt =
		"HTTP/1.0 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: %d\r\n"
		"\r\n%s";

void config_cgi(HTTPData *d){
	int len=0, i=0;
	char jsonbuff[2000];
	char nome[32],valor[32], offset=0;
	int print_tudo = d->getGET("tudo",NULL), only_get;
	if(!print_tudo)len += sprintf(jsonbuff+len,"{\n");
	while( (only_get = d->getGET(i,nome,valor,32))!=0 ){
		CM::Config c = CM::CONFIGMANAGER_MAX;
		i++;
		if(nome[0]==0){
			continue;
		}
		if(only_get==-1){
			if(!print_tudo)c = CM::get()->setFromString(nome,NULL,valor,true);
		}else{
			c = CM::get()->setFromString(nome,valor,valor,true);
		}

		if(c<CM::CONFIGMANAGER_MAX && !print_tudo){
			if(CM::get()->type(c)==CM::INTEGER){
				len += sprintf(jsonbuff+len,"\"%s\":%s,\n",nome,valor);
			}else{
				len += sprintf(jsonbuff+len,"\"%s\":\"%s\",\n",nome,valor);
			}
			offset = 2;
		}
	}
	if(!print_tudo){
		len += sprintf(jsonbuff+len-offset,"\n}");
	}else{
		len = CM::get()->getFullJSON(jsonbuff,2000);
	}

	d->printf(reply_fmt,len,jsonbuff);
}

void stations_cgi(HTTPData *d){
	static const char auth[][16]={"OPEN","WEP","WPA_PSK","WPA2_PSK","WPA_WPA2_PSK","WPA2_ENTERPRISE"};
	int len=0, i=0;
	char jsonbuff[2000];
	wifi_ap_record_t *records;
	int record_count;
	WifiHandler *wh = WifiHandler::get();
	len += sprintf(jsonbuff+len,"[\n");

	wh->getWifiScan(&records, &record_count);
	for(i=0;i<record_count;i++){
		len += sprintf(jsonbuff+len,"{\n"
				"  \"ssid\":\"%s\",\n"
				"  \"bssid\":\"%02x%02x%02x%02x%02x%02x\",\n"
				"  \"authmode\":\"%s\",\n"
				"  \"rssi\":%d\n},",
				records[i].ssid, records[i].bssid[0],records[i].bssid[1],
				records[i].bssid[2],records[i].bssid[3],records[i].bssid[4],
				records[i].bssid[5],auth[records[i].authmode],records[i].rssi);
	}
	wh->freeWifiScan();

	if(record_count>0)len-=1;
	len += sprintf(jsonbuff+len,"\n]");
	d->printf(reply_fmt,len,jsonbuff);
}

void save_cgi(HTTPData *d){
	char jsonbuff[100];
	int len=0;
	len+=sprintf(jsonbuff+len,"{\n,\n");
	if(d->getGET("internal",NULL)){
		esp_err_t err = CM::get()->saveToNVM();
		len-=2;
		len+=sprintf(jsonbuff+len,"\"internal\":{\n");
		if(err==ESP_OK)len+=sprintf(jsonbuff+len,"  \"ok\":true,\n");
		else           len+=sprintf(jsonbuff+len,"  \"ok\":false,\n");
		len+=sprintf(jsonbuff+len,"  \"reason\":\"%s\"\n},\n",esp_err_to_name(err));
	}
	if(d->getGET("load_internal",NULL)){
		esp_err_t err = CM::get()->loadFromNVM();
		len-=2;
		len+=sprintf(jsonbuff+len,"\"load_internal\":{\n");
		if(err==ESP_OK)len+=sprintf(jsonbuff+len,"  \"ok\":true,\n");
		else           len+=sprintf(jsonbuff+len,"  \"ok\":false,\n");
		len+=sprintf(jsonbuff+len,"  \"reason\":\"%s\"\n},\n",esp_err_to_name(err));
	}
	len-=2;
	len+=sprintf(jsonbuff+len,"}");
	d->printf(reply_fmt,len,jsonbuff);
}

void reboot_cgi(HTTPData *d){
	char jsonbuff[100];
	int len=0;
	extern int64_t INTERNAL_PLEASE_RESTART;
	len+=sprintf(jsonbuff+len,"{\"ok\":true}");
	d->printf(reply_fmt,len,jsonbuff);
	INTERNAL_PLEASE_RESTART = esp_timer_get_time()+1000*1000;
}
