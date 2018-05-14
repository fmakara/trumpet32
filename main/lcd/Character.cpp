/*
 * Character.cpp
 *
 *  Created on: 4 de mai de 2018
 *      Author: makara
 */

#include "Character.h"
#include <stddef.h>

Character::Character():Sprite(NULL, 0, 0, 0) {
	o=0;
}

uint8_t* Character::loadFrom(uint8_t height, uint8_t *mem){
	w = mem[0];
	o = mem[1];
	h = height;
	s = (h+7)/8;
	buffer = mem+2;
	return mem+2+s*w;
}

uint32_t Character::ewidth() { return s+o; }
uint32_t Character::offset() { return o; }

int32_t Character::print(Sprite *m, int32_t x, int32_t y){
	copyTo(m,x,y);
	return x+w+o;
}
