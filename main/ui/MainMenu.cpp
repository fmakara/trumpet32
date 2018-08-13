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
  wifi->childs->push_back(new MenuElement(ME_IP,DIC::WIFI_STA_NM_CAP,CM::WIFI_STA_NM,wifi));

  //Main menu
  root->childs = new std::vector<MenuElement*>();
  root->childs->push_back(metronome);
  root->childs->push_back(wifi);
  root->childs->push_back(status);


}

void MainMenu::run(ScreenElement *scr){
  MenuElement *shown = root, *lastShown = root;
  Character *NS_font = Charset::get()->asArray(MainMenuFontNormal);
  Character *BS_font = Charset::get()->asArray(MainMenuFontBig);
  Writer NS_writer(NS_font,NULL,NULL);
  uint32_t NS_fontheight = NS_font[0].height();
  Sprite *s = scr->sprite();
  uint8_t lastInput = 0;
  Lcd *lcd = Lcd::get();
  CM *cm = CM::get();
  //only for menus
  int scrollcount=0, off_wait_time=1000, on_wait_time=100;
  //for menus and enums
  std::vector<Sprite*> menuSpriteList;
  //only for int
  const int int_pow2[10]={1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};
  int int_value=0, int_max_position;
  Sprite* intSpriteList[11];
  //for int and ip
  int int_position=0;
  //for ip
  Sprite* ipSpriteList[15];
  int ip_value[4];

  bool justEnteredMenu = true;
  int menu_nlines = ((s->height()-1)/NS_font[0].height())-1;
  Sprite *menuSpriteTop = new Sprite(s,0,0,s->width(),NS_fontheight);

  for(int i=1;i<=menu_nlines;i++)
    menuSpriteList.push_back(new Sprite(s,0,1+i*NS_fontheight,s->width()-2,NS_fontheight));
  for(int i=0;i<=11;i++){
    const int refW = BS_font['0'].ewidth(), refH = BS_font['0'].height();
    intSpriteList[i] = new Sprite(s,s->width()-3-(refW*(i+1)),(s->height()-refH)/2,refW,refH);
  }
  for(int i=0;i<=15;i++){
    const int refW = NS_font['0'].ewidth(), refH = NS_font['0'].height();
    ipSpriteList[i] = new Sprite(s,s->width()-3-(refW*(i+1)),(s->height()-refH)/2,refW,refH);
  }
  scr->visible(true);
  while(true){
    //render the screen
    if(shown->type==ME_MENU){
      if(justEnteredMenu){
        on_wait_time = 100;
        off_wait_time = 1000;
      }
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
      on_wait_time = 300;
      off_wait_time = 1000;

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
        on_wait_time = 300;
        off_wait_time = 1000;
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
    }else if(shown->type==ME_INTEGER){
      if(justEnteredMenu){
        int_value = cm->get(shown->cfg);
        int_position = 0;
        for(int_max_position=1;
            (int_pow2[int_max_position]<=cm->max(shown->cfg))||
            (int_pow2[int_max_position]<=-cm->min(shown->cfg));
            int_max_position++);

        on_wait_time = 300;
        off_wait_time = 1000;
      }

      s->clear();
      NS_writer.scroll_x=0;
      NS_writer.scroll_y=0;
      NS_writer.canvas = menuSpriteTop;
      NS_writer.renderCentered((char*)DIC::global->get(shown->name));
      char str_val[12]="           ";

      if(int_value>cm->max(shown->cfg))int_value=cm->max(shown->cfg);
      if(int_value<cm->min(shown->cfg))int_value=cm->min(shown->cfg);
      if(int_position>=int_max_position)int_position = int_max_position-1;
      if(int_position<0)int_position=0;
      sprintf(str_val,"%11d",int_value);
      for(int i=0;i<11;i++)BS_font[(uint8_t)str_val[10-i]].copyTo(intSpriteList[i],1,0);
      intSpriteList[int_position]->invert(0,0,99,99);
    }else if(shown->type==ME_IP){
      if(justEnteredMenu){
        uint32_t ip = cm->get(shown->cfg);
        ip_value[0] = (ip>>0 )&0xFF;
        ip_value[1] = (ip>>8 )&0xFF;
        ip_value[2] = (ip>>16)&0xFF;
        ip_value[3] = (ip>>24)&0xFF;
        int_position = 9;
        on_wait_time = 300;
        off_wait_time = 1000;
      }

      s->clear();
      NS_writer.scroll_x=0;
      NS_writer.scroll_y=0;
      NS_writer.canvas = menuSpriteTop;
      NS_writer.renderCentered((char*)DIC::global->get(shown->name));
      char str_val[16];
      int pos=0;

      if(int_position>11)int_position = 11;
      if(int_position<0)int_position=0;
      for(int i=0;i<4;i++){
        if(ip_value[i]<0)ip_value[i]=0;
        if(ip_value[i]>255)ip_value[i]=255;
        if(int_position-2>i*3)pos++;
      }
      pos+=int_position;

      sprintf(str_val,"%3d.%3d.%3d.%3d",ip_value[0],ip_value[1],ip_value[2],ip_value[3]);
      for(int i=0;i<15;i++)NS_font[(uint8_t)str_val[14-i]].copyTo(ipSpriteList[i],1,0);

      ipSpriteList[pos]->invert(0,0,99,99);
    }else{
      PopUpper::get()->popup("Not implemented / error",3000,0);
      break;
    }

    //check for inputs
    if(lastInput){
      lcd->buttons_waitRelease(lastInput,off_wait_time);
    }
    lastInput = lcd->buttons_get(Lcd::BUTTONS_USED);
    if(!lastInput){
      lastInput = lcd->buttons_waitPress(Lcd::BUTTONS_USED,on_wait_time);
    }

    if(shown->type==ME_MENU){

      if(lastInput&(Lcd::BUTTON_BACK|Lcd::BUTTON_LEFT)){
        //Go back, when possible
        if(shown->father==NULL)break;
        shown = shown->father;
      }else if(lastInput&(Lcd::BUTTON_OK|Lcd::BUTTON_RIGHT)){
        //Enter into child
        shown = shown->childs->at(shown->menu_index);
      }else if(lastInput & Lcd::BUTTON_UP){
        shown->menu_index--;
        if(off_wait_time<1000)off_wait_time--;
        else off_wait_time = 300;
      }else if(lastInput & Lcd::BUTTON_DOWN){
        shown->menu_index++;
        if(off_wait_time<1000)off_wait_time--;
        else off_wait_time = 300;
      }else{
        //Return speed to normal
        on_wait_time = 100;
        off_wait_time = 1000;
      }

    }else if(shown->type==ME_INTEGER_C ||
        shown->type==ME_STRING_C ||
        shown->type==ME_IP_C ||
        shown->type==ME_ENUM_C ){

      if(lastInput){
        //Go back with any button
        if(shown->father==NULL)break;
        shown = shown->father;
      }

    }else if(shown->type==ME_ENUM){

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
        if(off_wait_time<1000)off_wait_time--;
        else off_wait_time = 300;
      }else if(lastInput & Lcd::BUTTON_DOWN){
        shown->menu_index++;
        if(off_wait_time<1000)off_wait_time--;
        else off_wait_time = 300;
      }else{
        //Return speed to normal
        on_wait_time = 300;
        off_wait_time = 1000;
      }

    }else if(shown->type==ME_INTEGER){
      if(lastInput & Lcd::BUTTON_BACK){
        //Go back, when possible
        if(shown->father==NULL)break;
        shown = shown->father;
      }else if(lastInput & Lcd::BUTTON_OK){
        //Save and go back
        cm->set(shown->cfg,int_value);
        if(shown->father==NULL)break;
        shown = shown->father;
      }else if(lastInput & Lcd::BUTTON_LEFT){
        //Move digit to left
        int_position++;
      }else if(lastInput & Lcd::BUTTON_RIGHT){
        //Move digit to the right
        int_position--;
      }else if(lastInput & Lcd::BUTTON_UP){
        //increment and increase speed
        int_value += int_pow2[int_position];
        if(off_wait_time<1000)off_wait_time--;
        else off_wait_time = 300;
      }else if(lastInput & Lcd::BUTTON_DOWN){
        //decrement and increase speed
        int_value -= int_pow2[int_position];
        if(off_wait_time<1000)off_wait_time--;
        else off_wait_time = 300;
      }else{
        //Return speed to normal
        on_wait_time = 300;
        off_wait_time = 1000;
      }
    }else if(shown->type==ME_IP){
      if(lastInput & Lcd::BUTTON_BACK){
        //Go back, when possible
        if(shown->father==NULL)break;
        shown = shown->father;
      }else if(lastInput & Lcd::BUTTON_OK){
        //Save and go back
        uint32_t ip=(ip_value[0]<<0)|(ip_value[1]<<8)|(ip_value[2]<<16)|(ip_value[3]<<24);
        cm->set(shown->cfg,ip);
        if(shown->father==NULL)break;
        shown = shown->father;
      }else if(lastInput & Lcd::BUTTON_LEFT){
        //Move digit to left
        int_position++;
      }else if(lastInput & Lcd::BUTTON_RIGHT){
        //Move digit to the right
        int_position--;
      }else if(lastInput & Lcd::BUTTON_UP){
        //increment and increase speed
        ip_value[3-int_position/3] += int_pow2[int_position%3];
        if(off_wait_time<1000)off_wait_time--;
        else off_wait_time = 300;
      }else if(lastInput & Lcd::BUTTON_DOWN){
        //decrement and increase speed
        ip_value[3-int_position/3] -= int_pow2[int_position%3];
        if(off_wait_time<1000)off_wait_time--;
        else off_wait_time = 300;
      }else{
        //Return speed to normal
        on_wait_time = 300;
        off_wait_time = 1000;
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
  for(int i=0;i<11;i++)delete intSpriteList[i];
  for(int i=0;i<15;i++)delete ipSpriteList[i];
}
