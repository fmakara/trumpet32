/*
 * Sprite.h
 *
 *  Created on: 26 de abr de 2018
 *      Author: makara
 */
#ifndef MAIN_LCD_SPRITE_H_
#define MAIN_LCD_SPRITE_H_
#include <stdint.h>

class Sprite {
public:
	Sprite(uint32_t _width, uint32_t _height, uint32_t _step=0);
	Sprite(uint8_t *memory, uint32_t _width, uint32_t _height, uint32_t _step=0);
	Sprite(Sprite *origin, uint32_t offx, uint32_t offy, uint32_t width, uint32_t height);
	virtual ~Sprite();
	uint32_t width();
	uint32_t height();
	uint32_t step();
	uint32_t bitoffset();
	uint8_t* memory();
	void setMemory(uint8_t* memory);

	static const uint_fast8_t COPY_BOTH = 0;
	static const uint_fast8_t COPY_BLACK = 1;
	static const uint_fast8_t COPY_WHITE = 2;

	uint_fast8_t at(uint32_t x, uint32_t y);
	void dot(uint32_t x, uint32_t y, uint_fast8_t value=1);
	void copyTo(Sprite *m, int32_t x, int32_t y, uint_fast8_t which=COPY_BOTH);

	void clear();
	void set();
	void line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint8_t color=1);
	void circle(int32_t x0, int32_t y0, int32_t r, uint8_t color=1);
	void invert(int32_t x0, int32_t y0, int32_t x1, int32_t y1);
protected:
	uint32_t w, h, s, oy;
	uint8_t *buffer;
	bool dynamic;
};

#endif /* MAIN_LCD_SPRITE_H_ */
