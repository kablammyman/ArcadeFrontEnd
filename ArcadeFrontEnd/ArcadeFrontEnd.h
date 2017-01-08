#pragma once

#include "resource.h"


#include <string>
#include "Menu.h"
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
void LoadCurrentSnapshot();
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
SDL_Window *window;
SDL_Renderer *renderer;

SDL_Surface *SnapImgSurface = NULL;
SDL_Rect SnapImgRect;
SDL_Texture *SnapTexture = NULL;
string curSnap = "";

//Screen dimension constants
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;
bool isIdle = false;


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
void LoadCurrentSnapshot()
{
	string gameName = mainMenu->GetCurrentSelectedItemRomName();

	if (curSnap == gameName)
		return;

	string imgPath = CFGHelper::snapsPath + "\\" + FileUtils::GetFileNameNoExt(gameName) + ".png";
	
	SDL_Surface *temp = IMG_Load(imgPath.c_str());
	if (temp == NULL)
		return;

	SDL_FillRect(SnapImgSurface, NULL, 0x000000);
	SDL_BlitSurface(temp, NULL, SnapImgSurface, NULL);

	//transfer the completed surface to the texture
	SDL_UpdateTexture(SnapTexture, NULL, SnapImgSurface->pixels, SnapImgSurface->pitch);

	SDL_FreeSurface(temp);	
	curSnap = gameName;
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
	LoadCurrentSnapshot();
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
			LoadCurrentSnapshot();
		}
		else if (P1Controls.CheckJoystickFlag(JOY_UP))
		{
			mainMenu->Prev(SDL_GetTicks());
			LoadCurrentSnapshot();
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
void GenerateGameList(string mameListPath)
{
	ofstream masterList;
	ifstream mameList;

	masterList.open(CFGHelper::filePathBase + "\\gamelist.txt");
	mameList.open(mameListPath);
	string line;
	string name;
	string romName;
	string manufacturer;
	string year;

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
					AllGameListInfo.push_back(newGame);
					//have to use semi colons since some game names contians commas
					masterList << romName << ";" << name << ";" << manufacturer << ";" << year << endl;
				}
			}
			
		}
		mameList.close();
		masterList.close();
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
	ifstream masterList;

	masterList.open(CFGHelper::filePathBase + "\\gamelist.txt");

	string line;

	if (!masterList.is_open())
	{
		MessageBox(NULL, L"cant find game list", L"cant find your game list, restart this to have it auto generate", MB_OK);
		exit(0);
	}
	while (getline(masterList, line))
	{

		GameInfo newGame;
		vector<string> tokens = StringUtils::Tokenize(line,";");
		newGame.romName = tokens[0];
		newGame.name = tokens[1];
		newGame.manufacturer = tokens[2];
		newGame.year = tokens[3];
		string romPath = CFGHelper::romPath + "\\" + newGame.romName + ".zip";
		
		if (verify)
		{
			if (FileUtils::DoesPathExist(romPath))
			{
				AllGameListInfo.push_back(newGame);
			}
		}
		else
			AllGameListInfo.push_back(newGame);
	}
	
	masterList.close();
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

void ListChildWindows(HWND hWnd, DWORD dwPIDCheck)
{
	DWORD dwPID;
	wchar_t szBuffer[1000] = { 0 };
	GetWindowThreadProcessId(hWnd, &dwPID);
	GetWindowText(hWnd, szBuffer, 1000);
	//if (dwPID == dwPIDCheck)
	//	printf("%-8X %-40s %-8d\n", hWnd, szBuffer, dwPID);
	for (hWnd = GetWindow(hWnd, GW_CHILD); hWnd != NULL; hWnd = GetWindow(hWnd, GW_HWNDNEXT))
		ListChildWindows(hWnd, dwPIDCheck);
}

int mainExample2(int argc, char **argv)
{
	HWND hDesktopWindow = NULL;
	if (argc == 1)
	{
		printf("Invalid  arguments\n");
		return 0;
	}
	hDesktopWindow = GetDesktopWindow();
	ListChildWindows(hDesktopWindow, atoi(argv[1]));
	return 0;
}