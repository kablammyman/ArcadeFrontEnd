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

#define TICK_INTERVAL    30
static Uint32 next_time;

Uint32 time_left(void)
{
	Uint32 now;

	now = SDL_GetTicks();
	if (next_time <= now)
		return 0;
	else
		return next_time - now;
}

int main(int argc, char* argv[])
{
	MainApp app;
	//Event handler
	SDL_Event e;
	CFGHelper::filePathBase = CFGHelper::SetProgramPath(argv[0]);

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();

	//inGameTimer = SDL_AddTimer(DEFAULT_RESOLUTION, ticktock, NULL);//how long have we been playing this game
	//inputTimer = SDL_AddTimer(DEFAULT_RESOLUTION, ticktock, NULL);//when was the last time someone pressed any sort of button
	//totalRuntime = SDL_AddTimer(DEFAULT_RESOLUTION, ticktock, NULL);//how long has the arcade been on

	app.InitWindow(false);
	app.InitScreens();
	app.InitDB();

	InitDirectInput(mainWindowHandle);
	CFGHelper::LoadFrontEndControls(&feControls);

	next_time = SDL_GetTicks() + TICK_INTERVAL;
	bool done = false;
	while (!done)
	{
		UpdateInputState();
		app.LogicUpdate();
		
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				done = true;
			}
			if (e.type == SDL_WINDOWEVENT) {
				switch (e.window.event) {
				case SDL_WINDOWEVENT_SHOWN:
					SDL_Log("Window %d shown", e.window.windowID);
					break;
				case SDL_WINDOWEVENT_HIDDEN:
					SDL_Log("Window %d hidden", e.window.windowID);
					break;
				case SDL_WINDOWEVENT_EXPOSED:
					SDL_Log("Window %d exposed", e.window.windowID);
					break;
				case SDL_WINDOWEVENT_MOVED:
					SDL_Log("Window %d moved to %d,%d",
						e.window.windowID, e.window.data1,
						e.window.data2);
					break;
				case SDL_WINDOWEVENT_RESIZED:
					SDL_Log("Window %d resized to %dx%d",
						e.window.windowID, e.window.data1,
						e.window.data2);
					break;
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					SDL_Log("Window %d size changed to %dx%d",
						e.window.windowID, e.window.data1,
						e.window.data2);
					break;
				case SDL_WINDOWEVENT_MINIMIZED:
					SDL_Log("Window %d minimized", e.window.windowID);
					break;
				case SDL_WINDOWEVENT_MAXIMIZED:
					SDL_Log("Window %d maximized", e.window.windowID);
					break;
				case SDL_WINDOWEVENT_RESTORED:
					SDL_Log("Window %d restored", e.window.windowID);
					break;
				case SDL_WINDOWEVENT_ENTER:
					SDL_Log("Mouse entered window %d",
						e.window.windowID);
					break;
				case SDL_WINDOWEVENT_LEAVE:
					SDL_Log("Mouse left window %d", e.window.windowID);
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					SDL_Log("Window %d gained keyboard focus",
						e.window.windowID);
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					SDL_Log("Window %d lost keyboard focus",
						e.window.windowID);
					break;
				case SDL_WINDOWEVENT_CLOSE:
					SDL_Log("Window %d closed", e.window.windowID);
					break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
				case SDL_WINDOWEVENT_TAKE_FOCUS:
					SDL_Log("Window %d is offered a focus", e.window.windowID);
					break;
				case SDL_WINDOWEVENT_HIT_TEST:
					SDL_Log("Window %d has a special hit test", e.window.windowID);
					break;
#endif
				default:
					SDL_Log("Window %d got unknown event %d",
						e.window.windowID, e.window.event);
					break;
				}
			}
		}

		SDL_Delay(time_left());
		next_time += TICK_INTERVAL;

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

