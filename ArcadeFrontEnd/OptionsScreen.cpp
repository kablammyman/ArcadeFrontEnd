#include "stdafx.h"
#include "OptionsScreen.h"
#include "SDL_ttf.h"
#include "DirectInputStuff.h"

OptionsScreen::OptionsScreen(SDL_ScreenStruct *ss)
{
	screenStruct = ss;
	className = "OptionScreen";
	options = new OptionsMenu(10, 10, 25,screenStruct->fontSize*10, RGB{ 255, 255, 255 }, RGB{ 0, 255, 0 });
	options->AddMenuOption("Option 1", 1,1,5);
	options->AddMenuToggleOption("Option 2", true);
	options->AddMenuOption("Option 3", -1);
	options->AddMenuOption("Option 4", 4, 10, 15);
	options->AddMenuToggleOption("Option 5",5);
	
	options->SetInput(&joystick[0].PlayerControls);
	options->SetInputDelay(25);

	//infoImg = new RenderObject(0, 0, 900, 30);
	visibleMenuRect = { 0,0,screenStruct->screenW,screenStruct->screenH };
	renderer.Init(screenStruct->screenW, screenStruct->screenH);
	
	renderer.AddToRenderList(options);
	unsigned int rmask = 0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000, amask = 0xff000000;
	optionsSurface = SDL_CreateRGBSurface(0, visibleMenuRect.w, visibleMenuRect.h, 32, rmask, gmask, bmask, amask);
	optionsTexture = SDL_CreateTextureFromSurface(screenStruct->renderer, optionsSurface);

}
void OptionsScreen::Update()
{
	options->Update();
	if (joystick[0].PlayerControls.CheckButtonFlag(1))
	{
		NotifyObservers();
	}
}
void OptionsScreen::Draw()
{
	for (size_t i = 0; i < options->GetNumMenuItems(); i++)
	{
		ScreenText *screentext = options->GetMemuOptionAt(i);
		RGB color = options->GetMenuItemColorAt(i);
		string text = options->GetMenuItemStringAt(i) + "    "+ options->GetMenuItemValueStringAt(i);

		SDL_Surface *temp = TTF_RenderText_Solid(screenStruct->font, text.c_str(), SDL_Color{ color.r, color.g,color.b });
		SDL_Rect destRect = { screentext->x, screentext->y,temp->w,temp->h };
		
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