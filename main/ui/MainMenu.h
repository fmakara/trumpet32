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
		ME_INTEGER,
		ME_STRING,
	};
	struct MenuElement {
		MenuElementType type;
		DIC::DICTindex name;
		CM::Config cfg;
		bool editable;
		MenuElement *father;
		std::vector<MenuElement*> *childs;
		std::map<int,MenuElement*> *enumerate;
		int menu_index;
		int menu_offset;
		MenuElement(
				MenuElementType _type,
				DIC::DICTindex _name,
				CM::Config _cfg,
				bool _editable,
				MenuElement *_father){
			type = _type;
			name =_name;
			cfg = _cfg;
			editable = _editable;
			father = _father;
			childs = NULL;
			enumerate = NULL;
			menu_index = 0;
			menu_offset = 0;
		}
	};
	MenuElement *root;
};

#endif /* MAIN_UI_MAINMENU_H_ */
