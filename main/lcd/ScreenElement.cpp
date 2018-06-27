/*
 * ScreenElement.cpp
 *
 *  Created on: 17 de jun de 2018
 *      Author: makara
 */

#include "ScreenElement.h"

#include <stdint.h>

ScreenElement::ScreenElement(Sprite *_sprite, int _zIndex, bool _visible, bool _invert){
	spr = _sprite;
	z = _zIndex;
	v = _visible;
	last = NULL;
	next = NULL;
	posx = 0;
	posy = 0;
	mode = Sprite::COPY_BOTH;
}

ScreenElement::~ScreenElement(){
	if(last==NULL){// this is a list
		ScreenElement *p = this;
		while(p->next){
			p=p->next;
			p->last->last = NULL;
			p->last->next = NULL;
			p->last->z = INT32_MIN;
		}
	}
}

void ScreenElement::addToList(ScreenElement *listAddress){
	ScreenElement *n = listAddress->next;
	ScreenElement *l = listAddress;
	if(n)n->last = this;
	next = n;
	last = l;
	if(l)l->next = this;
	reposition();
}

ScreenElement* ScreenElement::createList(Sprite *dest){
	return new ScreenElement(dest, INT32_MIN, true, false);
}

void ScreenElement::renderInto(Sprite *dest){
	ScreenElement *p = this;
	while(p->last!=NULL)p = p->last;
	p = p->next;
	while(p){
		if(p->spr!=NULL && p->v){
			p->spr->copyTo(dest,p->posx,p->posy,p->mode);
		}
		p = p->next;
	}
}

void ScreenElement::sprite(Sprite *s){
	spr = s;
}

Sprite* ScreenElement::sprite(){
	return spr;
}

void ScreenElement::zIndex(int _z){
	z = _z;
	reposition();
}

int  ScreenElement::zIndex(){
	return z;
}

void ScreenElement::visible(bool _v){
	v = _v;
}

bool ScreenElement::visible(){
	return v;
}

void ScreenElement::setPosition(int32_t x, int32_t y, uint_fast8_t _mode){
	posx = x;
	posy = y;
	mode = _mode;
}

int32_t ScreenElement::getPosX(){
	return posx;
}

int32_t ScreenElement::getPosY(){
	return posy;
}

uint_fast8_t ScreenElement::getMode(){
	return mode;
}

void ScreenElement::reposition(){
	ScreenElement *e;
	if(last){
		if(z < last->z){
			next->last = last;
			last->next = next;
			e = last;
			while(e->last){
				if(e->last->z < z)break;
				e=e->last;
			}
			last = e->last;
			next = e;
			last->next = this;
			next->last = this;
		}
	}
	if(next){
		if(z > next->z){
			next->last = last;
			last->next = next;
			e = next;
			while(e->next){
				if(e->next->z > z)break;
				e=e->next;
			}
			last = e;
			next = e->next;
			last->next = this;
			next->last = this;
		}
	}
}
