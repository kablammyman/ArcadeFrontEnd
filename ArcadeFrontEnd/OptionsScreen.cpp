#include "stdafx.h"
#include "OptionsScreen.h"
#include "SDL_ttf.h"
#include "DirectInputStuff.h"
#include "MainApp.h"

OptionsScreen::OptionsScreen(SDL_ScreenStruct *ss)
{
	screenStruct = ss;
	className = "OptionScreen";
	RGB white,green;
	white.r = 255;
	white.g = 255;
	white.b = 255;
	green.g = 255;

	options = new OptionsMenu(10, 10, 25,screenStruct->fontSize*3,white, green);
	options->AddMenuOption("Idle Timer in mins", 5,1,60);
	options->AddMenuOption("Input delay", 25,10,50);
	options->AddMenuOption("Refresh rom list", -1);
	options->AddMenuOption("Remove crap roms", -1);
	options->AddMenuOption("Exit to main menu", -1);
	options->AddMenuOption("Exit to windows",-1);
	
	options->SetInput(&joystick[0].PlayerControls);
	options->SetInputDelay(MainApp::inputDelay);

	//infoImg = new RenderObject(0, 0, 900, 30);
	visibleMenuRect = { 0,0,screenStruct->screenW,screenStruct->screenH };
	renderController.Init(screenStruct->screenW, screenStruct->screenH);
	
	renderController.AddToRenderList(options);
	unsigned int rmask = 0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000, amask = 0xff000000;
	optionsSurface = SDL_CreateRGBSurface(0, visibleMenuRect.w, visibleMenuRect.h, 32, rmask, gmask, bmask, amask);
	optionsTexture = SDL_CreateTextureFromSurface(screenStruct->renderer, optionsSurface);

}
void OptionsScreen::UpdateScene()
{
	options->Update();
	if (CheckForSelectPress())
	{
		id = options->GetCurrentSelectionItemIndex();
		NotifyObservers();		
	}
}
void OptionsScreen::DrawScene()
{
	for (size_t i = 0; i < options->GetNumMenuItems(); i++)
	{
		ScreenText *screentext = options->GetMemuOptionAt(i);
		RGBA color = options->GetMenuItemColorAt(i);
		string text = options->GetMenuItemStringAt(i);
		
		if(options->GetMenuItemValueAt(i) > -1)
			text += "    "+ options->GetMenuItemValueStringAt(i);

		SDL_Surface *temp = TTF_RenderText_Solid(screenStruct->font, text.c_str(), SDL_Color{ color.r, color.g,color.b });
		SDL_Rect destRect = { screentext->GetPosX(), screentext->GetPosY(),temp->w,temp->h };
		
		SDL_BlitSurface(temp, NULL, optionsSurface, &destRect);
		SDL_FreeSurface(temp);
	}

	/* Clear the entire screen to our selected color. */
	SDL_RenderClear(screenStruct->renderer);

	//transfer the completed surface to the texture
	SDL_UpdateTexture(optionsTexture, NULL, optionsSurface->pixels, optionsSurface->pitch);
	
	SDL_RenderCopy(screenStruct->renderer, optionsTexture, NULL, &visibleMenuRect);
	SDL_RenderPresent(screenStruct->renderer);
}