#pragma once
#include <string>
#include <vector>

#include "SDL_ttf.h"
#include "SDL.h"

#include "Scene.h"
#include "SDL_ScreenStruct.h"




//even tho this is called loading screen, this is for any work is done to worth with the db
class LoadingScreen : public Scene
{
	SDL_ScreenStruct *screenStruct;
	SDL_Color White;
	SDL_Rect messageRect;
	SDL_Surface* messageSurface;
	SDL_Texture* messageTexture;

public:
	LoadingScreen(SDL_ScreenStruct * ss, string msg);
	~LoadingScreen()
	{
		if (messageSurface)
			SDL_FreeSurface(messageSurface);
		
		if(messageTexture)
			SDL_DestroyTexture(messageTexture);
	}
	
	void SetLoadingMessage(string msg);
	void DrawScene();
	void UpdateScene();

};