#pragma once
#include "Scene.h"
#include "OptionMenu.h"
#include "SDL_ScreenStruct.h"


class OptionsScreen : public Scene
{
	
	OptionsMenu *options;
	RenderObject *infoImg;

	SDL_Surface* optionsSurface;
	SDL_Texture* optionsTexture;

	SDL_ScreenStruct *screenStruct;
	SDL_Rect visibleMenuRect;
	

public:
	OptionsScreen(SDL_ScreenStruct *ss);
	void UpdateScene();
	void DrawScene();
};