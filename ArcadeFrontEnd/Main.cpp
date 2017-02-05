#include "stdafx.h"
#include "resource.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_syswm.h"
#include <SDL_image.h>

#include <stdio.h>
#include <string>

#include "CFGUtils.h"
#include "GameInfo.h"

#include "DirectInputStuff.h"
#include "MainApp.h"

#include "WindowsStuff.h"

int main(int argc, char* argv[])
{
	MainApp app;
	
	CFGHelper::filePathBase = CFGHelper::SetProgramPath(argv[0]);

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();

	//inGameTimer = SDL_AddTimer(DEFAULT_RESOLUTION, ticktock, NULL);//how long have we been playing this game
	//inputTimer = SDL_AddTimer(DEFAULT_RESOLUTION, ticktock, NULL);//when was the last time someone pressed any sort of button
	//totalRuntime = SDL_AddTimer(DEFAULT_RESOLUTION, ticktock, NULL);//how long has the arcade been on

	app.InitWindow();
	app.InitScreens();
	app.InitDB();

	InitDirectInput(mainWindowHandle);

	bool done = false;
	while (!done)
	{
		UpdateInputState();
		app.LogicUpdate();
		app.GraphicsUpdate();
	}
	
	/*SDL_RemoveTimer(inGameTimer);
	SDL_RemoveTimer(inputTimer);
	SDL_RemoveTimer(totalRuntime);*/



	
	IMG_Quit();
	FreeDirectInput();
	SDL_Quit();
	return 0;
}

