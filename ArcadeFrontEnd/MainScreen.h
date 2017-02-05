#pragma once

#include <string>
#include <vector>


#include "Menu.h"
#include "SnapShot.h"
#include "CFGHelper.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "SQLiteUtils.h"

#include "Scene.h"

#define BUTTON1 1
#define BUTTON2 2
#define OPTIONS_BUTTON 3
using namespace std;


const string tableName = "Arcade";

class MainScreen : public Scene
{
	unsigned int lastButtonPressTime;
	unsigned int buttonPressTimeout;
	unsigned int currentGamePlayTime;
	int numCredits;
	bool isRandomSelection;
	bool menuMode;//are we in the main menu, or playing a game via mame?
	bool isIdle;

	Menu *mainMenu;
	SnapShot *snaps;
	SQLiteUtils *db;
	GameInfo curGameStats;
public:
	MainScreen(SDL_Renderer *r, std::vector<GameInfo> & items, SQLiteUtils *d, int windowW, int windowH, std::string fontPath, int fontSize, SDL_Color textColor, SDL_Color backColor, SDL_Color edge);
	void WriteDBInfo();
	void ResetButtonPressTimer();
	bool IsButtonTimeUp();
	void StartGame(bool startRandom = false);
	void ReturnFromGame();
	static SDL_Window *window;
	static SDL_Renderer *renderer;

	void Update();
	void Draw();
};





