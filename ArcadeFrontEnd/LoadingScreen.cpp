#include "stdafx.h"
#include <fstream>

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_syswm.h"
#include <SDL_image.h>


#include "LoadingScreen.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "GameInfo.h"
#include "CFGHelper.h"



using namespace std;
//do this on first start up....or if you cant find the DB
LoadingScreen::LoadingScreen(std::vector<GameInfo> *a, SQLiteUtils *d, SDL_ScreenStruct *ss)
{
	className = "LoadingScreen";
	AllGameListInfo = a;
	db = d;
	screenStruct = ss;
	TTF_Font* Sans = TTF_OpenFont(screenStruct->fontPath.c_str(), screenStruct->fontSize);
	White = { 255, 255, 255 };
	backgroundWork = NULL;

	messageRect.x = 0;
	messageRect.y = screenStruct->screenH / 2;
	messageRect.w = 100;
	messageRect.h = 100;

	populateDB = false;

	if (populateDB)
	{
		messageSurface = TTF_RenderText_Solid(screenStruct->font, "Cant find your game database, Generating it now...please wait", White);
		messageTexture = SDL_CreateTextureFromSurface(screenStruct->renderer, messageSurface); //now you can convert it into a texture

		backgroundWork = new thread(&LoadingScreen::GenerateGameList,this,CFGHelper::mameList, CFGHelper::catverPath);
	}
	else
	{

		messageSurface = TTF_RenderText_Solid(screenStruct->font, "Loading...", White);
		messageTexture = SDL_CreateTextureFromSurface(screenStruct->renderer, messageSurface); //now you can convert it into a texture
		/*string imgPath = CFGHelper::snapsPath + "\\" + "005.png";
		SDL_Rect destRect = { 0,0,0,0 };
		SDL_Surface *temp = IMG_Load(imgPath.c_str());
		SDL_Texture* Img = SDL_CreateTextureFromSurface(screenStruct->renderer, temp);
		destRect.w = temp->w;
		destRect.h = temp->h;*/

		backgroundWork = new thread(&LoadingScreen::FillGameListFromDB,this,true);
	}

	messageRect.w = messageSurface->w;
	messageRect.h = messageSurface->h;
	backgroundWork->detach();
}

void LoadingScreen::GenerateGameList(string mameListPath, string catverListPath)
{
	string line;
	ifstream catverList;
	vector<SQLiteUtils::dbDataPair> catver;
	//only do this if we have a catver to read from, otherwise skip
	if (!catverListPath.empty())
	{
		catverList.open(catverListPath);

		while (getline(catverList, line))
		{
			//we got to theend of cat listing
			if (line.find("[VerAdded]") != string::npos)
				break;
			SQLiteUtils::dbDataPair pair;
			vector<string> temp = StringUtils::Tokenize(line, "=");

			//we only want the game data, no headers or anything else
			if (temp.size() <= 1)
				continue;

			pair.first = temp[0];
			pair.second = temp[1];

			catver.push_back(pair);
		}
		catverList.close();
	}
	ifstream mameList;
	mameList.open(mameListPath);

	string name;
	string romName;
	string manufacturer;
	string year;
	size_t lastIndexUsed = 0;

	if (mameList.is_open())
	{
		while (getline(mameList, line))
		{
			size_t rom = line.find("<game name=");
			if (rom != string::npos)
			{
				romName = StringUtils::GetDataBetweenChars(line, '"', '"', rom);
				continue;
			}
			size_t desc = line.find("<description>");
			if (desc != string::npos)
			{
				name = StringUtils::GetDataBetweenSubStrings(line, "<description>", "</description>", desc);
				continue;
			}
			size_t yr = line.find("<year>");
			if (yr != string::npos)
			{
				//string y = StringUtils::GetDataBetweenSubStrings(line, "<year>", "</year>", yr);
				//year = stoi(y);
				year = StringUtils::GetDataBetweenSubStrings(line, "<year>", "</year>", yr);
				continue;
			}
			size_t man = line.find("<manufacturer>");
			if (man != string::npos)
			{
				manufacturer = StringUtils::GetDataBetweenSubStrings(line, "<manufacturer>", "</manufacturer>", man);
				continue;
			}
			if (line.find("</game>") != string::npos)
			{
				GameInfo newGame;
				newGame.romName = romName;
				newGame.name = name;
				newGame.manufacturer = manufacturer;
				newGame.year = year;

				string romPath = CFGHelper::romPath + "\\" + romName + ".zip";
				if (FileUtils::DoesPathExist(romPath))
				{
					string output;
					vector<SQLiteUtils::dbDataPair> newGameInfo;
					newGameInfo.push_back(make_pair("romName", romName));
					newGameInfo.push_back(make_pair("name", name));
					newGameInfo.push_back(make_pair("manufacturer", manufacturer));
					newGameInfo.push_back(make_pair("year", year));
					newGameInfo.push_back(make_pair("numSelectedRandom", "0"));
					newGameInfo.push_back(make_pair("numSelectedManually", "0"));
					newGameInfo.push_back(make_pair("totalPlayTime", "0"));
					newGameInfo.push_back(make_pair("numCredits", "0"));

					if (catver.size() > 0)
					{
						//since both list are sorted, we can prob march thru the array in a smart way to find if a game exists
						for (size_t i = lastIndexUsed; i < catver.size(); i++)
						{
							if (catver[i].first == romName)
							{
								newGameInfo.push_back(make_pair("genre", catver[i].second));
								newGame.genre = catver[i].second;
								lastIndexUsed = i;
								break;
							}
							else if (catver[i].first[0] > romName[0])
							{
								//prob not in the list
								break;
							}
						}
					}
					db->insertNewDataEntry(newGameInfo, output);

					newGame.id = db->GetLatestID();
					AllGameListInfo->push_back(newGame);

				}
			}

		}
		mameList.close();

	}
	else
	{
		MessageBox(NULL, L"cant find mame list", L"cant find your mame list, have mame create it, then put that path in the cfg", MB_OK);
		exit(0);
	}
}
//-----------------------------------------------------------------------------------------
void LoadingScreen::FillGameListFromDB(bool verify)
{
	string output;
	string querey = "id,romName,name,manufacturer,year,genre";
	db->doDBQuerey(querey, output);
	vector<string> allGames;
	db->SplitDataIntoResults(allGames, output, "ID", false);

	for (size_t i = 0; i < allGames.size(); i++)
	{

		GameInfo newGame;
		vector<string> tokens = StringUtils::Tokenize(allGames[i], "\n");
		newGame.id = atoi(db->GetDataFromSingleLineOutput(tokens[0]).c_str());
		newGame.romName = db->GetDataFromSingleLineOutput(tokens[1]);
		newGame.name = db->GetDataFromSingleLineOutput(tokens[2]);
		newGame.manufacturer = db->GetDataFromSingleLineOutput(tokens[3]);
		newGame.year = db->GetDataFromSingleLineOutput(tokens[4]);

		if (tokens.size() >= 5)
			newGame.genre = db->GetDataFromSingleLineOutput(tokens[5]);


		if (verify)
		{
			string romPath = CFGHelper::romPath + "\\" + newGame.romName + ".zip";
			if (FileUtils::DoesPathExist(romPath))
			{
				AllGameListInfo->push_back(newGame);
			}
		}
		else
			AllGameListInfo->push_back(newGame);
	}
	NotifyObservers();
}
//-----------------------------------------------------------------------------------------
bool LoadingScreen::IsStringInVector(vector<string> & list, string keyword)
{
	for (size_t i = 0; i < list.size(); i++)
	{
		size_t pos = list[i].find(keyword);
		if (pos != string::npos)
			return true;
	}
	return false;
}
//-----------------------------------------------------------------------------------------
void LoadingScreen::RemoveCrappyROMS(bool deleteZip)
{
	string output;
	string querey = "id,romName,name,manufacturer,year,genre";
	db->doDBQuerey(querey, output);
	vector<string> allGames;
	db->SplitDataIntoResults(allGames, output, "ID", false);

	vector<string> badManufacturer;
	badManufacturer.push_back("Stern");
	badManufacturer.push_back("Igrosoft");
	badManufacturer.push_back("IGT - International Gaming Technology");
	badManufacturer.push_back("bootleg");

	vector<string> badGenre;
	badGenre.push_back("Casino");
	badGenre.push_back("Electromechanical");
	badGenre.push_back("Quiz");
	badGenre.push_back("Mahjong");
	badGenre.push_back("Tabletop");

	for (size_t i = 0; i < allGames.size(); i++)
	{
		bool deleteRom = false;
		GameInfo newGame;
		vector<string> tokens = StringUtils::Tokenize(allGames[i], "\n");
		newGame.id = atoi(db->GetDataFromSingleLineOutput(tokens[0]).c_str());
		newGame.romName = db->GetDataFromSingleLineOutput(tokens[1]);
		newGame.name = db->GetDataFromSingleLineOutput(tokens[2]);
		newGame.manufacturer = db->GetDataFromSingleLineOutput(tokens[3]);
		newGame.year = db->GetDataFromSingleLineOutput(tokens[4]);

		if (tokens.size() >= 5)
		{
			newGame.genre = db->GetDataFromSingleLineOutput(tokens[5]);
			vector<string> temp = StringUtils::Tokenize(newGame.genre, "/");
			StringUtils::TrimWhiteSpace(temp[0]);
			if (IsStringInVector(badGenre, temp[0]))
				deleteRom = true;
			else if (temp.size() > 1)
			{
				StringUtils::TrimWhiteSpace(temp[1]);
				if (IsStringInVector(badGenre, temp[1]))
					deleteRom = true;
			}
		}

		if (IsStringInVector(badManufacturer, newGame.manufacturer))
			deleteRom = true;

		if (deleteRom)
		{
			string querey = "DELETE FROM " + tableName + " WHERE ID = " + to_string(newGame.id);
			string out;
			db->executeSQL(querey, out);

			if (deleteZip)
			{
				string romPath = CFGHelper::romPath + "\\" + newGame.romName + ".zip";
				if (FileUtils::DoesPathExist(romPath))
				{
					FileUtils::Delete_File(romPath);
				}
			}
		}
	}
}

void LoadingScreen::Update()
{

	messageRect.x++;
	if(messageRect.x + messageRect.w > screenStruct->screenW)
		messageRect.x = 0;

		/*SDL_FillRect(SnapImgSurface, NULL, 0x000000);
		SDL_BlitSurface(temp, NULL, SnapImgSurface, NULL);
		SDL_UpdateTexture(SnapTexture, NULL, SnapImgSurface->pixels, SnapImgSurface->pitch);
		SDL_RenderCopy(screenStruct->renderer, SnapTexture, NULL, &destRect);*/
}

void LoadingScreen::Draw()
{
	SDL_RenderCopy(screenStruct->renderer, messageTexture, NULL, &messageRect);
	SDL_RenderPresent(screenStruct->renderer);
}