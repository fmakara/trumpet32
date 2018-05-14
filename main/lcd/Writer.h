/*
 * Writer.h
 *
 *  Created on: 4 de mai de 2018
 *      Author: makara
 */

#ifndef MAIN_LCD_WRITER_H_
#define MAIN_LCD_WRITER_H_

#include "Charset.h"
#include <stddef.h>

class Writer {
public:
	Writer();
	Writer(Charset *s, Sprite *c, char *t);
	Charset *set;
	Sprite *canvas;
	char *text;
	int32_t scroll_x, scroll_y;
	uint8_t no_overflow_x;
	int32_t max_scroll(int32_t *max_x=NULL, int32_t *max_y=NULL);
	void fast_print(char *t);
	void render();
};

#endif /* MAIN_LCD_WRITER_H_ */
