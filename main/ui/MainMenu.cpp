/*
 * MainMenu.cpp
 *
 *  Created on: 30 de jul de 2018
 *      Author: makara
 */

#include "MainMenu.h"
#include <stdio.h>
#include "../lcd/Charset.h"
#include "../lcd/Writer.h"
#include "../lcd/Lcd.h"
#include "PopUpper.h"

MainMenu* MainMenu::singleton = NULL;

MainMenu* MainMenu::get(){
  if(singleton==NULL)singleton = new MainMenu();
  return singleton;
}

MainMenu::MainMenu() {
  root = new MenuElement(ME_MENU,DIC::MENU_MAINMENU_CAP,CM::INVALID,NULL);
  //Metronome submenu
  MenuElement *metronome = new MenuElement(ME_MENU,DIC::MENU_METRONOME_CAP,CM::INVALID,root);
  metronome->childs = new std::vector<MenuElement*>();
  metronome->childs->push_back(new MenuElement(ME_INTEGER,DIC::METRO_BPM_CAP,CM::METRO_BPM,metronome));
  metronome->childs->push_back(new MenuElement(ME_INTEGER,DIC::METRO_COMPASS_CAP,CM::METRO_COMPASS,metronome));
  metronome->childs->push_back(new MenuElement(ME_INTEGER,DIC::PLAYR_B_VOLUME_CAP,CM::PLAYR_B_VOLUME,metronome));
  metronome->childs->push_back(new MenuElement(ME_INTEGER,DIC::PLAYR_B1_FREQ_CAP,CM::PLAYR_B1_FREQ,metronome));
  metronome->childs->push_back(new MenuElement(ME_INTEGER,DIC::PLAYR_B1_TIME_CAP,CM::PLAYR_B1_TIME,metronome));
  metronome->childs->push_back(new MenuElement(ME_INTEGER,DIC::PLAYR_B2_FREQ_CAP,CM::PLAYR_B2_FREQ,metronome));
  metronome->childs->push_back(new MenuElement(ME_INTEGER,DIC::PLAYR_B2_TIME_CAP,CM::PLAYR_B2_TIME,metronome));

  // General status submenu
  MenuElement *status = new MenuElement(ME_MENU,DIC::MENU_STATUS_CAP,CM::INVALID,root);
  status->childs = new std::vector<MenuElement*>();
  status->childs->push_back(new MenuElement(ME_INTEGER_C,DIC::PLAYR_NOTE_CAP,CM::PLAYR_NOTE,status));
  status->childs->push_back(new MenuElement(ME_INTEGER_C,DIC::PLAYR_VOLUME_CAP,CM::PLAYR_VOLUME,status));
  status->childs->push_back(new MenuElement(ME_INTEGER_C,DIC::USER_LEDS_CAP,CM::USER_LEDS,status));
  status->childs->push_back(new MenuElement(ME_INTEGER_C,DIC::USER_BUTTONS_CAP,CM::USER_BUTTONS,status));
  status->childs->push_back(new MenuElement(ME_INTEGER_C,DIC::USER_PISTONS_CAP,CM::USER_PISTONS,status));

  std::map<int,DIC::DICTindex> *wifi_modes = new std::map<int,DIC::DICTindex>();
  (*wifi_modes)[0] = DIC::WIFIMODE_NONE;
  (*wifi_modes)[1] = DIC::WIFIMODE_AP;
  (*wifi_modes)[2] = DIC::WIFIMODE_STA;
  (*wifi_modes)[3] = DIC::WIFIMODE_BOTH;
  //Wifi configs
  MenuElement *wifi = new MenuElement(ME_MENU,DIC::MENU_WIFI_CAP,CM::INVALID,root);
  wifi->childs = new std::vector<MenuElement*>();
  wifi->childs->push_back(new MenuElement(ME_ENUM,DIC::WIFI_MODE_CAP,CM::WIFI_MODE,wifi,wifi_modes));
  wifi->childs->push_back(new MenuElement(ME_INTEGER,DIC::WIFI_SCAN_TIMES_CAP,CM::WIFI_SCAN_TIMES,wifi));
  wifi->childs->push_back(new MenuElement(ME_INTEGER,DIC::WIFI_AP_CHANNEL_CAP,CM::WIFI_AP_CHANNEL,wifi));
  wifi->childs->push_back(new MenuElement(ME_STRING,DIC::WIFI_DEVICENAME_CAP,CM::WIFI_DEVICENAME,wifi));
  wifi->childs->push_back(new MenuElement(ME_STRING,DIC::WIFI_AP_SSID_CAP,CM::WIFI_AP_SSID,wifi));
  wifi->childs->push_back(new MenuElement(ME_STRING,DIC::WIFI_AP_PASSWD_CAP,CM::WIFI_AP_PASSWD,wifi));
  wifi->childs->push_back(new MenuElement(ME_STRING,DIC::WIFI_STA_SSID_CAP,CM::WIFI_STA_SSID,wifi));
  wifi->childs->push_back(new MenuElement(ME_STRING,DIC::WIFI_STA_PASSWD_CAP,CM::WIFI_STA_PASSWD,wifi));
  wifi->childs->push_back(new MenuElement(ME_IP_C,DIC::WIFI_AP_IP_CAP,CM::WIFI_AP_IP,wifi));
  wifi->childs->push_back(new MenuElement(ME_IP_C,DIC::WIFI_STA_IP_CAP,CM::WIFI_STA_IP,wifi));
  wifi->childs->push_back(new MenuElement(ME_IP_C,DIC::WIFI_STA_GW_CAP,CM::WIFI_STA_GW,wifi));
  wifi->childs->push_back(new MenuElement(ME_IP_C,DIC::WIFI_STA_NM_CAP,CM::WIFI_STA_NM,wifi));

  //Main menu
  root->childs = new std::vector<MenuElement*>();
  root->childs->push_back(metronome);
  root->childs->push_back(wifi);
  root->childs->push_back(status);


}

void MainMenu::run(ScreenElement *scr){
  MenuElement *shown = root, *lastShown = root;
  Character *NS_font = Charset::get()->asArray(MainMenuFontNormal);
  Writer NS_writer(NS_font,NULL,NULL);
  uint32_t NS_fontheight = NS_font[0].height();
  Sprite *s = scr->sprite();
  uint8_t lastInput = 0;
  Lcd *lcd = Lcd::get();
  CM *cm = CM::get();
  //only for menus
  int scrollcount=0;
  bool justEnteredMenu = true;
  int menu_nlines = ((s->height()-1)/NS_font[0].height())-1;
  Sprite *menuSpriteTop = new Sprite(s,0,0,s->width(),NS_fontheight);
  std::vector<Sprite*> menuSpriteList;
  for(int i=1;i<=menu_nlines;i++)
    menuSpriteList.push_back(new Sprite(s,0,1+i*NS_fontheight,s->width()-2,NS_fontheight));

  scr->visible(true);
  while(true){
    //render the screen
    if(shown->type==ME_MENU){
      //Find the number of lines available to show
      //housekeeping for the selected index and screen offset
      if(shown->menu_index<0)shown->menu_index=0;
      if(shown->menu_index>=shown->childs->size())shown->menu_index=shown->childs->size()-1;

      if(shown->menu_offset>=(shown->childs->size()-menu_nlines))
        shown->menu_offset=(shown->childs->size()-menu_nlines);
      if(shown->menu_offset<0)shown->menu_offset=0;

      //The offset shall always follow the index within the available window
      if(shown->menu_index>=shown->menu_offset+menu_nlines)
        shown->menu_offset = shown->menu_index-menu_nlines+1;
      if(shown->menu_index<shown->menu_offset)
        shown->menu_offset = shown->menu_index;

      //Begin the drawing
      s->clear();
      NS_writer.scroll_y=0;
      NS_writer.canvas = menuSpriteTop;
      NS_writer.renderCentered((char*)DIC::global->get(shown->name));

      for(int i=0;i<menu_nlines;i++){
        NS_writer.scroll_x=0;
        NS_writer.canvas = menuSpriteList[i];
        NS_writer.text = (char*)DIC::global->get(shown->childs->at(i+shown->menu_offset)->name);
        if(i+shown->menu_offset == shown->menu_index){
          int txtwidth;
          NS_writer.max_scroll(&txtwidth,NULL);
          if(scrollcount<8){
            //offset already at 0
          }else if((scrollcount-8)*5<+txtwidth){
            NS_writer.scroll_x = (scrollcount-8)*5;
          }else if((scrollcount-16)*5<+txtwidth){
            NS_writer.scroll_x = txtwidth;
          }else{
            scrollcount = 0;
          }
          NS_writer.render();
          menuSpriteList[i]->invert(0,0,999,999);
        }else{
          NS_writer.render();
        }
      }

      // Horizontal tab and scroll bar
      s->line(0,NS_fontheight,s->width(),NS_fontheight);
      if(shown->childs->size()>menu_nlines){
        int perline = (s->height()-NS_fontheight-3)/shown->childs->size();
        s->line(s->width()-1,NS_fontheight+3+perline*shown->menu_offset,
            s->width()-1,NS_fontheight+3+perline*(menu_nlines+shown->menu_offset));
      }

    }else if(shown->type==ME_INTEGER_C ||
        shown->type==ME_STRING_C ||
        shown->type==ME_IP_C ||
        shown->type==ME_ENUM_C
    ){
      char txtbuffer[512], *txtvalue;
      int txtwidth=0;
      s->clear();

      // Propriety name
      NS_writer.scroll_x=0;
      NS_writer.scroll_y=0;
      NS_writer.canvas = s;
      NS_writer.renderCentered((char*)DIC::global->get(shown->name));

      // Horizontal tab
      s->line(0,NS_fontheight,s->width(),NS_fontheight);

      //recover the string value (CM has already for IP,INT and STRING)
      if(shown->type==ME_ENUM_C){
        int32_t value = cm->get(shown->cfg);
        if(shown->enumerate->count(value)>0){
          txtvalue = (char*)DIC::global->get(shown->enumerate->at(value));
        }else{
          sprintf(txtbuffer,"ENUM?(%d)",value);
          txtvalue = txtbuffer;
        }
      }else{
        cm->get(shown->cfg,txtbuffer);
        txtvalue = txtbuffer;
      }
      NS_writer.max_scroll(&txtwidth,NULL);
      NS_writer.scroll_x = 0;
      NS_writer.scroll_y = -((NS_fontheight+s->height())/2);

      //Write centered or (if too big) scrolling
      if(txtwidth>0){
        if(scrollcount<8){
          //offset already at 0
        }else if((scrollcount-8)*5<+txtwidth){
          NS_writer.scroll_x = (scrollcount-8)*5;
        }else if((scrollcount-16)*5<+txtwidth){
          NS_writer.scroll_x = txtwidth;
        }else{
          scrollcount = 0;
        }
        NS_writer.render();
      }else{
        NS_writer.renderCentered(txtvalue);
      }

    }else if(shown->type==ME_ENUM){
      if(justEnteredMenu){
        //Find the corresponding index of the enum
        int32_t value = cm->get(shown->cfg);
        int idx = 0;
        if(shown->enumerate->count(value)>0){
          std::map<int,DIC::DICTindex>::iterator it = shown->enumerate->begin();
          for(; it->first!=value ; ++it, ++idx);
        }
        shown->menu_index=idx;
      }
      //Find the number of lines available to show
      //housekeeping for the selected index and screen offset
      if(shown->menu_index<0)shown->menu_index=0;
      if(shown->menu_index>=shown->enumerate->size())shown->menu_index=shown->enumerate->size()-1;

      if(shown->menu_offset>=(shown->enumerate->size()-menu_nlines))
        shown->menu_offset=(shown->enumerate->size()-menu_nlines);
      if(shown->menu_offset<0)shown->menu_offset=0;

      //The offset shall always follow the index within the available window
      if(shown->menu_index>=shown->menu_offset+menu_nlines)
        shown->menu_offset = shown->menu_index-menu_nlines+1;
      if(shown->menu_index<shown->menu_offset)
        shown->menu_offset = shown->menu_index;

      //Begin the drawing
      s->clear();
      NS_writer.scroll_x=0;
      NS_writer.scroll_y=0;
      NS_writer.canvas = menuSpriteTop;
      NS_writer.renderCentered((char*)DIC::global->get(shown->name));

      std::map<int,DIC::DICTindex>::iterator it = shown->enumerate->begin();
      for(int i=0;i<shown->menu_offset;i++)++it;
      for(int i=0;i<menu_nlines;i++,++it){
        NS_writer.scroll_x=0;
        NS_writer.canvas = menuSpriteList[i];
        NS_writer.text = (char*)DIC::global->get(it->second);
        if(i+shown->menu_offset == shown->menu_index){
          int txtwidth;
          NS_writer.max_scroll(&txtwidth,NULL);
          if(scrollcount<8){
            //offset already at 0
          }else if((scrollcount-8)*5<+txtwidth){
            NS_writer.scroll_x = (scrollcount-8)*5;
          }else if((scrollcount-16)*5<+txtwidth){
            NS_writer.scroll_x = txtwidth;
          }else{
            scrollcount = 0;
          }
          NS_writer.render();
          menuSpriteList[i]->invert(0,0,999,999);
        }else{
          NS_writer.render();
        }
      }

      // Horizontal tab and scroll bar
      s->line(0,NS_fontheight+1,s->width(),NS_fontheight+1);
      if(shown->enumerate->size()>menu_nlines){
        int perline = (s->height()-NS_fontheight-3)/shown->enumerate->size();
        s->line(s->width()-1,NS_fontheight+3+perline*shown->menu_offset,
            s->width()-1,NS_fontheight+3+perline*(menu_nlines+shown->menu_offset));
      }
    }else{
      PopUpper::get()->popup("Not implemented / error",3000,0);
      break;
    }
    //check for inputs
    if(shown->type==ME_MENU){
      //Not only if there is a press, but if it was already pressed and not computed
      if(lastInput){
        lcd->buttons_waitRelease(lastInput,1000);
        lastInput = 0;
      }else{
        lastInput = lcd->buttons_get(Lcd::BUTTONS_USED);
      }
      if(!lastInput)lastInput = lcd->buttons_waitPress(Lcd::BUTTONS_USED,100);

      if(lastInput&(Lcd::BUTTON_BACK|Lcd::BUTTON_LEFT)){
        //Go back, when possible
        if(shown->father==NULL)break;
        shown = shown->father;
      }else if(lastInput&(Lcd::BUTTON_OK|Lcd::BUTTON_RIGHT)){
        //Enter into child
        shown = shown->childs->at(shown->menu_index);
      }else if(lastInput & Lcd::BUTTON_UP){
        shown->menu_index--;
      }else if(lastInput & Lcd::BUTTON_DOWN){
        shown->menu_index++;
      }
    }else if(shown->type==ME_INTEGER_C ||
        shown->type==ME_STRING_C ||
        shown->type==ME_IP_C ||
        shown->type==ME_ENUM_C ){
      if(lastInput){
        lcd->buttons_waitRelease(lastInput,1000);
        lastInput = 0;
      }else{
        lastInput = lcd->buttons_get(Lcd::BUTTONS_USED);
      }
      if(!lastInput)lastInput = lcd->buttons_waitPress(Lcd::BUTTONS_USED,300);

      if(lastInput){
        //Go back with any button
        if(shown->father==NULL)break;
        shown = shown->father;
      }
    }else if(shown->type==ME_ENUM){
      //Not only if there is a press, but if it was already pressed and not computed
      if(lastInput){
        lcd->buttons_waitRelease(lastInput,1000);
        lastInput = 0;
      }else{
        lastInput = lcd->buttons_get(Lcd::BUTTONS_USED);
      }
      if(!lastInput)lastInput = lcd->buttons_waitPress(Lcd::BUTTONS_USED,300);

      if(lastInput&(Lcd::BUTTON_BACK|Lcd::BUTTON_LEFT)){
        //Go back, when possible
        if(shown->father==NULL)break;
        shown = shown->father;
      }else if(lastInput&(Lcd::BUTTON_OK|Lcd::BUTTON_RIGHT)){
        //Save config and go back
        std::map<int,DIC::DICTindex>::iterator it = shown->enumerate->begin();
        for(int idx=0 ; idx<shown->menu_index ; ++it, ++idx);
        cm->set(shown->cfg,it->first,false);
        if(shown->father==NULL)break;
        shown = shown->father;
      }else if(lastInput & Lcd::BUTTON_UP){
        shown->menu_index--;
      }else if(lastInput & Lcd::BUTTON_DOWN){
        shown->menu_index++;
      }
    }
    if(lastInput){
      scrollcount=0;
    }else{
      scrollcount++;
    }
    justEnteredMenu = !(shown==lastShown);
    lastShown = shown;
  }
  //cleanup
  scr->visible(false);
  delete menuSpriteTop;
  while(!menuSpriteList.empty()) {
    delete menuSpriteList.back();
    menuSpriteList.pop_back();
  }
}
