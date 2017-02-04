#pragma once

#include <string>
#include "Menu.h"

class SnapShot
{
	SDL_Surface *SnapImgSurface;
	SDL_Rect SnapImgRect;
	SDL_Texture *SnapTexture;
	std::string curSnap;
	SDL_Renderer *renderer;
	TTF_Font* titleFont;
	SDL_Color titleColor;

	Menu *menu;
public:
	SnapShot();
	SnapShot(SDL_Renderer *r,Menu *m,int windowX, int windowY, int windowW, int windowH);
	~SnapShot();
	void LoadCurrentSnapshot();
	void Draw();
	static std::string GetReadableTimeFromMilis(unsigned int milis);
};