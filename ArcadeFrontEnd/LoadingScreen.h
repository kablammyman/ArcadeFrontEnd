#pragma once
#include <string>
#include <vector>

#include "SDL_ttf.h"
#include "SDL.h"

#include "Scene.h"
#include "SQLiteUtils.h"
#include "GameInfo.h"
#include "SDL_ScreenStruct.h"

#include <thread>


//even tho this is called loading screen, this is for any work is done to worth with the db
class LoadingScreen : public Scene
{
	SQLiteUtils *db;
	std::vector<GameInfo> *AllGameListInfo;

	SDL_ScreenStruct *screenStruct;
	SDL_Color White;
	SDL_Rect messageRect;
	SDL_Surface* messageSurface;
	SDL_Texture* messageTexture;
	bool populateDB;
	std::thread *backgroundWork;


public:
	LoadingScreen(std::vector<GameInfo> *a, SQLiteUtils *d, SDL_ScreenStruct *ss);
	~LoadingScreen()
	{
		if (messageSurface)
			SDL_FreeSurface(messageSurface);
		
		if(messageTexture)
			SDL_DestroyTexture(messageTexture);
	}
	std::string tableName;
	void GenerateGameList(std::string mameListPath, std::string catverListPath);
	void FillGameListFromDB(bool verify = false);
	bool IsStringInVector(std::vector<std::string> & list, std::string keyword);
	void RemoveCrappyROMS(bool deleteZip = false);

	void Draw();
	void Update();

};