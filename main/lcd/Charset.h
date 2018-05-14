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
	Charset(uint8_t *src);
	uint32_t height();
	Character* at(uint8_t c);
protected:
	uint8_t h;
	Character chars[128];
};

#endif /* MAIN_LCD_CHARSET_H_ */
