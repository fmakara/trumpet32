/*
 * ScreenElement.h
 *
 *  Created on: 17 de jun de 2018
 *      Author: makara
 */

#ifndef MAIN_LCD_SCREENELEMENT_H_
#define MAIN_LCD_SCREENELEMENT_H_

#include "Sprite.h"
#include <stddef.h>

class ScreenElement {
public:
  ScreenElement(Sprite *_sprite=NULL, int _zIndex=0, bool _visible=true, bool _invert=false );
  ~ScreenElement();
  void addToList(ScreenElement *listAddress);
  static ScreenElement* createList(Sprite *dest);
  void renderInto(Sprite *dest);

  void sprite(Sprite *s);
  Sprite* sprite();
  void zIndex(int _z);
  int  zIndex();
  void visible(bool _v);
  bool visible();

  void setPosition(int32_t x, int32_t y, uint_fast8_t _mode=Sprite::COPY_BOTH);
  int32_t getPosX();
  int32_t getPosY();
  uint_fast8_t getMode();

  Sprite *spr;
protected:
  void reposition();

  int32_t z;
  bool v;

  uint_fast8_t mode;
  int32_t posx, posy;

  ScreenElement *last;
  ScreenElement *next;
};

#endif /* MAIN_LCD_SCREENELEMENT_H_ */
