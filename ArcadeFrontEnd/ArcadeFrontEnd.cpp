// ArcadeFrontEnd.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ArcadeFrontEnd.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_syswm.h"
#include <SDL_image.h>

#include <stdio.h>
#include <string>

#include "CFGUtils.h"



using namespace std;


//http://cboard.cprogramming.com/windows-programming/111358-fake-keystrokes-another-process.html




void handle_keys(SDL_Event* event, bool *quit)
{
	if (event->type == SDL_KEYDOWN)
	{
		if (event->key.keysym.sym == SDLK_ESCAPE)
		{
			*quit = true;
		}
	}

	else if (event->type == SDL_KEYUP)
	{
		if (event->key.keysym.sym == SDLK_e)
		{
			string gameName = mainMenu->GetCurrentSelectedItem();
			LaunchGame(gameName);
			//printf("test");
			//mainMenu->SkipToLetter('7');
			
		}

		else if (event->key.keysym.sym == SDLK_DOWN)
		{
			mainMenu->Next();
			LoadCurrentSnapshot();
		}
		else if (event->key.keysym.sym == SDLK_UP)
		{
			mainMenu->Prev();
			LoadCurrentSnapshot();
		}
	}
}


int main(int argc, char* argv[]) 
{
	CFGHelper::filePathBase = CFGHelper::SetProgramPath(argv[0]);
	CFGHelper::LoadCFGFile();
	
	//vector<string> romList = FileUtils::GetAllFileNamesInDir(CFGHelper::romPath, "zip");

	

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();

	inGameTimer = SDL_AddTimer(DEFAULT_RESOLUTION, ticktock, NULL);//how long have we been playing this game
	inputTimer = SDL_AddTimer(DEFAULT_RESOLUTION, ticktock, NULL);//when was the last time someone pressed any sort of button
	totalRuntime = SDL_AddTimer(DEFAULT_RESOLUTION, ticktock, NULL);//how long has the arcade been on

	int fontSize = 8;
	string menuFontPath = CFGHelper::filePathBase + "\\consola.ttf";
	
										   // Create an application window with the following settings:
	window = SDL_CreateWindow(
		"Arcade Frontend",                  // window title
		SDL_WINDOWPOS_UNDEFINED,           // initial x position
		SDL_WINDOWPOS_UNDEFINED,           // initial y position
		SCREEN_WIDTH,                               // width, in pixels
		SCREEN_HEIGHT,                               // height, in pixels
		//SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE
		SDL_WINDOW_RESIZABLE
		);
	// Check that the window was successfully created
	if (window == NULL) {
		// In the case that the window could not be made...
		printf("Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	//get current screen res...if this app is a window, it wont get window size!
	unsigned int windowCheck = SDL_GetWindowFlags(window);
	if (windowCheck & SDL_WINDOW_FULLSCREEN)
	{
		SDL_DisplayMode DM;
		SDL_GetCurrentDisplayMode(0, &DM);
		SCREEN_WIDTH = DM.w;
		SCREEN_HEIGHT = DM.h;
		fontSize = 24;
	}

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);//needs to happen before SDL_GetWindowWMInfo, otherwise it wont work!
	SDL_GetWindowWMInfo(window, &info);

	mainWindowHandle = info.info.win.window;
	InitDirectInput(mainWindowHandle);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	unsigned int rmask = 0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000, amask = 0xff000000;
	SnapImgSurface = SDL_CreateRGBSurface(0, 500, 500, 32, rmask, gmask, bmask, amask);

	//this pixel format is STRANGE! but its the one that works with the mame snaps...
	SnapTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 500, 500);

	if (!FileUtils::DoesPathExist(CFGHelper::filePathBase + "\\gamelist.txt"))
	{
		TTF_Font* Sans = TTF_OpenFont(menuFontPath.c_str(), 36); 
		SDL_Color White = { 255, 255, 255 }; 
		SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, "Cant find your game list, Generating it now...please wait", White);
		SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage); //now you can convert it into a texture

		SDL_Rect Message_rect; //create a rect
		Message_rect.x = 0; 
		Message_rect.y = SCREEN_HEIGHT/2;
		Message_rect.w = surfaceMessage->w; 
		Message_rect.h = surfaceMessage->h;
		SDL_RenderCopy(renderer, Message, NULL, &Message_rect); 
		SDL_RenderPresent(renderer);
		GenerateGameList(CFGHelper::mameList);
		SDL_FreeSurface(surfaceMessage);
		SDL_DestroyTexture(Message);
	}
	else
	{
		TTF_Font* Sans = TTF_OpenFont(menuFontPath.c_str(), 36);
		SDL_Color White = { 255, 255, 255 };
		SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, "Loading...", White);
		SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage); 
		 

		/*string imgPath = CFGHelper::snapsPath + "\\" + "005.png";
		SDL_Rect destRect = { 0,0,0,0 };
		SDL_Surface *temp = IMG_Load(imgPath.c_str());
		SDL_Texture* Img = SDL_CreateTextureFromSurface(renderer, temp);
		destRect.w = temp->w;
		destRect.h = temp->h;

		SDL_FillRect(SnapImgSurface, NULL, 0x000000);
		SDL_BlitSurface(temp, NULL, SnapImgSurface, NULL);
		SDL_UpdateTexture(SnapTexture, NULL, SnapImgSurface->pixels, SnapImgSurface->pitch);
		SDL_RenderCopy(renderer, SnapTexture, NULL, &destRect);*/

		//SDL_RenderCopy(renderer, Img, NULL, &destRect);
		
		
		SDL_Rect Message_rect; //create a rect
		Message_rect.x = SCREEN_WIDTH /2 - surfaceMessage->w;
		Message_rect.y = SCREEN_HEIGHT / 2;
		Message_rect.w = surfaceMessage->w;
		Message_rect.h = surfaceMessage->h;
		SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
		

		SDL_RenderPresent(renderer);
		FillGameListFromCSV();

		SDL_FreeSurface(surfaceMessage);
		SDL_DestroyTexture(Message);
	}

	
	mainMenu = new Menu(renderer, AllGameListInfo, SCREEN_WIDTH, SCREEN_HEIGHT, menuFontPath, fontSize, SDL_Color{ 0,0,255 }, SDL_Color{ 255,255,255 }, SDL_Color{ 0,0,255 });
	
	SnapImgRect.x = mainMenu->GetMenuWidth() + 50;
	SnapImgRect.y = (SCREEN_HEIGHT / 2) - (250);
	SnapImgRect.w = 500;
	SnapImgRect.h = 500;

	LoadCurrentSnapshot();
	
	SDL_Event event;
	bool done = false;
	while (!done)
	{
		UpdateInputState();
		// Poll for events 
		while (SDL_PollEvent(&event)) 
		{
			handle_keys(&event, &done);
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, SnapTexture, NULL, &SnapImgRect);
		mainMenu->Draw();

		
		SDL_RenderPresent(renderer);
	}
	
	SDL_RemoveTimer(inGameTimer);
	SDL_RemoveTimer(inputTimer);
	SDL_RemoveTimer(totalRuntime);

	SDL_DestroyTexture(SnapTexture);

	SDL_DestroyWindow(window);
	IMG_Quit();
	FreeDirectInput();
	SDL_Quit();
	return 0;
}