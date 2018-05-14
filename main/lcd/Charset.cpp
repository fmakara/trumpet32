/*
 * Charset.cpp
 *
 *  Created on: 4 de mai de 2018
 *      Author: makara
 */

#include "Charset.h"

Charset::Charset(uint8_t *src) {
	uint8_t *p = src+1;
	h = src[0];
	for(int i=0;i<128;i++)p=chars[i].loadFrom(h,p);
}

uint32_t Charset::height() { return h; }


Character* Charset::at(uint8_t c){
	if(c<128)return &chars[c];
	else return &chars[0];
}
