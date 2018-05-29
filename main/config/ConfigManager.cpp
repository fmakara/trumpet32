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


#include "nvs_flash.h"
#include "nvs.h"
#include "lwip/ip4_addr.h"


ConfigManager* ConfigManager::singleton = 0;
ConfigManager* ConfigManager::get(){
	if(singleton==0)singleton = new ConfigManager();
	return singleton;
}

int32_t ConfigManager::min(Config c){return params[c].min;}
int32_t ConfigManager::max(Config c){return params[c].max;}
ConfigManager::ConfigType ConfigManager::type(Config c){return params[c].type;}

ConfigManager::ConfigManager() {
  int i, memNeeded;
  uint8_t *j;
  for(i=0, memNeeded=0 ; i<(sizeof(params)/sizeof(ConfigParameters)) ; i++){
    if(params[i].type==INTEGER){
      memNeeded += sizeof(int32_t);
    }else if(params[i].type==STRING){
      memNeeded += params[i].max+1;
    }else if(params[i].type==IP){
      memNeeded += sizeof(uint32_t);
    }else{ //BAD
      break;
    }
  }
  if(i != CONFIGMANAGER_MAX){
    printf("ERROR in Config manager: %d -/-> %d configs",i,CONFIGMANAGER_MAX);
    //Reboot the esp32?
  }
  buffer = (uint8_t*)malloc(memNeeded);
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
    }else if(params[i].type==IP){
	  j+=sizeof(uint32_t);
    }
  }
}

int32_t ConfigManager::get(Config c){
  if(params[c].type==INTEGER)return *((int32_t*)params[c].p);
  if(params[c].type==STRING)return atoi((char*)params[c].p);
  if(params[c].type==IP)return *((int32_t*)params[c].p);
  return 0;
}

void ConfigManager::get(Config c, char *s){
  if(s==NULL)return;
  if(params[c].type==INTEGER){
    sprintf(s,"%d",*((int32_t*)params[c].p));
  }else if(params[c].type==STRING){
    strcpy(s, (char*)params[c].p);
    if(c==WIFI_STA_SSID){
    	for(int i=0;s[i];i++)printf(".%02X",s[i]);
    	printf("\n");
    }
  }else if(params[c].type==IP){
	uint8_t *ip = (uint8_t*)params[c].p;
	sprintf(s,"%u.%u.%u.%u",ip[0],ip[1],ip[2],ip[3]);
  }
}

void ConfigManager::set(Config c, int32_t v, bool fromweb){
  if(fromweb && !params[c].webWriteable)return;
  if(params[c].type==INTEGER){
    if(v>params[c].max)v = params[c].max;
    if(v<params[c].min)v = params[c].min;
    *((int32_t*)params[c].p) = v;
    if(params[c].cb)params[c].cb(v);
  }else if(params[c].type==STRING){
    char buff[33];
    sprintf(buff,"%d",v);
    if(strlen(buff)<=params[c].max && strlen(buff)>=params[c].min){
      strcpy((char*)params[c].p, buff);
      if(params[c].cb)params[c].cb(0);
    }
  }else if(params[c].type==IP){
	*((uint32_t*)params[c].p) = v;
	if(params[c].cb)params[c].cb(v);
  }
}

void ConfigManager::set(Config c, const char *s, bool fromweb){
  if(fromweb && !params[c].webWriteable)return;
  if(s==NULL)return;
  if(params[c].type==INTEGER){
    int32_t buff = atoi(s);
    if(buff>params[c].max)buff = params[c].max;
    if(buff<params[c].min)buff = params[c].min;
    *((int32_t*)params[c].p) = buff;
    if(params[c].cb)params[c].cb(buff);
  }else if(params[c].type==STRING){
    if(strlen(s)>=params[c].min){
      strncpy((char*)params[c].p, s, params[c].max);
      params[c].p[params[c].max] = 0;
      if(c==WIFI_STA_SSID){
          	for(int i=0;s[i];i++)printf(" %02X",s[i]);
          	printf("\n");
          }
      if(params[c].cb)params[c].cb(0);
    }
  }else if(params[c].type==IP){
	ip4_addr_t ip;
	if(ip4addr_aton(s, &ip)){
		*((uint32_t*)params[c].p) = (uint32_t)ip.addr;
		if(params[c].cb)params[c].cb((uint32_t)ip.addr);
	}
  }
}

void ConfigManager::setCallback(Config c, void (*cb)(int32_t)){
  params[c].cb = cb;
}

ConfigManager::Config ConfigManager::stringToIndex(const char *s){
  int i;
  char local[33];
  for(i=0 ; s[i] && i<32 ; i++){
	if(s[i]>='a' && s[i]<='z')local[i]=s[i]-'a'+'A';
	else local[i]=s[i];
  }
  if(i>=32)return CONFIGMANAGER_MAX;
  local[i]=0;
  for(i=0; i<CONFIGMANAGER_MAX ; i++){
    if(!strcmp(local,params[i].name))return (Config)i;
  }
  return CONFIGMANAGER_MAX;
}

ConfigManager::Config ConfigManager::setFromString(const char *config, const char *from, char *valset, bool fromweb){
  Config i = stringToIndex(config);
  if(i==CONFIGMANAGER_MAX){
	valset[0] = 0;
    return CONFIGMANAGER_MAX;
  }
  if(from!=NULL)set(i, from, fromweb);
  get(i, valset);
  return i;
}

int ConfigManager::getFullJSON(char *s, int maxlen){
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
      len = snprintf(p, maxlen, "\"%s\":%s,",params[i].name,buff);
      if(len<0)return -(int)(p-s);
    }else if(params[i].type==IP){
      uint8_t *ip = (uint8_t*)params[i].p;
	  len = snprintf(p, maxlen, "\"%s\":\"%u.%u.%u.%u\",",params[i].name,
			  ip[0],ip[1],ip[2],ip[3]);
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

int ConfigManager::setFromJSON(char *s){
  return 0;
}

int ConfigManager::char2unicode(char *s, char *j, int max){
  char *lj = j, *ls = s;
  int size=1;
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
	  if(*(ls+1)>=' ' && *(ls+1)<='~'){
	      sprintf(lj,"\\u%04X",*(ls));
	      printf("%04X ",*(ls));
	      ls++;
	  }else{
	      sprintf(lj,"\\u%02X%02X",*(ls),*(ls+1));
	      printf("%02X %02X %c %c ",*(ls),*(ls+1),*(ls),*(ls+1));
	      ls+=2;
	  }
      lj+=6;
      size+=6;
	}
    ls++;
  }
  lj[0]='"';
  lj[1]=0;
  size+=2;
  return size;
}

int ConfigManager::unicode2char(char *j, char *s, int max){
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
	      printf("%04X ",unicode);
          if(unicode>0xFF){
        	  *(ls+1) = (unicode>>8) & 0xFF;
              *(ls) = unicode & 0xFF;
              ls+=2;
              size+=2;
          }else{
              *(ls) = unicode & 0xFF;
              ls++;
              size++;
          }
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

esp_err_t ConfigManager::saveToNVM(){
	int i;
	esp_err_t err;
	nvs_handle handle;
	err = nvs_open("TRUMPET", NVS_READWRITE, &handle);
	if(err!=ESP_OK)return err;
	for(i=0;i<CONFIGMANAGER_MAX;i++){
		if(params[i].nvmWriteable){
			if(params[i].type==INTEGER){
				err = nvs_set_i32(handle, params[i].name, *((int32_t*)params[i].p));
			}else if(params[i].type==STRING){
				err = nvs_set_str(handle, params[i].name, (char*)params[i].p);
			}else if(params[i].type==IP){
				err = nvs_set_u32(handle, params[i].name, *((uint32_t*)params[i].p));
			}
			if(err!=ESP_OK){
				nvs_close(handle);
				return err;
			}
		}
	}
	err = nvs_commit(handle);
	nvs_close(handle);
	return err;
}

esp_err_t ConfigManager::loadFromNVM(){
	int i;
	esp_err_t err;
	nvs_handle handle;
	err = nvs_open("TRUMPET", NVS_READONLY, &handle);
	if(err==ESP_ERR_NVS_NOT_FOUND){
		err = nvs_open("TRUMPET", NVS_READWRITE, &handle);
	}
	if(err!=ESP_OK)return err;
	for(i=0;i<CONFIGMANAGER_MAX;i++){
		if(params[i].nvmWriteable){
			if(params[i].type==INTEGER){
				err = nvs_get_i32(handle, params[i].name, ((int32_t*)params[i].p));
			}else if(params[i].type==STRING){
				size_t max = params[i].max;
				err = nvs_get_str(handle, params[i].name, (char*)params[i].p, &max);
			}else if(params[i].type==IP){
				err = nvs_get_u32(handle, params[i].name, ((uint32_t*)params[i].p));
			}
			if(!(err==ESP_OK||err==ESP_ERR_NVS_NOT_FOUND)){
				nvs_close(handle);
				return err;
			}
		}
	}
	err = nvs_commit(handle);
	nvs_close(handle);
	return err;
}
