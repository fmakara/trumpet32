#include <stdio.h>
#include "lcd/Sprite.h"
#include "lcd/Lcd.h"
#include "lcd/Charset.h"
#include "lcd/Writer.h"
#include "config/ConfigManager.h"
#include "config/WifiHandler.h"
#include "audio/AdcReader.h"
#include "config/Webpages.h"
#include "ui/PopUpper.h"
#include "ui/Dictionary.h"
#include "ui/MainMenu.h"
#include "audio/Player.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include "nvs_flash.h"

#include "esp_log.h"

#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "mongoose/WebService.h"


extern "C" {
void app_main();
}

const uint8_t chrset_p6[]={10, 1, 0, 0, 0, 11, 0, 120, 0, 132, 0, 34, 1, 77, 2, 129, 2, 145, 2, 129, 2, 77, 2, 34, 1, 132, 0, 120, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 0, 0, 1, 1, 190, 0, 3, 1, 7, 0, 0, 0, 7, 0, 7, 1, 32, 0, 232, 0, 56, 0, 238, 0, 56, 0, 46, 0, 8, 0, 5, 1, 136, 0, 148, 0, 254, 3, 164, 0, 68, 0, 9, 1, 12, 0, 18, 0, 146, 0, 108, 0, 16, 0, 108, 0, 146, 0, 144, 0, 96, 0, 6, 1, 108, 0, 146, 0, 146, 0, 172, 0, 64, 0, 176, 0, 1, 1, 7, 0, 3, 1, 252, 0, 2, 1, 1, 2, 3, 1, 1, 2, 2, 1, 252, 0, 5, 1, 10, 0, 4, 0, 31, 0, 4, 0, 10, 0, 5, 1, 16, 0, 16, 0, 124, 0, 16, 0, 16, 0, 2, 1, 0, 1, 192, 0, 3, 1, 16, 0, 16, 0, 16, 0, 1, 1, 192, 0, 4, 1, 0, 3, 224, 0, 28, 0, 3, 0, 4, 1, 124, 0, 130, 0, 130, 0, 124, 0, 3, 1, 132, 0, 254, 0, 128, 0, 4, 1, 194, 0, 162, 0, 146, 0, 140, 0, 4, 1, 130, 0, 146, 0, 146, 0, 108, 0, 5, 1, 48, 0, 40, 0, 36, 0, 254, 0, 32, 0, 4, 1, 158, 0, 146, 0, 146, 0, 98, 0, 4, 1, 124, 0, 146, 0, 146, 0, 96, 0, 4, 1, 130, 0, 98, 0, 26, 0, 6, 0, 4, 1, 108, 0, 146, 0, 146, 0, 108, 0, 4, 1, 12, 0, 146, 0, 146, 0, 124, 0, 1, 1, 216, 0, 2, 1, 0, 1, 216, 0, 5, 1, 32, 0, 80, 0, 80, 0, 80, 0, 136, 0, 6, 1, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 5, 1, 136, 0, 80, 0, 80, 0, 80, 0, 32, 0, 4, 1, 2, 0, 162, 0, 18, 0, 12, 0, 8, 1, 120, 0, 132, 0, 50, 1, 74, 1, 74, 1, 122, 1, 68, 0, 56, 0, 6, 1, 192, 0, 56, 0, 38, 0, 38, 0, 56, 0, 192, 0, 5, 1, 254, 0, 146, 0, 146, 0, 146, 0, 108, 0, 6, 1, 56, 0, 68, 0, 130, 0, 130, 0, 130, 0, 130, 0, 6, 1, 254, 0, 130, 0, 130, 0, 130, 0, 68, 0, 56, 0, 5, 1, 254, 0, 146, 0, 146, 0, 146, 0, 130, 0, 5, 1, 254, 0, 18, 0, 18, 0, 18, 0, 18, 0, 6, 1, 56, 0, 68, 0, 130, 0, 146, 0, 146, 0, 242, 0, 6, 1, 254, 0, 16, 0, 16, 0, 16, 0, 16, 0, 254, 0, 3, 1, 130, 0, 254, 0, 130, 0, 3, 1, 128, 0, 130, 0, 126, 0, 5, 1, 254, 0, 16, 0, 40, 0, 68, 0, 130, 0, 4, 1, 254, 0, 128, 0, 128, 0, 128, 0, 7, 1, 254, 0, 6, 0, 24, 0, 96, 0, 24, 0, 6, 0, 254, 0, 6, 1, 254, 0, 4, 0, 8, 0, 16, 0, 32, 0, 254, 0, 7, 1, 56, 0, 68, 0, 130, 0, 130, 0, 130, 0, 68, 0, 56, 0, 5, 1, 254, 0, 34, 0, 34, 0, 34, 0, 28, 0, 7, 1, 56, 0, 68, 0, 130, 0, 130, 0, 130, 1, 68, 2, 56, 2, 5, 1, 254, 0, 18, 0, 50, 0, 76, 0, 128, 0, 5, 1, 140, 0, 146, 0, 146, 0, 146, 0, 98, 0, 5, 1, 2, 0, 2, 0, 254, 0, 2, 0, 2, 0, 6, 1, 126, 0, 128, 0, 128, 0, 128, 0, 128, 0, 126, 0, 5, 1, 14, 0, 112, 0, 128, 0, 112, 0, 14, 0, 9, 1, 6, 0, 120, 0, 128, 0, 120, 0, 6, 0, 120, 0, 128, 0, 120, 0, 6, 0, 5, 1, 130, 0, 108, 0, 16, 0, 108, 0, 130, 0, 5, 1, 2, 0, 12, 0, 240, 0, 12, 0, 2, 0, 5, 1, 194, 0, 162, 0, 146, 0, 138, 0, 134, 0, 3, 1, 255, 3, 1, 2, 1, 2, 4, 0, 3, 0, 28, 0, 224, 0, 0, 3, 3, 1, 1, 2, 1, 2, 255, 3, 6, 1, 8, 0, 4, 0, 2, 0, 2, 0, 4, 0, 8, 0, 5, 0, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 2, 1, 1, 0, 2, 0, 4, 1, 64, 0, 168, 0, 168, 0, 240, 0, 5, 1, 255, 0, 136, 0, 136, 0, 136, 0, 112, 0, 4, 1, 112, 0, 136, 0, 136, 0, 136, 0, 5, 1, 112, 0, 136, 0, 136, 0, 136, 0, 255, 0, 4, 1, 112, 0, 168, 0, 168, 0, 176, 0, 3, 1, 254, 0, 9, 0, 9, 0, 5, 1, 112, 0, 136, 2, 136, 2, 136, 2, 248, 1, 5, 1, 255, 0, 8, 0, 8, 0, 8, 0, 240, 0, 1, 1, 250, 0, 2, 1, 8, 2, 250, 1, 5, 1, 255, 0, 32, 0, 48, 0, 72, 0, 128, 0, 1, 1, 255, 0, 7, 1, 248, 0, 8, 0, 8, 0, 240, 0, 8, 0, 8, 0, 240, 0, 5, 1, 248, 0, 8, 0, 8, 0, 8, 0, 240, 0, 5, 1, 112, 0, 136, 0, 136, 0, 136, 0, 112, 0, 5, 1, 248, 3, 136, 0, 136, 0, 136, 0, 112, 0, 5, 1, 112, 0, 136, 0, 136, 0, 136, 0, 248, 3, 3, 1, 248, 0, 16, 0, 8, 0, 3, 1, 152, 0, 168, 0, 200, 0, 2, 1, 126, 0, 136, 0, 5, 1, 120, 0, 128, 0, 128, 0, 128, 0, 248, 0, 5, 1, 8, 0, 112, 0, 128, 0, 112, 0, 8, 0, 7, 1, 56, 0, 192, 0, 96, 0, 24, 0, 96, 0, 192, 0, 56, 0, 5, 1, 136, 0, 80, 0, 32, 0, 80, 0, 136, 0, 5, 0, 8, 0, 112, 2, 128, 1, 112, 0, 8, 0, 3, 1, 200, 0, 168, 0, 152, 0, 4, 1, 32, 0, 32, 0, 222, 1, 1, 2, 1, 1, 255, 3, 4, 1, 1, 2, 222, 1, 32, 0, 32, 0, 6, 1, 48, 0, 8, 0, 16, 0, 16, 0, 32, 0, 24, 0, 1, 0, 0, 0};


void callback_updateAP(int32_t ip){
  WifiHandler::get()->updateAP();
}

void callback_updateSTA(int32_t ip){
  WifiHandler::get()->updateSTA();
}

int64_t GLOBAL_PLEASE_RESTART = INT64_MAX;

void app_main(){
  nvs_flash_init();
  CM::get()->loadFromNVM();
  //Load the basic font
  Charset::get()->loadFromBytes(chrset_p6,sizeof(chrset_p6),6);
  //Initialize basic peripherals
  Lcd *lcd = Lcd::get();
  PopUpper::get();
  Dictionary::global = new Dictionary();

  WifiHandler::get();
  WifiHandler::get()->updateAP();
  WifiHandler::get()->updateSTA();
  //AdcReader::get();

  CM::get()->setCallback(CM::WIFI_AP_SSID,callback_updateAP);
  CM::get()->setCallback(CM::WIFI_STA_SSID,callback_updateSTA);

  esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = "storage",
      .max_files = 5,
      .format_if_mount_failed = true
  };
  esp_err_t ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      printf("Failed to mount or format filesystem\n\n");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      printf("Failed to find SPIFFS partition\n\n");
    } else {
      printf("Failed to initialize SPIFFS (%s)\n\n", esp_err_to_name(ret));
    }
    return;
  }

  size_t total = 0, used = 0;
  ret = esp_spiffs_info("storage", &total, &used);
  if (ret != ESP_OK) {
    printf( "Failed to get SPIFFS partition information (%s)\n\n", esp_err_to_name(ret));
  } else {
    printf( "Partition size: total: %d, used: %d\n\n", total, used);
  }

  WebService *ws = new WebService("80");
  init_cgi(ws);

  ws->addSpiffs("/*", "/spiffs/*");
  printf("Running WebService");

  Player *p = Player::get();
  p->startup_sine_waves();
  printf("Running Player\n");

  int r = 0;
  ScreenElement menu(new Sprite(84,48),2);
  menu.addToList(lcd->root);

  while(1){
    MainMenu::get()->run(&menu);

    vTaskDelay(1000);
    if(GLOBAL_PLEASE_RESTART < esp_timer_get_time()){
      CM::get()->saveToNVM();
      printf("Restarting now.\n");
      esp_restart();
    }
  }
}

