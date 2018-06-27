/*
 * Charset.h
 *
 *  Created on: 4 de mai de 2018
 *      Author: makara
 */

#ifndef MAIN_LCD_CHARSET_H_
#define MAIN_LCD_CHARSET_H_

#include "Character.h"

class Charset {
public:
	static Charset* get();
	uint32_t height(uint8_t index);
	Character* at(uint8_t index, uint8_t c);
	Character* asArray(uint8_t index);
	void loadFromBytes(const uint8_t *b, uint32_t size, int16_t index=-1);
protected:
	struct FONT {
		uint8_t *buffer;
		Character chars[128];
		uint8_t height;
		uint8_t index;
		struct FONT *next;
	};
	FONT* closestTo(uint8_t _h);
	Charset();
	FONT *chainlist;
	Character fail;
	static Charset *singleton;
};

#endif /* MAIN_LCD_CHARSET_H_ */
