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
#include "esp_err.h"

class ConfigManager {
public:
  static ConfigManager* get();

  enum Config {
    //Integer configs
    PLAYR_FREQ = 0,
    PLAYR_VOLUME,
    PLAYR_B1_FREQ,
    PLAYR_B1_TIME,
    PLAYR_B_VOLUME,
    PLAYR_B2_FREQ,
    PLAYR_B2_TIME,
    USER_LEDS,
    USER_BUTTONS,
    USER_PISTONS,
    METRO_COMPASS,
    METRO_BPM,
    MOUTHP_FREQX10,
    MOUTHP_AMPL,
    INSTR_NOTE_NOM,
    INSTR_NOTE_REAL,
    INSTR_AMPLITUDE,
    INSTR_NOTE_ERR,
    SDCARD_STATUS,
    LCD_BRIGHTNESS,
    LCD_CONSTRAST,
    LEDS_INTENSITY,
    WIFI_MODE,
    WIFI_SCAN_TIMES,
    WIFI_AP_CHANNEL,
    //String configs
    WIFI_DEVICENAME,
    WIFI_AP_SSID,
    WIFI_AP_PASSWD,
    WIFI_STA_SSID,
    WIFI_STA_PASSWD,
    //IP configs
    WIFI_AP_IP,
    WIFI_STA_IP,
    WIFI_STA_GW,
    WIFI_STA_NM,
    //MAX_CONFIG always at the end
    CONFIGMANAGER_MAX
  };
  static const Config INVALID = CONFIGMANAGER_MAX;
  enum ConfigType {
    INTEGER, STRING, IP, BAD
  };
  int32_t get(Config c);
  void get(Config c, char *s);

  void set(Config c, int32_t v, bool fromweb=false);
  void set(Config c, const char *s, bool fromweb=false);

  int32_t min(Config c);
  int32_t max(Config c);
  ConfigType type(Config c);


  void setCallback(Config c, void (*cb)(int32_t));

  Config stringToIndex(const char *s);
  Config setFromString(const char *config, const char *from, char *valset, bool fromweb=false);

  int getFullJSON(char *s, int maxlen);
  int setFromJSON(char *s);

  static int char2unicode(char *s, char *j, int max);
  static int unicode2char(char *j, char *s, int max);

  esp_err_t saveToNVM();
  esp_err_t loadFromNVM();

protected:
  uint8_t *buffer;
  ConfigManager();
  static ConfigManager *singleton;

  struct ConfigParameters {
    ConfigType type;
    int32_t min;
    int32_t max;
    char name[16];
    void (*cb)(int32_t);
    uint8_t *p;
    bool webWriteable;
    bool nvmWriteable;
  };
  ConfigParameters params[CONFIGMANAGER_MAX+1] = {
      {INTEGER,    0,     72, "PLAYR_NOTE"     , NULL, NULL, false, false},
      {INTEGER,    0,    255, "PLAYR_VOLUME"   , NULL, NULL, false, false},
      {INTEGER,    0,  10000, "PLAYR_B1_FREQ"  , NULL, NULL, false, false},
      {INTEGER,    0,   1000, "PLAYR_B1_TIME"  , NULL, NULL, false, false},
      {INTEGER,    0,    255, "PLAYR_B_VOLUME" , NULL, NULL, false, false},
      {INTEGER,    0,  10000, "PLAYR_B2_FREQ"  , NULL, NULL, false, false},
      {INTEGER,    0,   1000, "PLAYR_B2_TIME"  , NULL, NULL, false, false},
      {INTEGER,    0,   0x0F, "USER_LEDS"      , NULL, NULL, false, false},
      {INTEGER,    0,   0x3F, "USER_BUTTONS"   , NULL, NULL, false, false},
      {INTEGER,    0,   0x0F, "USER_PISTONS"   , NULL, NULL, false, false},
      {INTEGER,    0,      9, "METRO_COMPASS"  , NULL, NULL, true,  true },
      {INTEGER,    0,    600, "METRO_BPM"      , NULL, NULL, true,  true },
      {INTEGER,    0, 100000, "MOUTHP_FREQX10" , NULL, NULL, false, false},
      {INTEGER,    0,    255, "MOUTHP_AMPL"    , NULL, NULL, false, false},
      {INTEGER,    0,     72, "INSTR_NOTE_NOM" , NULL, NULL, false, false},
      {INTEGER,    0,     72, "INSTR_NOTE_REAL", NULL, NULL, false, false},
      {INTEGER,    0,    255, "INSTR_AMPLITUDE", NULL, NULL, false, false},
      {INTEGER, -100,    100, "INSTR_NOTE_ERR" , NULL, NULL, false, false},
      {INTEGER,    0,      3, "SDCARD_STATUS"  , NULL, NULL, false, false},
      {INTEGER,    0,    100, "LCD_BRIGHTNESS" , NULL, NULL, true,  true },
      {INTEGER,    0,    100, "LCD_CONSTRAST"  , NULL, NULL, true,  true },
      {INTEGER,    0,    100, "LEDS_INTENSITY" , NULL, NULL, true,  true },
      {INTEGER,    0,      3, "WIFI_MODE"      , NULL, NULL, true,  true },
      {INTEGER,    0,     20, "WIFI_SCAN_TIMES", NULL, NULL, true,  false},
      {INTEGER,    1,     13, "WIFI_AP_CHANNEL", NULL, NULL, true,  true },

      {STRING,     3,     32, "WIFI_DEVICENAME", NULL, NULL, true,  true },
      {STRING,     0,     32, "WIFI_AP_SSID"   , NULL, NULL, true,  true },
      {STRING,     0,     32, "WIFI_AP_PASSWD" , NULL, NULL, true,  true },
      {STRING,     0,     32, "WIFI_STA_SSID"  , NULL, NULL, true,  true },
      {STRING,     0,     32, "WIFI_STA_PASSWD", NULL, NULL, true,  true },

      {IP,         0,      0, "WIFI_AP_IP"     , NULL, NULL, false, false},
      {IP,         0,      0, "WIFI_STA_IP"    , NULL, NULL, false, false},
      {IP,         0,      0, "WIFI_STA_GW"    , NULL, NULL, false, false},
      {IP,         0,      0, "WIFI_STA_NM"    , NULL, NULL, false, false},

      {BAD,        0,      0, ""               , NULL, NULL, false, false},
  };
};

typedef ConfigManager CM;

#endif /* MAIN_CONFIGS_CONFIGMANAGER_H_ */
