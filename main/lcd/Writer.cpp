/*
 * Writer.cpp
 *
 *  Created on: 4 de mai de 2018
 *      Author: makara
 */

#include "Writer.h"
#include <stddef.h>
#include <stdio.h>

Writer::Writer():
	set(NULL), canvas(NULL), text(NULL),
	scroll_x(0), scroll_y(0), no_overflow_x(0)
{ }

Writer::Writer(Charset *s, Sprite *c, char *t):
	set(s), canvas(c), text(t),
	scroll_x(0), scroll_y(0), no_overflow_x(0)
{ }

int32_t Writer::max_scroll(int32_t *max_x, int32_t *max_y){
	int32_t x=0, y=0, o=0;/*
	if(set==NULL || canvas==NULL){
		printf("Writer ERROR: Charset or Canvas not specified!\n");
		return 0;
	}
	y = set->height();
	for(o=0;text[o]!='\0';o++){
		if(text[0]=='\n'){
			x=0;
			y+=set->height();
		}else{
			Character *c = set->at(text[o]);
			if(no_overflow_x){
				if(c->width()+x<canvas->width()){
					x += c->ewidth();
					if(x>canvas->width()){
						x=0;
						y+=set->height();
					}
				}else{
					x = c->ewidth();
					y +=set->height();
				}
			}else{
				x+=c->ewidth();
			}
		}
	}
	x-=canvas->width();
	y-=canvas->height();
	if(x<0)x=0;
	if(y<0)y=0;
	if(max_x!=NULL)*max_x = x;
	if(max_y!=NULL)*max_y = y;*/
	return y;
}

void Writer::fast_print(char *t){
	text = t;
	render();
}
void Writer::render(){
	/*int32_t x = -scroll_x, y = -scroll_y, o;
	if(set==NULL || canvas==NULL){
		printf("Writer ERROR: Charset or Canvas not specified!\n");
		return;
	}
	canvas->clear();
	if(text==NULL)return;

	for(o=0;text[o]!='\0';o++){
		if(text[o]=='\n'){
			x=0;
			y+=set->height();
		}else{
			Character *c = set->at(text[o]);
			if(no_overflow_x){
				if(c->width()+x<canvas->width()){
					x = c->print(canvas,x,y);
					if(x>canvas->width()){
						x=0;
						y+=set->height();
					}
				}else{
					y +=set->height();
					x = c->print(canvas,0,y);
				}
			}else{
				x = c->print(canvas,x,y);
			}
		}
	}*/
}
