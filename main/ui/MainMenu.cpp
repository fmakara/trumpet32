/*
 * MainMenu.cpp
 *
 *  Created on: 30 de jul de 2018
 *      Author: makara
 */

#include "MainMenu.h"
#include <stdio.h>
#include "../lcd/Charset.h"
#include "../lcd/Writer.h"
#include "../lcd/Lcd.h"
#include "PopUpper.h"

MainMenu* MainMenu::singleton = NULL;

MainMenu* MainMenu::get(){
	if(singleton==NULL)singleton = new MainMenu();
	return singleton;
}

MainMenu::MainMenu() {
	root = new MenuElement(ME_MENU,DIC::MENU_MAINMENU_CAP,CM::INVALID,false,NULL);
	root->childs = new std::vector<MenuElement*>();
	root->childs->push_back(new MenuElement(ME_INTEGER,DIC::PLAYR_B1_FREQ_CAP,CM::PLAYR_B1_FREQ,true,root));
	root->childs->push_back(new MenuElement(ME_INTEGER,DIC::PLAYR_B1_TIME_CAP,CM::PLAYR_B1_TIME,true,root));
	root->childs->push_back(new MenuElement(ME_INTEGER,DIC::PLAYR_B2_FREQ_CAP,CM::PLAYR_B2_FREQ,true,root));
	root->childs->push_back(new MenuElement(ME_INTEGER,DIC::PLAYR_B2_TIME_CAP,CM::PLAYR_B2_TIME,true,root));
}

void MainMenu::run(ScreenElement *scr){
	MenuElement *shown = root;
	Character *NS_font = Charset::get()->asArray(MainMenuFontNormal);
	Writer NS_writer(NS_font,NULL,NULL);
	uint32_t NS_fontheight = NS_font[0].height();
	Sprite *s = scr->sprite();
	uint8_t lastInput = 0;
	Lcd *lcd = Lcd::get();
	//only for menus
	int menu_scrollcount=0;
	int menu_nlines = ((s->height()-3)/NS_font[0].height())-1;
	Sprite *menuSpriteTop = new Sprite(s,0,0,s->width(),NS_fontheight);
	std::vector<Sprite*> menuSpriteList;
	for(int i=1;i<=menu_nlines;i++)
		menuSpriteList.push_back(new Sprite(s,0,3+i*NS_fontheight,s->width()-3,NS_fontheight));

	scr->visible(true);
	while(true){
		//render the screen
		if(shown->type==ME_MENU){
			//Find the number of lines available to show
			//housekeeping for the selected index and screen offset
			if(shown->menu_index<0)shown->menu_index=0;
			if(shown->menu_index>=shown->childs->size())shown->menu_index=shown->childs->size()-1;

			if(shown->menu_offset>=(shown->childs->size()-menu_nlines))
				shown->menu_offset=(shown->childs->size()-menu_nlines);
			if(shown->menu_offset<0)shown->menu_offset=0;

			//The offset shall always follow the index within the available window
			if(shown->menu_index>=shown->menu_offset+menu_nlines)
				shown->menu_offset = shown->menu_index-menu_nlines+1;
			if(shown->menu_index<shown->menu_offset)
				shown->menu_offset = shown->menu_index;

			//Begin the drawing
			s->clear();
			NS_writer.scroll_y=0;
			NS_writer.canvas = menuSpriteTop;
			NS_writer.renderCentered((char*)DIC::global->get(shown->name));

			for(int i=0;i<menu_nlines;i++){
				NS_writer.scroll_x=0;
				NS_writer.canvas = menuSpriteList[i];
				NS_writer.text = (char*)DIC::global->get(shown->childs->at(i+shown->menu_offset)->name);
				if(i+shown->menu_offset == shown->menu_index){
					int txtwidth;
					NS_writer.max_scroll(&txtwidth,NULL);
					if(menu_scrollcount<8){
						//offset already at 0
					}else if((menu_scrollcount-8)*5<+txtwidth){
						NS_writer.scroll_x = (menu_scrollcount-8)*5;
					}else if((menu_scrollcount-16)*5<+txtwidth){
						NS_writer.scroll_x = txtwidth;
					}else{
						menu_scrollcount = 0;
					}
					NS_writer.render();
					menuSpriteList[i]->invert(0,0,999,999);
				}else{
					NS_writer.render();
				}
			}

			// Horizontal tab and scroll bar
			s->line(0,NS_fontheight+1,s->width(),NS_fontheight+1);
			if(shown->childs->size()>menu_nlines){
				int perline = (s->height()-NS_fontheight-3)/shown->childs->size();
				s->line(s->width()-1,NS_fontheight+3+perline*shown->menu_offset,
						s->width()-1,NS_fontheight+3+perline*(menu_nlines+shown->menu_offset));
			}

		}else{
			PopUpper::get()->popup("Not implemented / error",3000,0);
			break;
		}
		//check for inputs
		if(shown->type==ME_MENU){
			//Not only if there is a press, but if it was already pressed and not computed
			if(lastInput){
				lcd->buttons_waitRelease(lastInput,1000);
				lastInput = 0;
			}else{
				lastInput = lcd->buttons_get(Lcd::BUTTONS_USED);
			}
			if(!lastInput)lastInput = lcd->buttons_waitPress(Lcd::BUTTONS_USED,100);

			if(lastInput&(Lcd::BUTTON_BACK|Lcd::BUTTON_LEFT)){
				//Go back, when possible
				if(shown->father==NULL)break;
				shown = shown->father;
			}else if(lastInput&(Lcd::BUTTON_OK|Lcd::BUTTON_RIGHT)){
				//Enter into child
				shown = shown->childs->at(shown->menu_index);
			}else if(lastInput & Lcd::BUTTON_UP){
				shown->menu_index--;
			}else if(lastInput & Lcd::BUTTON_DOWN){
				shown->menu_index++;
			}
			if(lastInput){
				menu_scrollcount=0;
			}else{
				menu_scrollcount++;
			}
		}
	}
	//cleanup
	scr->visible(false);
	delete menuSpriteTop;
	while(!menuSpriteList.empty()) {
		delete menuSpriteList.back();
		menuSpriteList.pop_back();
	}
}
