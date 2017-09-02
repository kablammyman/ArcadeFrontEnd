#pragma once

#include <string>
#include "Menu.h"

//#include "ScreenText.h"
#include "BitmapFont.h"

class SnapShot
{
	SDL_Surface *SnapImgSurface;
	SDL_Rect SnapImgRect;
	SDL_Texture *SnapTexture;

	SDL_Renderer *renderer;
	TTF_Font* titleFont;
	SDL_Color titleColor;

	Menu *menu;


	SDL_Texture *giTexture;
	SDL_Rect giImgRect;

	GameInfo curGamneInfo;
	//temp?? I prob should use the ttf fonts since they are much bigger
	//TTF_Font* titleFont;
	//SDL_Color titleColor;
	//SDL_Surface *giImgSurface;

	StockBitmapFont font;
	PIXMAP *buffer;

public:
	SnapShot();
	SnapShot(SDL_Renderer *r,Menu *m,int windowX, int windowY, int windowW, int windowH);
	~SnapShot();
	void LoadCurrentSnapshot();
	void Draw();
	static std::string GetReadableTimeFromMilis(unsigned int milis);
};