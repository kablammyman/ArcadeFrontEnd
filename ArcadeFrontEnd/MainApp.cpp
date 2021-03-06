#include "stdafx.h"

#include "MainApp.h"
#include "CFGHelper.h"
#include "SQLiteUtils.h"
#include "WindowsStuff.h"
#include "FrontendButtons.h"


int MainApp::inputDelay;

MainApp::MainApp()
{
	//Screen dimension constants
	SCREEN_WIDTH = 1280;
	SCREEN_HEIGHT = 1024;

	sceneIndex = 0;
	populateDB = false;

	inputDelay = 5;
	largeFontSize = 48;
	fontSize = 8;
	admin = new AdminWork(&AllGameListInfo, &db);
	admin->AddObserver(this);
}

MainApp::~MainApp()
{
	SDL_DestroyWindow(window);
}
void MainApp::AddMainScreen()
{
	mainMenu = new MainScreen(renderer, AllGameListInfo, &db,SCREEN_WIDTH, SCREEN_HEIGHT, screenStruct.fontPath, screenStruct.fontSize, SDL_Color{ 0,0,255 }, SDL_Color{ 255,255,255 }, SDL_Color{ 0,0,255 });
	mainMenu->AddObserver(this);
	allScenes.push_back(mainMenu);
}

void MainApp::InitDB()
{
	
	CFGHelper::LoadCFGFile();
	db.openSQLiteDB(CFGHelper::dbPath, dbOut);
	//check to see if we are making a new db table or we have an existing one
	db.executeSQL("pragma schema_version", dbOut);
	if (db.GetDataFromSingleLineOutput(dbOut) == "0")
	{
		//brand new file, create the db
		string create = "ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,romName TEXT ,name TEXT,year TEXT, manufacturer TEXT,genre TEXT,numCredits INTEGER,totalPlayTime INTEGER,lastPlayedDate TEXT, numSelectedRandom INTEGER,numSelectedManually INTEGER";
		if (!db.createTable(tableName, create))
		{
			MessageBox(NULL, L"cant create db", L"couldnt create the db for your stats", MB_OK);
			exit(0);
		}

		admin->DoGenerateGameList();
		loading->SetLoadingMessage("Creating new game list");
		curScreen = loading;
	}
	else
	{
		db.setTableName(tableName);
		admin->DoFillGameList();
		loading->SetLoadingMessage("Loading");
		curScreen = loading;
	}

}

void MainApp::InitWindow(bool fullScreen)
{
	// Create an application window with the following settings:
	if(!fullScreen)
	window = SDL_CreateWindow(
		"Arcade Frontend",                  // window title
		SDL_WINDOWPOS_UNDEFINED,           // initial x position
		SDL_WINDOWPOS_UNDEFINED,           // initial y position
		SCREEN_WIDTH,                               // width, in pixels
		SCREEN_HEIGHT,                               // height, in pixels
		SDL_WINDOW_RESIZABLE
	);
	else
		window = SDL_CreateWindow(
			"Arcade Frontend",                  // window title
			SDL_WINDOWPOS_UNDEFINED,           // initial x position
			SDL_WINDOWPOS_UNDEFINED,           // initial y position
			SCREEN_WIDTH,                               // width, in pixels
			SCREEN_HEIGHT,                               // height, in pixels
			SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE
		);

	// Check that the window was successfully created
	if (window == NULL) {
		// In the case that the window could not be made...
		printf("Could not create window: %s\n", SDL_GetError());
		exit(1);
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
	MainScreen::window = window;
}

void MainApp::InitScreens()
{
	string menuFontPath = CFGHelper::filePathBase + "\\consola.ttf";
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	TTF_Font* font = TTF_OpenFont(menuFontPath.c_str(), largeFontSize);

	screenStruct.screenW = SCREEN_WIDTH;
	screenStruct.screenH = SCREEN_HEIGHT;
	screenStruct.font = font;
	screenStruct.renderer = renderer;
	screenStruct.fontSize = fontSize;
	screenStruct.fontPath = menuFontPath;

	loading = new LoadingScreen(&screenStruct,"what am i doing with my life?");
	loading->AddObserver(this);
	
	options = new OptionsScreen(&screenStruct);
	options->AddObserver(this);

	allScenes.push_back(loading);
	allScenes.push_back(options);

	curScreen = allScenes[sceneIndex];

}

void MainApp::LogicUpdate()
{
	curScreen->UpdateScene();
}

void MainApp::GraphicsUpdate()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	curScreen->DrawScene();
	SDL_RenderPresent(renderer);
}

void MainApp::Notify(Observee* observee)
{
	if (observee->className == "AdminWork")
	{
		if (observee->id == 0)
		{
			AddMainScreen();
			curScreen = mainMenu;
		}
		else if (observee->id == 1)
		{
			curScreen = mainMenu;
		}
		else
		{
			/*admin->DoTest();
			loading->SetLoadingMessage("30 second test");
			curScreen = loading;*/
		}
	}
	else if (observee->className == "OptionScreen")
	{
		switch (observee->id)
		{
		case 2: 
			loading->SetLoadingMessage("Refreshing Game List");
			curScreen = loading;
			admin->DoGenerateGameList();

			break;
		case 3:
			loading->SetLoadingMessage("Removing roms we cant play or dont like");
			curScreen = loading;
			admin->DoRemoveCrappyRoms();
			break;
		case 4:
			curScreen = mainMenu;
			break;
		case 5:
			exit(0);
			break;
		}
		
	}
	else if (observee->className == "MainScreen")
	{
		if (observee->id == 0)
		{
			curScreen = options;
		}
	}

}