// ArcadeFrontEnd.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainScreen.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_syswm.h"
#include <SDL_image.h>

#include <stdio.h>

#include "Input.h"
#include "CFGUtils.h"

#include "WindowsStuff.h"
#include "DirectInputStuff.h"

using namespace std;

/*#define DEFAULT_RESOLUTION  1
static int ticks = 0;
static Uint32 SDLCALL ticktock(Uint32 interval, void *param)
{
	++ticks;
	return (interval);
}
static Uint32 SDLCALL callback(Uint32 interval, void *param)
{
SDL_Log("Timer %d : param = %d", interval, (int)(uintptr_t)param);
return interval;
}
SDL_TimerID inGameTimer, inputTimer, totalRuntime;*/



SDL_Window *MainScreen::window;
SDL_Renderer *MainScreen::renderer;

MainScreen::MainScreen(SDL_Renderer *r, std::vector<GameInfo> & items, SQLiteUtils *d, int windowW, int windowH, std::string fontPath, int fontSize, SDL_Color textColor, SDL_Color backColor, SDL_Color edge)
{
	className = "MainScreen";
	mainMenu = new Menu(r, items, windowW, windowH, fontPath, fontSize, textColor, backColor, edge);
	snaps = new SnapShot(r, mainMenu, mainMenu->GetMenuWidth() + 2, 10, windowW, windowH);
	snaps->LoadCurrentSnapshot();
	
	ResetButtonPressTimer();
	menuMode = true;
	isIdle = false;
	buttonPressTimeout = 300000;//default is 5 minutes
	db = d;
}

void MainScreen::ResetButtonPressTimer()
{
	lastButtonPressTime = SDL_GetTicks();
	isIdle = false;
}


bool MainScreen::IsButtonTimeUp()
{
	if (SDL_GetTicks() - lastButtonPressTime > buttonPressTimeout)
	{
		isIdle = true;
		return true;
	}
	return false;
}




void MainScreen::StartGame(bool startRandom)
{
	ResetButtonPressTimer();
	ClearMainJoystickInputFlags();
	menuMode = false;
	if (startRandom)
		mainMenu->SelectRandomGame();

	string romName = mainMenu->GetCurrentSelectedItemRomName();
	currentGamePlayTime = 0;
	numCredits = 0;
	isRandomSelection = startRandom;
	//SDL_MinimizeWindow( window);
	menuMode = false;
	
	//so we can know when the game ends
	mainscreenPointer = this;
	LaunchGame(romName, FileUtils::GetFileNameFromPathString(CFGHelper::mamePath));
	currentGamePlayTime = SDL_GetTicks();
}
void MainScreen::UpdateScene()
{


	if (IsButtonTimeUp() && !menuMode)
	{
		SendMessage(mameWindowHandle, WM_CLOSE, 0, 0);
	}

	if (menuMode)
	{
		if (CheckForAnyMenuPress())
			ResetButtonPressTimer();

		if (CheckForMenuDown())
		{
			mainMenu->Next(SDL_GetTicks());
			snaps->LoadCurrentSnapshot();
		}
		else if (CheckForMenuUp())
		{
			mainMenu->Prev(SDL_GetTicks());
			snaps->LoadCurrentSnapshot();
		}

		if (CheckForSelectPress())
		{
			StartGame();
		}
		else if (CheckForSkipPress())
		{
			bool skipUp = false;
			if (CheckForMenuUp())
				skipUp = true;

			mainMenu->SkipToNextLetter(skipUp);
			snaps->LoadCurrentSnapshot();
		}
		else if (CheckForOptionsPress())
		{
			id = 0;
			NotifyObservers();
		}
		

		//select a new random game
		if (isIdle)
		{
			StartGame(true);
		}
	}
	//if in game, check all inputs to make sure the game is not set to idle
	else
	{
		if (CheckP1Coin())
		{
			numCredits++;
		}
		if (CheckP2Coin())
		{
			numCredits++;
		}
		if (CheckP3Coin())
		{
			numCredits++;
		}
		if (CheckP4Coin())
		{
			numCredits++;
		}

		for (int i = 0; i < enumCurJoystickIndex; i++)
			if (joystick[i].PlayerControls.CheckForAllInputFlags())
				ResetButtonPressTimer();
	}
}




//-----------------------------------------------------------------------------------------
void MainScreen::WriteDBInfo()
{
	GameInfo *gi = mainMenu->GetCurrentSelectedItem();
	int id;
	string output;
	SQLiteUtils::dbDataPair WhereClause(make_pair("romName", gi->romName));
	//db->doDBQuerey("ID", WhereClause, output);
	db->doDBQuerey("romName", WhereClause, output);

	size_t found = output.find_last_of("|");
	string rowID = output.substr(found + 1);
	id = atoi(rowID.c_str());

	string gameData = db->GetDataFromID(id, "");

	//we havent put this game in the db yet, so add all the info
	if (gameData == "")
	{
		vector<SQLiteUtils::dbDataPair> newGameInfo;
		newGameInfo.push_back(make_pair("romName", gi->romName));
		newGameInfo.push_back(make_pair("name", gi->name));
		newGameInfo.push_back(make_pair("manufacturer", gi->manufacturer));
		newGameInfo.push_back(make_pair("year", gi->year));
		newGameInfo.push_back(make_pair("numSelectedRandom", "0"));
		newGameInfo.push_back(make_pair("numSelectedManually", "0"));
		newGameInfo.push_back(make_pair("totalPlayTime", "0"));
		newGameInfo.push_back(make_pair("numCredits", "0"));
		db->insertNewDataEntry(newGameInfo, output);
		id = db->GetLatestID();
	}



	vector<SQLiteUtils::dbDataPair> updateGameInfo;
	if (isRandomSelection)
		updateGameInfo.push_back(make_pair("numSelectedRandom", "numSelectedRandom + 1"));
	else
		updateGameInfo.push_back(make_pair("numSelectedManually", "numSelectedManually + 1"));

	updateGameInfo.push_back(make_pair("lastPlayedDate", "DateTime('now')"));
	updateGameInfo.push_back(make_pair("numCredits", "numCredits + " + to_string(numCredits)));
	updateGameInfo.push_back(make_pair("totalPlayTime", "totalPlayTime + " + to_string(currentGamePlayTime)));
	SQLiteUtils::dbDataPair WhereClause2(make_pair("id", to_string(id)));
	db->UpdateIntEntry(updateGameInfo, WhereClause2, output);

	//update the game info thats in mem
	gi->numCredits += numCredits;
	gi->totalPlayTime += currentGamePlayTime;
	gi->lastPlayedDate = "Today";
}

void MainScreen::ReturnFromGame()
{
	ClearMainJoystickInputFlags();
	menuMode = true;
	snaps->LoadCurrentSnapshot();
	currentGamePlayTime = SDL_GetTicks() - currentGamePlayTime;
	WriteDBInfo();
}

void MainScreen::DrawScene()
{
	snaps->Draw();
	mainMenu->Draw();
}

