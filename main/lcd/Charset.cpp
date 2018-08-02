/*
 * Charset.cpp
 *
 *  Created on: 4 de mai de 2018
 *      Author: makara
 */

#include "Charset.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Charset* Charset::singleton = 0;
Charset* Charset::get(){
  if(singleton == 0)singleton = new Charset();
  return singleton;
}

Charset::Charset() {
  chainlist = NULL;
}

void Charset::loadFromBytes(const uint8_t *b, uint32_t size, int16_t index){
  uint8_t *p;
  FONT *l, *nova = new FONT();
  nova->buffer = (uint8_t*)malloc(size);
  memcpy(nova->buffer,b,size);
  nova->height = b[0];
  if(index<0)nova->index = nova->height;
  else nova->index = index;
  p = nova->buffer+1;
  for(int i=0;i<128;i++)p=nova->chars[i].loadFrom(nova->height,p);
  nova->next = NULL;
  if(chainlist==NULL){
    chainlist = nova;
  }else{
    l = chainlist;
    while(l->next!=NULL)l=l->next;
    l->next = nova;
  }
}

uint32_t Charset::height(uint8_t index) {
  FONT* f = chainlist;
  if(f==NULL)return 0;
  do{
    if(f->index==index)return f->height;
    f = f->next;
  }while(f!=NULL);
  printf("Alert: Charset not found %d",index);
  return chainlist->height;
}

Character* Charset::at(uint8_t index, uint8_t c){
  FONT* f = chainlist;
  if(f==NULL)return &fail;
  if(c>=128)return &fail;
  while(f->next!=NULL){
    if(f->index==index){
      return &f->chars[c];
    }
    f = f->next;
  }
  if(f->index==index){
    return &f->chars[c];
  }else{
    printf("!");
    return &chainlist->chars[c];
  }
}

Character* Charset::asArray(uint8_t index){
  FONT* f = chainlist;
  if(f==NULL)return NULL;
  while(f->next!=NULL){
    if(f->index==index){
      return f->chars;
    }
    f = f->next;
  }
  if(f->index==index){
    return f->chars;
  }else{
    printf("!");
    return chainlist->chars;
  }
}
