#pragma once

#include "MainScreen.h"
#include "OptionsScreen.h"
#include "LoadingScreen.h"
#include "SDL_ScreenStruct.h"
#include "AdminWork.h"

#include "Observer.h"

class MainApp : public Observer
{
public:
	//Screen dimension constants
	int SCREEN_WIDTH;
	int SCREEN_HEIGHT;

	vector<Scene*> allScenes;
	Scene *curScreen;
	size_t sceneIndex;

	MainScreen *mainMenu;
	OptionsScreen *options;
	LoadingScreen *loading;
	

	SDL_Window *window;
	SDL_Renderer *renderer;
	
	SQLiteUtils db;
	vector<GameInfo> AllGameListInfo;
	AdminWork *admin;
	
	SDL_ScreenStruct screenStruct;

	bool populateDB;
	string dbOut;
	int largeFontSize;
	int fontSize;
	static int inputDelay;
	MainApp();
	~MainApp();
	//part of observer
	void Notify(Observee* observee);

	void AddMainScreen();
	void InitDB();
	void InitWindow(bool fullScreen = true);
	void InitScreens();
	void LogicUpdate();
	void GraphicsUpdate();
	
};