/*
 * ConfigManager.cpp
 *
 *  Created on: 17 de mai de 2018
 *      Author: makara
 */

#include "ConfigManager.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

ConfigManager::ConfigManager() {
  int i, memNeeded;
  uint8_t *j;
  for(i=0, memNeeded=0 ; i<(sizeof(params)/sizeof(ConfigParameters)) ; i++){
    if(params[i].type==INTEGER){
      memNeeded += sizeof(int32_t);
    }else if(params[i].type==STRING){
      memNeeded += params[i].max+1;
    }else{ //BAD
      break;
    }
  }
  if(i != CONFIGMANAGER_MAX){
    printf("ERROR in Config manager: %d -/-> %d configs",i,CONFIGMANAGER_MAX);
    //Reboot the esp32?
  }
  buffer = malloc(memNeeded);
  if(buffer == NULL){
    printf("ERROR in Config manager: Could not malloc %d bytes",memNeeded);
    //Reboot the esp32?
  } 
  memset(buffer,0,memNeeded); 
  j = buffer;
  for(i=0 ; i<CONFIGMANAGER_MAX ; i++){
    params[i].p = j;
    if(params[i].type==INTEGER){
      j+=sizeof(int32_t);
    }else if(params[i].type==STRING){
      j+=params[i].max+1;
    }
  }
}

int32_t ConfigManager::get(Config c){
  if(params[c].type==INTEGER)return *((int32_t*)params[c].p);
  if(params[c].type==STRING)return atoi((char*)params[c].p);
  return 0;
}

void ConfigManager::get(Config c, char *s){
  if(s==NULL)return;
  if(params[c].type==INTEGER){
    sprintf(s,"%d",*((int32_t*)params[c].p));
  }else if(params[c].type==STRING){
    strcpy(s, (char*)params[c].p);
  }
}

void ConfigManager::set(Config c, int32_t v){
  if(params[c].type==INTEGER){
    if(v>params[c].max)v = params[c].max;
    if(v<params[c].min)v = params[c].min;
    *((int32_t*)params[c].p) = v;
    params[c].cb(v);
  }else if(params[c].type==STRING){
    char buff[33];
    sprintf(buff,"%d",v);
    if(strlen(buff)<=params[c].max && strlen(buff)>=params[c].min){
      strcpy((char*)params[c], buff);
      params[c].cb(0);
    }
  }
}

void ConfigManager::set(Config c, const char *s){
  if(s==NULL)return;
  if(params[c].type==INTEGER){
    int32_t buff = atoi(s);
    if(buff>params[c].max)buff = params[c].max;
    if(buff<params[c].min)buff = params[c].min;
    *((int32_t*)params[c].p) = buff;
    params[c].cb(buff);
  }else if(params[c].type==STRING){
    if(strlen(s)>=params[c].min){
      strncpy((char*)params[c].p, s, params[c].max);
      params[c].p[params[c].max] = 0;
    }
  }
}

void ConfigManager::setCallback(Config c, void (*cb)(int32_t)){
  params[c].cb = cb;
}

ConfigManager::Config ConfigManager::stringToIndex(const char *s){
  for(int i=0; i<CONFIGMANAGER_MAX ; i++){
    if(!strcmp(s,params[i].name))return i;
  }
  return CONFIGMANAGER_MAX;
}

ConfigManager::Config ConfigManager::setFromString(const char *config, const char *from, char *set){
  Config i = stringToIndex(config);
  if(i==CONFIGMANAGER_MAX){
    set[0] = 0;
    return CONFIGMANAGER_MAX;
  }
  set(i, from);
  get(i, set);
  return i;
}

int ConfigManager::getFullJSON(const char *s, int maxlen){
  //https://www.json.org/json-pt.html
  //Implementing only the first level of object, ASCII string and integer
  int i, len;
  char *p = s;
  len = snprintf(p,maxlen,"{");
  if(len<0)return -(int)(p-s);
  p += len;
  maxlen -= len;
  for(i=0;i<CONFIGMANAGER_MAX && maxlen>0;i++){
    if(params[i].type==INTEGER){
      len = snprintf(p, maxlen,"\"%s\":%d,",
          params[i].name,*((int32_t*)params[i].p));
      if(len<0)return -(int)(p-s);
    }else if(params[i].type==STRING){
      char buff[100];
      char2unicode((char*)params[i].p, buff, 100);
      len = snprintf(p, maxlen, "\"%s\":\"%d\",",params[i].name,buff);
      if(len<0)return -(int)(p-s);
    }
    p += len;
    maxlen -= len;
  }
  len = snprintf(p-1,maxlen,"}");
  if(len<0)return -(int)(p-s);
  p+=len;
  maxlen-=len;
  if(maxlen<=0)return -(int)(p-s);
  return (int)(p-s);
}

int ConfigManager::setFromJSON(const char *s){
  return 0;
}

int ConfigManager::char2unicode(const char *s, char *j, int max){
  char *lj = j, *ls = s;
  int unicode, size=1;
  max--;
  *lj='"';
  lj++;
  while(*ls!=0 && size<max-5){
	if(*ls=='"' || *ls=='\\' || *ls=='/' || *ls=='\b' ||
			*ls=='\f'|| *ls=='\n' || *ls=='\r'|| *ls=='\t'){
      lj[0]='\\';
      switch(*ls){
        case '"':
        case '\\':
        case '/': lj[1]=*ls; break;
        case '\b':lj[1]='b'; break;
        case '\f':lj[1]='f'; break;
        case '\n':lj[1]='n'; break;
        case '\r':lj[1]='r'; break;
        case '\t':lj[1]='t'; break;
      }
      lj+=2;
      size+=2;
	}else if(*ls>=' ' && *ls<='~'){
      *lj = *ls;
      lj++;
      size++;
	}else{
      sprintf(lj,"\\u%4X",*ls);
      lj+=6;
      size+=6;
	}
    ls++;
  }
  *lj='"';
  return size;
}

int ConfigManager::unicode2char(const char *j, char *s, int max){
  char *lj = j, *ls = s;
  int unicode, size=0;
  max--;
  while(*lj!=0 && size<max){
    if(*lj == '"'){
      lj++;
    }else if(*lj == '\\'){
      lj++;
      switch(*lj){
        case 0:break;
        case '"': *ls='"';ls++;size++;break;
        case '\\': *ls='\\';ls++;size++;break;
        case '/': *ls='/';ls++;size++;break;
        case 'b': *ls='\b';ls++;size++;break;
        case 'f': *ls='\f';ls++;size++;break;
        case 'n': *ls='\n';ls++;size++;break;
        case 'r': *ls='\r';ls++;size++;break;
        case 't': *ls='\t';ls++;size++;break;
        case 'u':
          unicode = ((lj[1]-'0')<<12)|((lj[2]-'0')<<8)|
            ((lj[3]-'0')<<4)|(lj[4]-'0');
          *ls = unicode & 0xFF;
          size++;
          ls++;
          lj+=4;
          break;
      }
      lj++;
    }else{
      *ls = *lj;
      ls++;
      ls++;
      size++;
    }
  }
  *ls = 0;
  return size;
}
