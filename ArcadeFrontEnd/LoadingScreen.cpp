#include "stdafx.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_syswm.h"
#include <SDL_image.h>


#include "LoadingScreen.h"


using namespace std;
//do this on first start up....or if you cant find the DB
LoadingScreen::LoadingScreen(SDL_ScreenStruct * ss, string msg)
{
	className = "LoadingScreen";
	screenStruct = ss;
	TTF_Font* Sans = TTF_OpenFont(screenStruct->fontPath.c_str(), screenStruct->fontSize);
	White = { 255, 255, 255 };

	messageRect.x = 0;
	messageRect.y = screenStruct->screenH / 2;
	messageRect.w = 100;
	messageRect.h = 100;

	//default message
	SetLoadingMessage(msg);
}

void LoadingScreen::SetLoadingMessage(string msg)
{
	messageSurface = TTF_RenderText_Solid(screenStruct->font, msg.c_str(), White);
	messageTexture = SDL_CreateTextureFromSurface(screenStruct->renderer, messageSurface); //now you can convert it into a texture

	/*string imgPath = CFGHelper::snapsPath + "\\" + "005.png";
	SDL_Rect destRect = { 0,0,0,0 };
	SDL_Surface *temp = IMG_Load(imgPath.c_str());
	SDL_Texture* Img = SDL_CreateTextureFromSurface(screenStruct->renderer, temp);
	destRect.w = temp->w;
	destRect.h = temp->h;*/

	messageRect.w = messageSurface->w;
	messageRect.h = messageSurface->h;
}


void LoadingScreen::UpdateScene()
{

	messageRect.x++;
	if(messageRect.x + messageRect.w > screenStruct->screenW)
		messageRect.x = 0;

		/*SDL_FillRect(SnapImgSurface, NULL, 0x000000);
		SDL_BlitSurface(temp, NULL, SnapImgSurface, NULL);
		SDL_UpdateTexture(SnapTexture, NULL, SnapImgSurface->pixels, SnapImgSurface->pitch);
		SDL_RenderCopy(screenStruct->renderer, SnapTexture, NULL, &destRect);*/
}

void LoadingScreen::DrawScene()
{
	SDL_RenderCopy(screenStruct->renderer, messageTexture, NULL, &messageRect);
	SDL_RenderPresent(screenStruct->renderer);
}