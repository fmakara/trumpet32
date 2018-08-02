/*
 * Dictionary.h
 *
 *  Created on: 8 de jul de 2018
 *      Author: makara
 */

#ifndef MAIN_UI_DICTIONARY_H_
#define MAIN_UI_DICTIONARY_H_

#include <stddef.h>

class Dictionary {
public:
  enum DICTindex{
    YES_CAP = 0,
    NO_CAP,
    PLAYR_NOTE_CAP,
    PLAYR_VOLUME_CAP,
    PLAYR_B1_FREQ_CAP,
    PLAYR_B1_TIME_CAP,
    PLAYR_B_VOLUME_CAP,
    PLAYR_B2_FREQ_CAP,
    PLAYR_B2_TIME_CAP,
    USER_LEDS_CAP,
    USER_BUTTONS_CAP,
    USER_PISTONS_CAP,
    METRO_COMPASS_CAP,
    METRO_BPM_CAP,
    MOUTHP_FREQX10_CAP,
    MOUTHP_AMPL_CAP,
    INSTR_NOTE_NOM_CAP,
    INSTR_NOTE_REAL_CAP,
    INSTR_AMPLITUDE_CAP,
    INSTR_NOTE_ERR_CAP,
    SDCARD_STATUS_CAP,
    LCD_BRIGHTNESS_CAP,
    LCD_CONSTRAST_CAP,
    LEDS_INTENSITY_CAP,
    WIFI_MODE_CAP,
    WIFI_SCAN_TIMES_CAP,
    WIFI_AP_CHANNEL_CAP,
    WIFI_DEVICENAME_CAP,
    WIFI_AP_SSID_CAP,
    WIFI_AP_PASSWD_CAP,
    WIFI_STA_SSID_CAP,
    WIFI_STA_PASSWD_CAP,
    WIFI_AP_IP_CAP,
    WIFI_STA_IP_CAP,
    WIFI_STA_GW_CAP,
    WIFI_STA_NM_CAP,
    MENU_MAINMENU_CAP,
    DICT_MAX
  };

  Dictionary(char *str=NULL);
  ~Dictionary();
  int loadFrom(char *str=NULL);
  const char* get(DICTindex i);
  static Dictionary *global;
protected:
  char *list[DICT_MAX][2];
  char *buffer;
  static const char names[];
  static const int names_size;
};

typedef Dictionary DIC;

#endif /* MAIN_UI_DICTIONARY_H_ */
