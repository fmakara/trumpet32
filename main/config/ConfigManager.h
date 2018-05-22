/*
 * ConfigManager.h
 *
 *  Created on: 17 de mai de 2018
 *      Author: makara
 */

#ifndef MAIN_CONFIGS_CONFIGMANAGER_H_
#define MAIN_CONFIGS_CONFIGMANAGER_H_

#include <stdint.h>
#include <stddef.h>
//#include "Character.h"

class ConfigManager {
public:
  static ConfigManager* get();

  enum Config {
		//Integer configs
    SPLAYER_NOTE = 0,
    SPLAYER_VOLUME,
    SPLAYER_BEEP1_FREQ,
    SPLAYER_BEEP1_TIME,
    SPLAYER_BEEP_VOLUME,
    SPLAYER_BEEP2_FREQ,
    SPLAYER_BEEP2_TIME,
    USER_LEDS,
    USER_BUTTONS,
    USER_PISTONS,
    METRONOME_COMPASS,
    METRONOME_BPM,
    MOUTHP_FREQUENCYx10,
    MOUTHP_AMPLITUDE,
    INSTRUMENT_NOTE_NOMINAL,
    INSTRUMENT_NOTE_REAL,
    INSTRUMENT_AMPLITUDE,
    INSTRUMENT_NOTE_ERROR,
    SDCARD_STATUS,
    LCD_BRIGHTNESS,
    LCD_CONSTRAST,
    LEDS_INTENSITY,
    WIFI_MODE,
    WIFI_AP_CHANNEL,
    //String configs
    WIFI_DEVICENAME,
    WIFI_AP_SSID,
    WIFI_AP_PASSWD,
    WIFI_STA_SSID,
    WIFI_STA_PASSWD,
    //MAX_CONFIG always at the end
    CONFIGMANAGER_MAX
	};

  int32_t get(Config c);
  void get(Config c, char *s);

  void set(Config c, int32_t v);
  void set(Config c, const char *s);

  void setCallback(Config c, void (*cb)(int32_t));

  Config stringToIndex(const char *s);
  Config setFromString(const char *config, const char *from, char *valset);

  int getFullJSON(char *s, int maxlen);
  int setFromJSON(char *s);

  static int char2unicode(char *s, char *j, int max);
  static int unicode2char(char *j, char *s, int max);
protected:
  uint8_t *buffer;
  ConfigManager();
  static ConfigManager *singleton;

  enum ConfigType {
    INTEGER, STRING, BAD
  };
  struct ConfigParameters {
    ConfigType type;
    int32_t min;
    int32_t max;
    char name[32];
    void (*cb)(int32_t);
    uint8_t *p;
  };
  ConfigParameters params[CONFIGMANAGER_MAX+1] = {
    {INTEGER,    0,     72, "SPLAYER_NOTE"                   , NULL, NULL},
    {INTEGER,    0,    255, "SPLAYER_VOLUME"                 , NULL, NULL},
    {INTEGER,    0,  10000, "SPLAYER_BEEP1_FREQ"             , NULL, NULL},
    {INTEGER,    0,   1000, "SPLAYER_BEEP1_TIME"             , NULL, NULL},
    {INTEGER,    0,    255, "SPLAYER_BEEP_VOLUME"            , NULL, NULL},
    {INTEGER,    0,  10000, "SPLAYER_BEEP2_FREQ"             , NULL, NULL},
    {INTEGER,    0,   1000, "SPLAYER_BEEP2_TIME"             , NULL, NULL},
    {INTEGER,    0,   0x0F, "USER_LEDS"                      , NULL, NULL},
    {INTEGER,    0,   0x3F, "USER_BUTTONS"                   , NULL, NULL},
    {INTEGER,    0,   0x0F, "USER_PISTONS"                   , NULL, NULL},
    {INTEGER,    0,      9, "METRONOME_COMPASS"              , NULL, NULL},
    {INTEGER,    0,    600, "METRONOME_BPM"                  , NULL, NULL},
    {INTEGER,    0, 100000, "MOUTHP_FREQUENCYx10"            , NULL, NULL},
    {INTEGER,    0,    255, "MOUTHP_AMPLITUDE"               , NULL, NULL},
    {INTEGER,    0,     72, "INSTRUMENT_NOTE_NOMINAL"        , NULL, NULL},
    {INTEGER,    0,     72, "INSTRUMENT_NOTE_REAL"           , NULL, NULL},
    {INTEGER,    0,    255, "INSTRUMENT_AMPLITUDE"           , NULL, NULL},
    {INTEGER, -100,    100, "INSTRUMENT_NOTE_ERROR"          , NULL, NULL},
    {INTEGER,    0,      3, "SDCARD_STATUS"                  , NULL, NULL},
    {INTEGER,    0,    100, "LCD_BRIGHTNESS"                 , NULL, NULL},
    {INTEGER,    0,    100, "LCD_CONSTRAST"                  , NULL, NULL},
    {INTEGER,    0,    100, "LEDS_INTENSITY"                 , NULL, NULL},
    {INTEGER,    0,      3, "WIFI_MODE"                      , NULL, NULL},
    {INTEGER,    1,     13, "WIFI_AP_CHANNEL"                , NULL, NULL},

    {STRING,     3,     32, "WIFI_DEVICENAME"                , NULL, NULL},
    {STRING,     3,     32, "WIFI_AP_SSID"                   , NULL, NULL},
    {STRING,     3,     32, "WIFI_AP_PASSWD"                 , NULL, NULL},
    {STRING,     3,     32, "WIFI_STA_SSID"                  , NULL, NULL},
    {STRING,     3,     32, "WIFI_STA_PASSWD"                , NULL, NULL},

    {BAD,        0,      0, ""                               , NULL, NULL},
  };
};

typedef ConfigManager CM;

#endif /* MAIN_CONFIGS_CONFIGMANAGER_H_ */
