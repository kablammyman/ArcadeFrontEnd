#include "stdafx.h"
#include "Menu.h"
#include <algorithm>    // std::binary_search, std::sort
#include <time.h>
/*#if SDL_BYTEORDER == SDL_BIG_ENDIAN
rmask = 0xff000000;
gmask = 0x00ff0000;
bmask = 0x0000ff00;
amask = 0x000000ff;
#else
rmask = 0x000000ff;
gmask = 0x0000ff00;
bmask = 0x00ff0000;
amask = 0xff000000;
#endif
*/
using namespace std;

Menu::Menu(SDL_Renderer *r, std::vector<GameInfo> & items,int windowW, int windowH, string fontPath, int fontSize, SDL_Color textColor, SDL_Color backColor, SDL_Color edge )
{
	srand((unsigned int)time(0));
	unsigned int rmask = 0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000, amask = 0xff000000;
	selectionDelay = 100; //selector can move only once per selectionDelay many ms
	menuItems = items;
	//make sure list is in alpha betical order
	std::sort(items.begin(), items.end(), GameInfo::sortFn);

	curLetter = menuItems[0].name[0];

	renderer = r;
	curSlectedItem = 0;
	curMenuListIndex = 0;
	padding = 10;
	menuTextColor = textColor;
	backgroundColor = backColor;
	backEdgeColor = edge;

	menuFont = TTF_OpenFont(fontPath.c_str(), fontSize); 
	if (menuFont == NULL)
	{
		wstring error = L"cant find font" + wstring(fontPath.begin(),fontPath.end());
		MessageBox(NULL, error.c_str(), L"cant find font", MB_OK);
		exit(0);
	}
	menuFontHeight = TTF_FontLineSkip(menuFont);

	//calc menu dims
	menuWidth = (int)float(windowW / 3);
	menuHeight = windowH - (int)(menuFontHeight * 2);
	numItemsToDisplay = (int)float(menuHeight / menuFontHeight) - 1;

	if(numItemsToDisplay > (int)menuItems.size())
		numItemsToDisplay = (int)menuItems.size();

	//SDL_QueryTexture(visibleMenuItemsTexture, NULL, NULL, &menuWidth, &menuHeight);
	
	visibleMenuRect = { 0,padding,menuWidth- padding,menuHeight- padding };
	outterRect = { 0,0,menuWidth,menuHeight };
	selector = {0,padding,menuWidth,menuFontHeight +1};

	menuSurface = SDL_CreateRGBSurface(0, visibleMenuRect.w, visibleMenuRect.h, 32, rmask, gmask, bmask, amask);
	visibleMenuItemsTexture = SDL_CreateTextureFromSurface(renderer, menuSurface);
	PositionSelector();
	FillVisibleMenu();
}
//---------------------------------------------------------------------------------------
Menu::~Menu()
{
	SDL_DestroyTexture(visibleMenuItemsTexture);
	SDL_FreeSurface(menuSurface);
}
//---------------------------------------------------------------------------------------
void Menu::FillVisibleMenu()
{
	int curYPos = 0;
	SDL_Rect destRect = { 10,curYPos,0,0 };

	//clear the dest surface
	SDL_FillRect(menuSurface, NULL, SDL_MapRGB(menuSurface->format, backgroundColor.r, backgroundColor.g, backgroundColor.b));
	size_t numTitles = curMenuListIndex + numItemsToDisplay;
	
	if(numTitles > menuItems.size())
		numTitles = menuItems.size();

	//print the next X items inthe list onto the surface
	for (size_t i = curMenuListIndex; i < numTitles; i++)
	{
		SDL_Surface *temp = TTF_RenderText_Solid(menuFont, menuItems[i].name.c_str(), menuTextColor);
		destRect.y = curYPos;
		destRect.w = temp->w;
		destRect.h = temp->h;

		SDL_BlitSurface(temp, NULL, menuSurface, &destRect);
		curYPos += menuFontHeight;
		SDL_FreeSurface(temp);
	}
	//transfer the completed surface to the texture
	SDL_UpdateTexture(visibleMenuItemsTexture, NULL, menuSurface->pixels, menuSurface->pitch);
}
//---------------------------------------------------------------------------------------
void Menu::PositionSelector()
{
	selector.y = (curSlectedItem *menuFontHeight) + 1 + padding;
}
//---------------------------------------------------------------------------------------
void Menu::Next(unsigned delay)
{
	
	if(delay - curTime < selectionDelay)
		return;
	
	curTime = delay;

	if(curSlectedItem < numItemsToDisplay-1)
		curSlectedItem++;
	else
	{
		int listEnd = (int)float(menuItems.size() - numItemsToDisplay)-1;
		if (curMenuListIndex < listEnd)
		{
			curMenuListIndex++;
			FillVisibleMenu();
		}
		curSlectedItem = numItemsToDisplay - 1;
	}
	PositionSelector();
}
//---------------------------------------------------------------------------------------
void Menu::Prev(unsigned delay)
{
	if (delay - curTime < selectionDelay)
		return;

	curTime = delay;

	if (curSlectedItem > 0)
		curSlectedItem--;
	else
	{
		if (curMenuListIndex > 0)
		{
			curMenuListIndex--;
			FillVisibleMenu();
		}
		curSlectedItem = 0;
	}
	PositionSelector();
}
//---------------------------------------------------------------------------------------
void Menu::SelectRandomGame()
{
	int max = (int)menuItems.size()-1;
	if (max - 0 == 1)
		curSlectedItem = (1 + rand() % 10) > 5 ? 1 : 0;

	else
		curSlectedItem = (((max + 1) * rand()) / RAND_MAX);
	

	int listEnd = (int)float(max / numItemsToDisplay) - 1;
	if (curMenuListIndex < listEnd)
	{
		curMenuListIndex++;
		FillVisibleMenu();
	}
	
	PositionSelector();
}
//---------------------------------------------------------------------------------------
void Menu::SkipToLetter(char letter)
{
	if (!isdigit(letter) && !isalpha(letter))
		return;

	for (size_t i = 0; i < menuItems.size(); i++)
	{
		if (menuItems[i].name[0] == letter)
		{
			curSlectedItem = 0;
			curMenuListIndex = (int)i;
			FillVisibleMenu();
			PositionSelector();
			curLetter = letter;
			return;
		}
	}
	//if we get here, there are no roms starting with the letter specified
}
//---------------------------------------------------------------------------------------
std::string Menu::GetCurrentSelectedItemRomName()
{
	size_t index = curSlectedItem + curMenuListIndex;
	
	if(index > menuItems.size()-1)
		index = menuItems.size()-1;

	return menuItems[index].romName;
}
//---------------------------------------------------------------------------------------
GameInfo Menu::GetCurrentSelectedItem()
{
	size_t index = curSlectedItem + curMenuListIndex;

	if (index > menuItems.size() - 1)
		index = menuItems.size() - 1;

	return menuItems[index];
}
//---------------------------------------------------------------------------------------
void Menu::Draw()
{
	
	SDL_SetRenderDrawColor(renderer, backEdgeColor.r, backEdgeColor.g, backEdgeColor.b, 255);
	SDL_RenderFillRect(renderer, &outterRect);
	SDL_RenderCopy(renderer, visibleMenuItemsTexture, NULL, &visibleMenuRect);
	SDL_RenderDrawRect(renderer, &selector);
}