/*
 * MainMenu.h
 *
 *  Created on: 30 de jul de 2018
 *      Author: makara
 */

#ifndef MAIN_UI_MAINMENU_H_
#define MAIN_UI_MAINMENU_H_

#include "Dictionary.h"
#include "../config/ConfigManager.h"
#include "../lcd/ScreenElement.h"
#include <vector>
#include <map>

class MainMenu {
public:
  static MainMenu* get();
  void run(ScreenElement *scr);
protected:
  MainMenu();
  static const int MainMenuFontNormal = 8;
  static MainMenu *singleton;
  enum MenuElementType {
    ME_MENU,
    ME_INTEGER_C,
    ME_INTEGER,
    ME_STRING_C,
    ME_STRING,
    ME_IP_C,
    ME_IP_,
    ME_ENUM_C,
    ME_ENUM,
  };
  struct MenuElement {
    MenuElementType type;
    DIC::DICTindex name;
    CM::Config cfg;
    MenuElement *father;
    std::vector<MenuElement*> *childs;
    std::map<int,DIC::DICTindex> *enumerate;
    int menu_index;
    int menu_offset;
    MenuElement(
        MenuElementType _type,
        DIC::DICTindex _name,
        CM::Config _cfg,
        MenuElement *_father,
        std::map<int,DIC::DICTindex> *_enum=NULL){
      type = _type;
      name =_name;
      cfg = _cfg;
      father = _father;
      childs = NULL;
      enumerate = _enum;
      menu_index = 0;
      menu_offset = 0;
    }
  };
  MenuElement *root;
};

#endif /* MAIN_UI_MAINMENU_H_ */
