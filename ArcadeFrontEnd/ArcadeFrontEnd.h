#pragma once

#include "resource.h"


#include <string>
#include "Menu.h"
#include "SnapShot.h"
#include "CFGHelper.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "SQLiteUtils.h"

#include "Controls.h"

using namespace std;

//are we in the main menu, or playing a game via mame?
bool menuMode = true;

#include <fstream>


BOOL CALLBACK	 EnumWindowsProc(HWND hwnd, LPARAM param);

VOID CALLBACK WaitOrTimerCallback(_In_  PVOID lpParameter,_In_  BOOLEAN TimerOrWaitFired);

void LaunchGame(string gameName);

void WriteDBInfo();

unsigned int lastButtonPressTime;
unsigned int buttonPressTimeout = 30000;//should be 5 minutes

unsigned int currentGamePlayTime;
int numCredits;
bool isRandomSelection;

void ResetButtonPressTimer();
bool IsButtonTimeUp();

HRESULT InitDirectInput(HWND hDlg);
VOID FreeDirectInput();
HRESULT UpdateInputState();
HWND mainWindowHandle;
HWND mameWindowHandle;

HANDLE mameProc = NULL;
HANDLE mameHandle = NULL;

vector<GameInfo> AllGameListInfo;
GameInfo curGameStats;

SQLiteUtils db;

Menu *mainMenu;
SnapShot *snaps;

SDL_Window *window;
SDL_Renderer *renderer;



//Screen dimension constants
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;
bool isIdle = false;
const string tableName = "Arcade";

/////////////timer stuff///////////////////
#define DEFAULT_RESOLUTION  1
static int ticks = 0;
static Uint32 SDLCALL
ticktock(Uint32 interval, void *param)
{
	++ticks;
	return (interval);
}


void ResetButtonPressTimer()
{
	lastButtonPressTime = SDL_GetTicks();
	isIdle = false;
}


bool IsButtonTimeUp()
{
	if (SDL_GetTicks() - lastButtonPressTime > buttonPressTimeout)
	{
		isIdle = true;
		return true;
	}
	return false;
}

static Uint32 SDLCALL
callback(Uint32 interval, void *param)
{
	SDL_Log("Timer %d : param = %d", interval, (int)(uintptr_t)param);
	return interval;
}
SDL_TimerID inGameTimer, inputTimer, totalRuntime;


//-----------------------------------------------------------------------------------------
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM param)
{
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == param)
	{
		mameWindowHandle = hwnd;
		return FALSE;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------------------
bool myCreateProcess(string pathAndName, string args, HANDLE &processCreated)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	
	ZeroMemory(&pi, sizeof(pi));
	if (!
		CreateProcessA(
			pathAndName.c_str(),
			(LPSTR)args.c_str(),
			NULL, NULL, FALSE,
			CREATE_NEW_CONSOLE,
			NULL, NULL,
			&si,
			&pi)
		)
	{
		return false;
	}

	processCreated = pi.hProcess;

	return true;
}
//-----------------------------------------------------------------------------------------
void LaunchGame(string gameName)
{
	//SDL_MinimizeWindow( window);
	menuMode = false;

	string command = CFGHelper::mamePath;
	//without the exe name as the first param, CreateProcess doesnt work right....CreateProcess allows you to override
	//nornmal cmd behavior for some dumb ass reason!
	string args = FileUtils::GetFileNameFromPathString(CFGHelper::mamePath) + " " +
		gameName + " -rp \"" + CFGHelper::romPath + "\" -inipath \""+ CFGHelper::mameini +"\" -joystick";

	
	myCreateProcess(command,args,  mameProc);
	DWORD dwProcessID = GetProcessId(mameProc);
	HANDLE hProcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
	

	
	RegisterWaitForSingleObject(&mameHandle, hProcHandle, WaitOrTimerCallback, NULL, INFINITE, WT_EXECUTEONLYONCE);
	Sleep(500);
	if (!EnumWindows(EnumWindowsProc, dwProcessID) && (GetLastError() == ERROR_SUCCESS)) {
		//nopw that we have our main window, lets find child window
		if (!EnumChildWindows(mameWindowHandle, EnumWindowsProc, 0) && (GetLastError() == ERROR_SUCCESS))
		{
			printf("hello");
		}
	}

	currentGamePlayTime = SDL_GetTicks();
}

//-----------------------------------------------------------------------------------------
VOID CALLBACK WaitOrTimerCallback(
	_In_  PVOID lpParameter,
	_In_  BOOLEAN TimerOrWaitFired
)
{
	//MessageBox(0, L"The process has exited.", L"INFO", MB_OK);
	SDL_RestoreWindow(window);
	menuMode = true;
	snaps->LoadCurrentSnapshot();
	currentGamePlayTime = SDL_GetTicks() - currentGamePlayTime;
	WriteDBInfo();
	return;
}

void StartGame(bool startRandom = false)
{
	ResetButtonPressTimer();
	menuMode = false;
	if(startRandom)
		mainMenu->SelectRandomGame();

	string romName = mainMenu->GetCurrentSelectedItemRomName();
	currentGamePlayTime = 0;
	numCredits = 0;
	isRandomSelection = startRandom;
	LaunchGame(romName);
}
void LogicUpdate()
{
	
	if (P1Controls.CheckForAllInputFlags())
		ResetButtonPressTimer();

	
	if (IsButtonTimeUp() && !menuMode)
	{
		SendMessage(mameWindowHandle, WM_CLOSE, 0, 0);
	}

	if (menuMode)
	{
		if (P1Controls.CheckJoystickFlag(JOY_DOWN))
		{
			mainMenu->Next(SDL_GetTicks());
			snaps->LoadCurrentSnapshot();
		}
		else if (P1Controls.CheckJoystickFlag(JOY_UP))
		{
			mainMenu->Prev(SDL_GetTicks());
			snaps->LoadCurrentSnapshot();
		}

		if (P1Controls.CheckButtonFlag(BUTTON1))
		{
			StartGame();
		}

		//select a new random game
		if (isIdle)
		{
			StartGame(true);
		}
	}
}

//do this on first start up....or if you cant find the DB
void GenerateGameList(string mameListPath,string catverListPath)
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
			if(line.find("[VerAdded]") != string::npos)
				break;
			SQLiteUtils::dbDataPair pair;
			vector<string> temp =StringUtils::Tokenize(line, "=");

			//we only want the game data, no headers or anything else
			if(temp.size() <= 1)
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
				romName  = StringUtils::GetDataBetweenChars(line,'"','"',rom);
				continue;
			}
			size_t desc = line.find("<description>");
			if (desc != string::npos)
			{
				name = StringUtils::GetDataBetweenSubStrings(line, "<description>","</description>", desc);
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

				string romPath = CFGHelper::romPath +"\\"+ romName+".zip";
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
					db.insertNewDataEntry(newGameInfo, output);

					newGame.id = db.GetLatestID();
					AllGameListInfo.push_back(newGame);
					
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
void FillGameListFromCSV( bool verify = false)
{
	string output;
	string querey = "id,romName,name,manufacturer,year,genre";
	db.doDBQuerey(querey, output);
	vector<string> allGames;
	db.SplitDataIntoResults(allGames, output, "ID",false);

	for(size_t i = 0; i < allGames.size(); i++)
	{

		GameInfo newGame;
		vector<string> tokens = StringUtils::Tokenize(allGames[i],"\n");
		newGame.id = atoi(db.GetDataFromSingleLineOutput(tokens[0]).c_str());
		newGame.romName = db.GetDataFromSingleLineOutput(tokens[1]);
		newGame.name = db.GetDataFromSingleLineOutput(tokens[2]);
		newGame.manufacturer = db.GetDataFromSingleLineOutput(tokens[3]);
		newGame.year = db.GetDataFromSingleLineOutput(tokens[4]);
		
		if (tokens.size() >= 5)
			newGame.genre = db.GetDataFromSingleLineOutput(tokens[5]);

		
		if (verify)
		{
			string romPath = CFGHelper::romPath + "\\" + newGame.romName + ".zip";
			if (FileUtils::DoesPathExist(romPath))
			{
				AllGameListInfo.push_back(newGame);
			}
		}
		else
			AllGameListInfo.push_back(newGame);
	}
}
//-----------------------------------------------------------------------------------------
void WriteDBInfo()
{
	GameInfo gi = mainMenu->GetCurrentSelectedItem();
	int id;
	string output;
	SQLiteUtils::dbDataPair WhereClause(make_pair("romName", gi.romName));
	db.doDBQuerey("ID", WhereClause,output);
	
	size_t found = output.find_last_of("|");
	string rowID = output.substr(found + 1);
	id = atoi(rowID.c_str());

	string gameData = db.GetDataFromID(id, "");

	//we havent put this game in the db yet, so add all the info
	if (gameData == "")
	{
		vector<SQLiteUtils::dbDataPair> newGameInfo;
		newGameInfo.push_back(make_pair("romName", gi.romName));
		newGameInfo.push_back(make_pair("name", gi.name));
		newGameInfo.push_back(make_pair("manufacturer", gi.manufacturer));
		newGameInfo.push_back(make_pair("year", gi.year));
		newGameInfo.push_back(make_pair("numSelectedRandom", "0"));
		newGameInfo.push_back(make_pair("numSelectedManually", "0"));
		newGameInfo.push_back(make_pair("totalPlayTime", "0"));
		newGameInfo.push_back(make_pair("numCredits", "0"));
		db.insertNewDataEntry(newGameInfo, output);
		id = db.GetLatestID();
	}
	
	

	vector<SQLiteUtils::dbDataPair> updateGameInfo;
	if (isRandomSelection)
		updateGameInfo.push_back(make_pair("numSelectedRandom", "numSelectedRandom + 1"));
	else
		updateGameInfo.push_back(make_pair("numSelectedManually", "numSelectedManually + 1"));

	updateGameInfo.push_back(make_pair("lastPlayedDate", "DateTime('now')"));
	updateGameInfo.push_back(make_pair("numCredits", "numCredits + " + to_string(numCredits)));
	updateGameInfo.push_back(make_pair("totalPlayTime", "totalPlayTime + " + to_string(currentGamePlayTime)));
	SQLiteUtils::dbDataPair WhereClause2(make_pair("id",to_string(id)));
	db.UpdateIntEntry(updateGameInfo, WhereClause2,output);

}
//-----------------------------------------------------------------------------------------
bool IsStringInVector(vector<string> & list, string keyword)
{
	for (size_t i = 0; i < list.size(); i++)
	{
		size_t pos = list[i].find(keyword);
			if(pos != string::npos)
				return true;
	}
	return false;
}
//-----------------------------------------------------------------------------------------
void RemoveCrappyROMS(bool deleteZip = false)
{
	string output;
	string querey = "id,romName,name,manufacturer,year,genre";
	db.doDBQuerey(querey, output);
	vector<string> allGames;
	db.SplitDataIntoResults(allGames, output, "ID", false);

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
		newGame.id = atoi(db.GetDataFromSingleLineOutput(tokens[0]).c_str());
		newGame.romName = db.GetDataFromSingleLineOutput(tokens[1]);
		newGame.name = db.GetDataFromSingleLineOutput(tokens[2]);
		newGame.manufacturer = db.GetDataFromSingleLineOutput(tokens[3]);
		newGame.year = db.GetDataFromSingleLineOutput(tokens[4]);

		if (tokens.size() >= 5)
		{
			newGame.genre = db.GetDataFromSingleLineOutput(tokens[5]);
			vector<string> temp = StringUtils::Tokenize(newGame.genre,"/");
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
			db.executeSQL(querey,out);

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