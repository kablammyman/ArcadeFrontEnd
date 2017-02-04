#pragma once

#include "SDL_ttf.h"
#include "SDL.h"
#include <string>
//a way to bridge the gap between my code and sdl stuff

struct SDL_ScreenStruct
{
	SDL_Renderer *renderer;
	TTF_Font* font;
	std::string fontPath;
	int fontSize;
	int screenW,screenH;
	SDL_Rect* rect;
	SDL_Texture* texture;
	SDL_Surface *surface;
};