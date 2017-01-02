// ArcadeFrontEnd.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ArcadeFrontEnd.h"

// Example program:
// Using SDL2 to create an application window

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_syswm.h"
#include <SDL_image.h>

#include "CFGUtils.h"
#include "CFGHelper.h"
#include "FileUtils.h"

#include <stdio.h>
#include <string>


using namespace std;
HANDLE	 hTestProc = NULL;
HANDLE hNewHandle;

SDL_Rect img_rect;
SDL_Window *window;  
//http://cboard.cprogramming.com/windows-programming/111358-fake-keystrokes-another-process.html

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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
			//SDL_MinimizeWindow( window);
			myCreateProcess(CFGHelper::mamePath, "", hTestProc);
			DWORD dwProcessID = GetProcessId(hTestProc);
			HANDLE hProcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);

			
			RegisterWaitForSingleObject(&hNewHandle, hProcHandle, WaitOrTimerCallback, NULL, INFINITE, WT_EXECUTEONLYONCE);
		}
	}
	else if (event->type == SDL_JOYAXISMOTION)  /* Handle Joystick Motion */
	{
		if ((event->jaxis.value < -3200) || (event->jaxis.value > 3200))
		{
			if (event->jaxis.axis == 0)
			{
				/* Left-right movement code goes here */
				img_rect.x += 2;//event->jaxis.value;
				 
			}

			if (event->jaxis.axis == 1)
			{
				/* Up-Down movement code goes here */
				img_rect.y += 2;//event->jaxis.value;
			}

		}
	}
}

int main(int argc, char* argv[]) 
//int APIENTRY WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	CFGHelper::filePathBase = CFGHelper::SetProgramPath(argv[0]);
	CFGHelper::LoadCFGFile();
	//int numRoms = FileUtils::GetNumFilesInDir(CFGHelper::romPath,"zip");
	vector<string> romList = FileUtils::GetAllFileNamesInDir(CFGHelper::romPath, "zip");
	

	SDL_Init(SDL_INIT_VIDEO/* | SDL_INIT_JOYSTICK*/);              // Initialize SDL2
	
	

	/*SDL_Joystick *joystick;
	SDL_JoystickEventState(SDL_ENABLE);
	joystick = SDL_JoystickOpen(0);
	//If there's a problem opening the joystick
	if (joystick == NULL)
	{
		exit(0);
	}
	printf("%i joysticks were found.\n\n", SDL_NumJoysticks());
	printf("The names of the joysticks are:\n");

	//for (int i = 0; i < SDL_NumJoysticks(); i++)
	//	printf("    %s\n", SDL_JoystickName(i));
	*/

	IMG_Init(IMG_INIT_PNG);

	SDL_Surface * bitmapSurface = IMG_Load("D:\\emulators\\mame 0149\\snap\\sf2amf.png");
	string err = SDL_GetError();
	if (bitmapSurface == NULL)
	{
		MessageBox(NULL,L"cant find smile.bmp",L"cant find smile.bmp", MB_OK);
		exit(0);
	}



	TTF_Init();
	string fontPath = CFGHelper::filePathBase + "\\consola.ttf";
	TTF_Font* SansFont = TTF_OpenFont(fontPath.c_str(), 24); //this opens a font style and sets a size
	if (SansFont == NULL)
	{
		MessageBox(NULL, L"cant find Consolas.ttf", L"cant find Consolas.ttf", MB_OK);
		exit(0);
	}

	SDL_Color curColor = { 255, 0, 0 };  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color
	SDL_Surface* surfaceMessage = TTF_RenderText_Solid(SansFont, "put your text here\nand here", curColor); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
	

	SDL_Rect Message_rect; //create a rect
	Message_rect.x = 0;  //controls the rect's x coordinate 
	Message_rect.y = 0; // controls the rect's y coordinte
	Message_rect.w = surfaceMessage->w; // controls the width of the rect
	Message_rect.h = surfaceMessage->h; // controls the height of the rect

	img_rect.x = 0;  //controls the rect's x coordinate 
	img_rect.y = 0; // controls the rect's y coordinte
	img_rect.w = bitmapSurface->w; // controls the width of the rect
	img_rect.h = bitmapSurface->h; // controls the height of the rect
										   // Create an application window with the following settings:
	window = SDL_CreateWindow(
		"An SDL2 window",                  // window title
		SDL_WINDOWPOS_UNDEFINED,           // initial x position
		SDL_WINDOWPOS_UNDEFINED,           // initial y position
		SCREEN_WIDTH,                               // width, in pixels
		SCREEN_HEIGHT,                               // height, in pixels
		SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE
		//SDL_WINDOW_RESIZABLE
		);

	// Check that the window was successfully created
	if (window == NULL) {
		// In the case that the window could not be made...
		printf("Could not create window: %s\n", SDL_GetError());
		return 1;
	}
	


	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);//needs to happen before SDL_GetWindowWMInfo, otherwise it wont work!
	SDL_GetWindowWMInfo(window, &info);
	

	mainWindowHandle = info.info.win.window;
	InitDirectInput(mainWindowHandle);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage); //now you can convert it into a texture
	SDL_Texture *bitmapTex = SDL_CreateTextureFromSurface(renderer, bitmapSurface);
	SDL_Texture *squareTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 100, 300);
	
	//Get window surface
	//SDL_Surface *screenSurface = SDL_GetWindowSurface(window);
	//SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));//Fill the surface white

	SDL_Rect fillRect = { 0, 0, 100, 300 };
		
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
		//SDL_RenderCopy(renderer, bitmapTex, NULL, &img_rect);
		SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
		SDL_RenderDrawRect(renderer, &fillRect);
		//SDL_RenderFillRect(renderer, &fillRect);


		SDL_RenderPresent(renderer);
	}

	SDL_DestroyWindow(window);
	
	//Close the joystick
	//SDL_JoystickClose(joystick);
	IMG_Quit();
	FreeDirectInput();
	// Clean up
	SDL_Quit();
	return 0;
}