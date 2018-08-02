/*
 * Dictionary.cpp
 *
 *  Created on: 8 de jul de 2018
 *      Author: makara
 */

#include "Dictionary.h"

#include <stdlib.h>
#include <string.h>
#include "../config/ConfigManager.h"

Dictionary *Dictionary::global = NULL;

Dictionary::Dictionary(char *str) {
  buffer = NULL;
  loadFrom(str);
}

Dictionary::~Dictionary(){
  if(buffer!=NULL)free(buffer);
}
int Dictionary::loadFrom(char *str){
  int i, j, stringsFound, sizeNeeded;
  //Mark the name and default value
  for(i=0, j=0; i<DICT_MAX ; i++){
    list[i][0] = list[i][1] = (char*)names+j;
    do{
      j++;
      if(j>=names_size)return -1;
    }while(names[j]!='\0');
    j++;
  }
  if(str==NULL)return 0;
  //Find the strings and calculate the internal buffer needed
  sizeNeeded = 0;
  for(i=0 ; i<DICT_MAX ; i++){
    int len = strlen(list[i][0]);
    bool found = false;
    char *pos, *beginStr, *endStr;
    while(!found){
      pos = strstr(str,list[i][0]);
      if(pos==NULL)break;
      if(*(pos-1)=='"' && *(pos+len)=='"'){
        found = true;
      }
    }
    if(pos==NULL)continue;
    for(beginStr = pos+len+1 ; *beginStr!='\0' && *beginStr!='"' ; beginStr++);
    if(*beginStr=='\0')continue;
    for(endStr = beginStr+1 ; *endStr!='\0' && *endStr!='"' ; endStr++){
      if(*endStr=='\\')endStr++;
    }
    if(*endStr=='\0')continue;
    list[i][1] = beginStr;
    sizeNeeded += endStr - beginStr;
  }
  if(sizeNeeded==0)return 0;
  buffer = (char*)malloc(sizeNeeded);
  if(buffer==NULL)return -1;
  stringsFound = 0;
  // Convert from JSON notation and copy into buffer
  for(i=0, j=0; i<DICT_MAX ; i++){
    if(list[i][1]>=names && list[i][1]<(names+names_size)){
      continue;
    }
    int cstrlen = ConfigManager::unicode2char(list[i][1], buffer+j,1000);
    list[i][1] = buffer+j;
    j+=cstrlen+1;
    stringsFound++;
  }
  return stringsFound;
}
const char* Dictionary::get(DICTindex i){
  if(i>=0 && i<DICT_MAX){
    return list[i][1];
  }
  return list[1][1];
}

const char Dictionary::names[] = "\
YES_CAP\0\
NO_CAP\0\
PLAYR_NOTE_CAP\0\
PLAYR_VOLUME_CAP\0\
PLAYR_B1_FREQ_CAP\0\
PLAYR_B1_TIME_CAP\0\
PLAYR_B_VOLUME_CAP\0\
PLAYR_B2_FREQ_CAP\0\
PLAYR_B2_TIME_CAP\0\
USER_LEDS_CAP\0\
USER_BUTTONS_CAP\0\
USER_PISTONS_CAP\0\
METRO_COMPASS_CAP\0\
METRO_BPM_CAP\0\
MOUTHP_FREQX10_CAP\0\
MOUTHP_AMPL_CAP\0\
INSTR_NOTE_NOM_CAP\0\
INSTR_NOTE_REAL_CAP\0\
INSTR_AMPLITUDE_CAP\0\
INSTR_NOTE_ERR_CAP\0\
SDCARD_STATUS_CAP\0\
LCD_BRIGHTNESS_CAP\0\
LCD_CONSTRAST_CAP\0\
LEDS_INTENSITY_CAP\0\
WIFI_MODE_CAP\0\
WIFI_SCAN_TIMES_CAP\0\
WIFI_AP_CHANNEL_CAP\0\
WIFI_DEVICENAME_CAP\0\
WIFI_AP_SSID_CAP\0\
WIFI_AP_PASSWD_CAP\0\
WIFI_STA_SSID_CAP\0\
WIFI_STA_PASSWD_CAP\0\
WIFI_AP_IP_CAP\0\
WIFI_STA_IP_CAP\0\
WIFI_STA_GW_CAP\0\
WIFI_STA_NM_CAP\0\
MENU_MAINMENU_CAP\0\
\0";
const int Dictionary::names_size = sizeof(names);

