/*
 * PopUpper.cpp
 *
 *  Created on: 26 de jun de 2018
 *      Author: makara
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "PopUpper.h"
#include "../lcd/Lcd.h"
#include "../lcd/Charset.h"

PopUpper* PopUpper::singleton = 0;
PopUpper* PopUpper::get(){
  if(singleton == 0)singleton = new PopUpper();
  return singleton;
}

PopUpper::PopUpper() {
  DefaultTime = 1000;
  DefaultSize = 6;
  setZ(100);
  queue = xQueueCreate( 10, sizeof( POPUP ) );
  xTaskCreate(task, "PopUpper", 2048, this, 1, &t_handle);
}

void PopUpper::setZ(int _z){ screen.zIndex(_z); }
int PopUpper::getZ(){ return screen.zIndex(); }

void PopUpper::task(void* arg){
  PopUpper *popupper = (PopUpper*) arg;
  ScreenElement *s = &popupper->screen;
  Lcd *l = Lcd::get();
  s->addToList(l->root);
  while(true){
    POPUP p;
    if(xQueueReceive(popupper->queue,&p,100000)){
      uint32_t sx, sy;
      //Calculate the size of the generated sprite
      uint32_t widths[10]={0}, breaks[10]={0}, wordwidth, lines, lineheight;
      Character *cs = Charset::get()->asArray(p.size);
      lineheight = cs[0].height();
      wordwidth = 0;
      lines = 0;
      sx = 15;
      for(int i=0;;i++){
        if(p.msg[i]==0){
          if(wordwidth+widths[lines]>l->width()-4){
            if(widths[lines]>sx)sx = widths[lines];
            lines++;
            widths[lines]=0;
          }
          breaks[lines]=i;
          widths[lines]+=wordwidth;
          wordwidth=0;
          if(widths[lines]>sx)sx = widths[lines];
          break;
        }
        wordwidth+=cs[(uint8_t)p.msg[i]].ewidth();
        if(p.msg[i]==' '){
          if(wordwidth+widths[lines]>l->width()-4){
            if(widths[lines]>sx)sx = widths[lines];
            lines++;
            widths[lines]=0;
          }
          breaks[lines]=i;
          widths[lines]+=wordwidth;
          wordwidth=0;
        }
      }
      lines++;
      if(lines> (l->height()-4)/lineheight)lines = (l->height()-4)/lineheight;
      sy = lines*lineheight+4;
      if(sx>l->width()-6)sx = l->width()-6;
      sx+=6;

      //Create and position the sprite
      s->spr = new Sprite(sx+2,sy+2);
      s->spr->clear();
      s->spr->line(0,0,sx+1,0);
      s->spr->line(0,0,0,sy+1);
      s->spr->line(sx+1,0,sx+1,sy+1);
      s->spr->line(0,sy+1,sx+1,sy+1);
      s->setPosition((l->width()-sx-2)/2,(l->height()-sy-2)/2);
      //Render the text
      printf("Popup:%s\n",p.msg);
      uint32_t startx = (sx+2-widths[0])/2;
      uint32_t starty = 2;
      lines = 0;
      for(int i=0;p.msg[i];i++){
        startx = cs[(uint8_t)p.msg[i]].print(s->spr,startx,starty);
        if(breaks[lines]==i){
          lines++;
          startx = (sx-widths[lines])/2;
          starty += lineheight;
        }
      }
      //display
      s->visible(true);
      l->buttons_waitPress(Lcd::BUTTONS_USED,p.time);
      s->visible(false);
      delete s->spr;
      s->spr = NULL;
    }else vTaskDelay(1);
  }
}

void PopUpper::popup(const char *msg, uint32_t time, uint8_t size){
  POPUP p;
  //BaseType_t woken = pdFALSE;
  strncpy(p.msg,msg,127);
  p.msg[127]=0;
  if(time==0)time = DefaultTime;
  if(size==0)size = DefaultSize;
  p.size = size;
  p.time = time;
  xQueueSend(queue,&p,0);
  //xQueueSendFromISR(queue,&p,&woken);
}

void PopUpper::popup(uint32_t time, uint8_t size, const char *fmt, ...){
  POPUP p;
  va_list ap;
  if(time==0)time = DefaultTime;
  if(size==0)size = DefaultSize;
  p.size = size;
  p.time = time;
  va_start(ap, fmt);
  vsnprintf(p.msg,128,fmt,ap);
  va_end(ap);
  xQueueSend(queue,&p,0);
}
