/*
 * Character.h
 *
 *  Created on: 4 de mai de 2018
 *      Author: makara
 */

#ifndef MAIN_LCD_CHARACTER_H_
#define MAIN_LCD_CHARACTER_H_

#include <stdint.h>
#include "Sprite.h"

class Character: public Sprite {
public:
  Character();
  uint8_t* loadFrom(uint8_t height, uint8_t *mem);
  int32_t print(Sprite *m, int32_t x, int32_t y);
  uint32_t ewidth();
  uint32_t offset();
private:
  uint32_t o;
};

#endif /* MAIN_LCD_CHARACTER_H_ */
