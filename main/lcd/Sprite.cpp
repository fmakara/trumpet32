/*
 * Sprite.cpp
 *
 *  Created on: 26 de abr de 2018
 *      Author: makara
 */

#include "Sprite.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

Sprite::Sprite(uint8_t *memory, uint32_t _width, uint32_t _height, uint32_t _step) {
	w = _width;
	h = _height;
	s = _step;
	oy = 0;
	buffer = memory;
	if(s<(h+7)/8)s = (h+7)/8;
}

Sprite::Sprite(Sprite *origin, uint32_t offx, uint32_t offy, uint32_t width, uint32_t height){
	if(offx+1>origin->width())offx=origin->width()-1;
	if(offy+1>origin->height())offx=origin->height()-1;
	if(offx+width>origin->width())width=origin->width()-offx;
	if(offy+height>origin->height())width=origin->height()-offy;

	offy+=origin->bitoffset();
	w = width;
	h = height;
	s = origin->step();
	oy = offy%8;
	buffer = origin->memory()+s*offx+offy/8;
}


Sprite::~Sprite() {}
uint32_t Sprite::width(){return w;}
uint32_t Sprite::height(){return h;}
uint8_t* Sprite::memory(){return buffer;}
uint32_t Sprite::step(){return s;}
uint32_t Sprite::bitoffset(){return oy;}
void Sprite::setMemory(uint8_t *memory){buffer = memory;}


void Sprite::clear(){
	uint64_t m = (((uint64_t)1<<h)-1)<<oy;
	uint64_t *p;
	for(uint32_t x=0;x<w;x++){
		p = (uint64_t*)(buffer+x*s);
		*p &= ~m;
	}
}
void Sprite::set(){
	for(uint32_t x=0;x<w;x++){
		uint64_t m = (((uint64_t)1<<h)-1)<<oy;
		uint64_t *p = (uint64_t*)(buffer+x*s);
		*p |= m;
	}
}

uint_fast8_t Sprite::at(uint32_t x, uint32_t y){
	if(x>=w||y>=h)return 0;
	uint64_t *p = (uint64_t*)(buffer+x*s);
	if( (((*p)>>(y+oy))&1) ==1)return 1;
	else return 0;
}

void Sprite::dot(uint32_t x, uint32_t y, uint_fast8_t value){
	if(x>=w||y>=h)return;
	if(value){
		*(uint64_t*)(buffer+x*s) |=  (uint64_t)1<<(y+oy);
	}else{
		*(uint64_t*)(buffer+x*s) &=~((uint64_t)1<<(y+oy));
	}
}

void Sprite::copyTo(Sprite *m, int32_t x, int32_t y, uint_fast8_t which){
	uint32_t i;
	int32_t fromx, tox, copyw;
	int32_t fromy, toy, copyh;
	uint64_t fromline, *toline, mask;
	if(x>=0){
		if(x>m->width())return;
		fromx = 0;
		tox = x;
	}else{
		if(-x>w)return;
		fromx = -x;
		tox = 0;
	}
	copyw = w-fromx;
	if(tox+copyw>m->width()){
		copyw = m->width()-tox;
	}

	if(y>=0){
		if(y>m->height())return;
		fromy = 0;
		toy = y;
	}else{
		if(-y>h)return;
		fromy = -y;
		toy = 0;
	}
	copyh = h-fromy;
	if(toy+copyh>m->height()){
		copyh = m->height()-toy;
	}

	mask = ((uint64_t)1<<(copyh))-1;// Ex.: 3 -> 0b1000 - 1 = 0b0111
	mask = mask<<(toy+m->bitoffset());
	fromy += oy;
	if(which==COPY_BLACK){
		for(i=0;i<copyw;i++){
			fromline = (*(uint64_t*)(buffer+s*(i+fromx)));
			toline = (uint64_t*)(m->memory()+m->step()*(i+tox));
			fromline = (fromline>>fromy)<<(toy+m->bitoffset());
			*toline = (*toline)|(fromline&mask);
		}
	}else if(which==COPY_WHITE){
		mask = ~mask;
		for(i=0;i<copyw;i++){
			fromline = (*(uint64_t*)(buffer+s*(i+fromx)));
			toline = (uint64_t*)(m->memory()+m->step()*(i+tox));
			fromline = (fromline>>fromy)<<(toy+m->bitoffset());
			*toline = (*toline)&(fromline|mask);
		}
	}else{//COPY_BOTH - Default
		for(i=0;i<copyw;i++){
			fromline = (*(uint64_t*)(buffer+s*(i+fromx)));
			toline = (uint64_t*)(m->memory()+m->step()*(i+tox));
			fromline = (fromline>>fromy)<<(toy+m->bitoffset());
			*toline = ((*toline)&(~mask))|(fromline&mask);
		}
	}

}

void Sprite::line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint8_t color){
	uint64_t *toline, mask;
	int32_t ix, iy, dx, dy, i, xi, yi, D;
	if(x0==x1){
		if(x0<0 || x0>=w)return;
		if(y0>y1){
			i = y0;
			y0 = y1;
			y1 = i;
		}
		if(y1<0 || y0>=h)return;
		if(y0<0)y0=0;
		if(y1>=h)y1=h-1;
		toline =(uint64_t*)(buffer + s*x0);
		mask = (((uint64_t)1<<(1+y1-y0))-1)<<(y0+oy);
		if(color)*toline |= mask;
		else *toline &= ~mask;
		return;
	}
	if(y0==y1){
		if(y0<0 || y0>=h)return;
		if(x0>x1){
			i = x0;
			x0 = x1;
			x1 = i;
		}
		if(x1<0 || x0>=w)return;
		if(x0<0)x0=0;
		if(x1>=w)x1=w-1;
		mask = ((uint64_t)1<<(y0+oy));
		for(i=x0;i<=x1;i++){
			toline =(uint64_t*)(buffer + s*i);
			if(color)*toline |= mask;
			else *toline &= ~mask;
		}
		return;
	}

	//Bresenham's Line Algorithm
	dx = x1 - x0;
	dy = y1 - y0;
	ix = 1;
	iy = 1;
	xi = x0;
	yi = y0;
	if(dx<0){
		ix = -1;
		dx = -dx;
	}
	if(dy<0){
		iy = -1;
		dy = -dy;
	}
	if(dx>=dy){
		if(x0>x1){
			xi = x1;
			yi = y1;
			iy = -iy;
		}
		D = 2*dy - dx;
		for(i=0;i<=dx;i++){
			dot(i+xi,yi,color);
			if(D>0){
				yi += iy;
				D -= 2*dx;
			}
			D += 2*dy;
		}
	}else{
		if(y0>y1){
			xi = x1;
			yi = y1;
			ix = -ix;
		}
		D = 2*dx - dy;
		for(i=0;i<=dy;i++){
			dot(xi,i+yi,color);
			if(D>0){
				xi += ix;
				D -= 2*dy;
			}
			D += 2*dx;
		}
	}
}

void Sprite::circle(int32_t x0, int32_t y0, int32_t r, uint8_t color){
	int32_t x = r-1;
	int32_t y = 0;
	int32_t dx = 1;
	int32_t dy = 1;
	int32_t err = dx - (r << 1);

	while (x >= y){
		dot(x0 + x, y0 + y);
		dot(x0 + y, y0 + x);
		dot(x0 - y, y0 + x);
		dot(x0 - x, y0 + y);
		dot(x0 - x, y0 - y);
		dot(x0 - y, y0 - x);
		dot(x0 + y, y0 - x);
		dot(x0 + x, y0 - y);

		if (err <= 0){
			y++;
			err += dy;
			dy += 2;
		}

		if (err > 0){
			x--;
			dx += 2;
			err += dx - (r << 1);
		}
	}
}
void Sprite::invert(int32_t x0, int32_t y0, int32_t x1, int32_t y1){
	int32_t dy, i;
	uint64_t mask;
	if(x0<0)x0=0;
	if(x1<0)x1=0;
	if(y0<0)y0=0;
	if(y1<0)y1=0;
	if(x0>=width())x0=width()-1;
	if(x1>=width())x1=width()-1;
	if(y0>=height())y0=height()-1;
	if(y1>=height())y1=height()-1;
	dy = y1-y0;
	if(x1<x0){
		i = x0;
		x0 = x1;
		x1 = i;
	}
	if(dy<0){
		dy = -dy;
		y0 = y1;
	}
	mask = (((uint64_t)1<<(dy+1))-1)<<(y0+bitoffset());
	for(i=x0 ; i<=x1 ; i++){
		*((uint64_t*)(memory()+step()*i)) ^= mask;
	}
}
