#pragma once

#include <string>
#include <vector>

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_syswm.h"
#include "SDL_image.h"
#include "GameInfo.h"


class Menu
{
	SDL_Renderer *renderer;
	SDL_Color menuTextColor;
	SDL_Color backgroundColor;
	SDL_Color backEdgeColor;
	SDL_Texture* visibleMenuItemsTexture;
	SDL_Surface *menuSurface;
	SDL_Rect visibleMenuRect;
	SDL_Rect outterRect;
	SDL_Rect selector;
	TTF_Font* menuFont;
	char curLetter;

	std::vector<GameInfo> menuItems;
	unsigned int curTime;

	int curSlectedItem;
	int numItemsToDisplay;
	int numVisibleItems;
	int menuFontHeight;
	int textVertSpacing;//may not be needed afterall
	int menuWidth;
	int menuHeight;
	int curMenuListIndex;
	int padding;
	void PositionSelector();
	void FillVisibleMenu();

public:
	Menu(SDL_Renderer *r, std::vector<GameInfo> & items,int windowW, int windowH, std::string fontPath, int fontSize, SDL_Color textColor, SDL_Color backColor, SDL_Color edge);
	~Menu();
	
	GameInfo GetCurrentSelectedItem();
	std::string Menu::GetCurrentSelectedItemRomName();
	void Draw();
	void Next(unsigned delay = 0);
	void Prev(unsigned delay = 0);
	void SkipToLetter(char letter);
	void SkipToNextLetter(bool skipUp);
	void SelectRandomGame();
	int GetMenuWidth() { return menuWidth; }
	int GetMenuHeight() { return menuHeight; }
};