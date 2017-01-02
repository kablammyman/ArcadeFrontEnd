#pragma once

#include "resource.h"

#include <Windows.h>
#pragma warning(push)
#pragma warning(disable:6000 28251)
#include <dinput.h>
#pragma warning(pop)

#include <dinputd.h>
#include <string>
#include "Menu.h"
#include "CFGHelper.h"
#include "FileUtils.h"
#include "StringUtils.h"

using namespace std;

//are we in the main menu, or playing a game via mame?
bool menuMode = true;

#include <fstream>

//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
BOOL CALLBACK    EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext);
BOOL CALLBACK    EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
HRESULT InitDirectInput(HWND hDlg);
VOID FreeDirectInput();
HRESULT UpdateInputState();
HWND mainWindowHandle;

HANDLE mameProc = NULL;
HANDLE mameHandle = NULL;

Menu *mainMenu;

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=nullptr; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=nullptr; } }
LPDIRECTINPUT8          g_pDI = nullptr;

//need an array of these to check for button presses in game from all joysticks
LPDIRECTINPUTDEVICE8    g_pJoystick = nullptr;

#define VK_A 0x41
#define VK_B 0x42
#define VK_E 0x45
#define VK_H 0x48
#define VK_O 0x4F
#define VK_R 0x52
#define VK_S 0x53
#define VK_W 0x57

vector<GameInfo> AllGameListInfo;

SDL_Surface *SnapImgSurface = NULL;
SDL_Rect SnapImgRect;
SDL_Texture *SnapTexture = NULL;
string curSnap = "";

/////////////timer stuff///////////////////
#define DEFAULT_RESOLUTION  1
static int ticks = 0;
static Uint32 SDLCALL
ticktock(Uint32 interval, void *param)
{
	++ticks;
	return (interval);
}

static Uint32 SDLCALL
callback(Uint32 interval, void *param)
{
	SDL_Log("Timer %d : param = %d", interval, (int)(uintptr_t)param);
	return interval;
}
SDL_TimerID inGameTimer, inputTimer, totalRuntime;

//////////////////////direct input/////////////////////////
struct DI_ENUM_CONTEXT
{
	DIJOYCONFIG* pPreferredJoyCfg;
	bool bPreferredJoyCfgValid;
};

VOID CALLBACK WaitOrTimerCallback(
	_In_  PVOID lpParameter,
	_In_  BOOLEAN TimerOrWaitFired
)
{
	//MessageBox(0, L"The process has exited.", L"INFO", MB_OK);
	//SDL_RestoreWindow(window);
	menuMode = true;
	return;
}
//-----------------------------------------------------------------------------------------
int SendKeyPressToProcess(void)
{
	HWND hWnd = NULL;
	HWND hWndEdit = NULL;

	hWnd = FindWindow(L"Notepad", NULL);
	if (hWnd == NULL)
	{
		printf("Error: Can't find Notepad, aborting\n");
		return 0;
	}
	hWndEdit = FindWindowEx(hWnd, NULL, L"Edit", NULL);
	if (hWndEdit == NULL)
	{
		printf("Error: Can't find Notepad Edit, aborting\n");
		return 0;
	}
	PostMessage(hWndEdit, WM_KEYDOWN, VK_B, 1);
	PostMessage(hWndEdit, WM_KEYDOWN, VK_O, 1);
	PostMessage(hWndEdit, WM_KEYDOWN, VK_B, 1);
	PostMessage(hWndEdit, WM_KEYDOWN, VK_SPACE, 1);
	PostMessage(hWndEdit, WM_KEYDOWN, VK_W, 1);
	PostMessage(hWndEdit, WM_KEYDOWN, VK_A, 1);
	PostMessage(hWndEdit, WM_KEYDOWN, VK_S, 1);
	PostMessage(hWndEdit, WM_KEYDOWN, VK_SPACE, 1);
	PostMessage(hWndEdit, WM_KEYDOWN, VK_H, 1);
	PostMessage(hWndEdit, WM_KEYDOWN, VK_E, 1);
	PostMessage(hWndEdit, WM_KEYDOWN, VK_R, 1);
	PostMessage(hWndEdit, WM_KEYDOWN, VK_E, 1);
	return 0;
}
//-----------------------------------------------------------------------------------------
void LoadCurrentSnapshot()
{
	string gameName = mainMenu->GetCurrentSelectedItem();
	
	if(curSnap == gameName)
		return;

	SDL_Rect destRect = { 0,0,0,0 };

	//clear the dest surface...this will be our border color
	SDL_FillRect(SnapImgSurface, NULL, SDL_MapRGB(SnapImgSurface->format, 0,0,0));
	string imgPath = CFGHelper::snapsPath +"\\"+ FileUtils::GetFileNameNoExt(gameName) +".png";
	
	SDL_Surface *temp = IMG_Load(imgPath.c_str());
	if(temp == NULL)
		return;

	destRect.w = temp->w;
	destRect.h = temp->h;
	SDL_BlitSurface(temp, NULL, SnapImgSurface, &destRect);

	//transfer the completed surface to the texture
	SDL_UpdateTexture(SnapTexture, NULL, SnapImgSurface->pixels, SnapImgSurface->pitch);
	
	curSnap = gameName;
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
			&pi
		)
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
	string n = FileUtils::GetFileNameNoExt(gameName);
	string command = CFGHelper::mamePath;
	string args = FileUtils::GetFileNameNoExt(gameName) + " -joystick";
	
	myCreateProcess(command, args, mameProc);
	DWORD dwProcessID = GetProcessId(mameProc);
	HANDLE hProcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);

	RegisterWaitForSingleObject(&mameHandle, hProcHandle, WaitOrTimerCallback, NULL, INFINITE, WT_EXECUTEONLYONCE);
}
//-----------------------------------------------------------------------------------------
HRESULT InitDirectInput(HWND hDlg)
{
	HRESULT hr;

	// Register with the DirectInput subsystem and get a pointer
	// to a IDirectInput interface we can use.
	// Create a DInput object
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION,
		IID_IDirectInput8, (VOID**)&g_pDI, nullptr)))
		return hr;




	DIJOYCONFIG PreferredJoyCfg = { 0 };
	DI_ENUM_CONTEXT enumContext;
	enumContext.pPreferredJoyCfg = &PreferredJoyCfg;
	enumContext.bPreferredJoyCfgValid = false;

	IDirectInputJoyConfig8* pJoyConfig = nullptr;
	if (FAILED(hr = g_pDI->QueryInterface(IID_IDirectInputJoyConfig8, (void**)&pJoyConfig)))
		return hr;

	PreferredJoyCfg.dwSize = sizeof(PreferredJoyCfg);
	if (SUCCEEDED(pJoyConfig->GetConfig(0, &PreferredJoyCfg, DIJC_GUIDINSTANCE))) // This function is expected to fail if no joystick is attached
		enumContext.bPreferredJoyCfgValid = true;
	SAFE_RELEASE(pJoyConfig);

	// Look for a simple joystick we can use for this sample program.
	if (FAILED(hr = g_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL,
		EnumJoysticksCallback,
		&enumContext, DIEDFL_ATTACHEDONLY)))
		return hr;


	// Make sure we got a joystick
	if (!g_pJoystick)
	{
		/*MessageBox(nullptr, TEXT("Joystick not found. The sample will now exit."),
			TEXT("DirectInput Sample"),
			MB_ICONERROR | MB_OK);
		EndDialog(hDlg, 0);*/
		return S_OK;
	}

	// Set the data format to "simple joystick" - a predefined data format 
	//
	// A data format specifies which controls on a device we are interested in,
	// and how they should be reported. This tells DInput that we will be
	// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
	if (FAILED(hr = g_pJoystick->SetDataFormat(&c_dfDIJoystick2)))
		return hr;

	// Set the cooperative level to let DInput know how this device should
	// interact with the system and with other DInput applications.
	if (FAILED(hr = g_pJoystick->SetCooperativeLevel(hDlg, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		//if( FAILED( hr = g_pJoystick->SetCooperativeLevel( hDlg, DISCL_EXCLUSIVE | DISCL_FOREGROUND ) ) )
		return hr;

	// Enumerate the joystick objects. The callback function enabled user
	// interface elements for objects that are found, and sets the min/max
	// values property for discovered axes.
	if (FAILED(hr = g_pJoystick->EnumObjects(EnumObjectsCallback,
		(VOID*)hDlg, DIDFT_ALL)))
		return hr;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: EnumJoysticksCallback()
// Desc: Called once for each enumerated joystick. If we find one, create a
//       device interface on it so we can play with it.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance,
	VOID* pContext)
{
	auto pEnumContext = reinterpret_cast<DI_ENUM_CONTEXT*>(pContext);
	HRESULT hr;


	// Skip anything other than the perferred joystick device as defined by the control panel.  
	// Instead you could store all the enumerated joysticks and let the user pick.
	if (pEnumContext->bPreferredJoyCfgValid &&
		!IsEqualGUID(pdidInstance->guidInstance, pEnumContext->pPreferredJoyCfg->guidInstance))
		return DIENUM_CONTINUE;

	// Obtain an interface to the enumerated joystick.
	hr = g_pDI->CreateDevice(pdidInstance->guidInstance, &g_pJoystick, nullptr);

	// If it failed, then we can't use this joystick. (Maybe the user unplugged
	// it while we were in the middle of enumerating it.)
	if (FAILED(hr))
		return DIENUM_CONTINUE;

	// Stop enumeration. Note: we're just taking the first joystick we get. You
	// could store all the enumerated joysticks and let the user pick.
	//return DIENUM_STOP;

	//get all the joysticks!
	return DIENUM_CONTINUE;
}

//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//       joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi,
	VOID* pContext)
{
	HWND hDlg = (HWND)pContext;

	static int nSliderCount = 0;  // Number of returned slider controls
	static int nPOVCount = 0;     // Number of returned POV controls

								  // For axes that are returned, set the DIPROP_RANGE property for the
								  // enumerated axis in order to scale min/max values.
	if (pdidoi->dwType & DIDFT_AXIS)
	{
		DIPROPRANGE diprg;
		diprg.diph.dwSize = sizeof(DIPROPRANGE);
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		diprg.diph.dwHow = DIPH_BYID;
		diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
		diprg.lMin = -1000;
		diprg.lMax = +1000;

		// Set the range for the axis
		if (FAILED(g_pJoystick->SetProperty(DIPROP_RANGE, &diprg.diph)))
			return DIENUM_STOP;

	}


	return DIENUM_CONTINUE;
}

//-----------------------------------------------------------------------------
// Name: UpdateInputState()
// Desc: Get the input device's state and display it.
//-----------------------------------------------------------------------------
HRESULT UpdateInputState()
{
	HRESULT hr;
	//TCHAR strText[512] = { 0 }; // Device state text
	DIJOYSTATE2 js;           // DInput joystick state 

	if (!g_pJoystick)
		return S_OK;

	// Poll the device to read the current state
	hr = g_pJoystick->Poll();
	if (FAILED(hr))
	{
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.
		hr = g_pJoystick->Acquire();
		while (hr == DIERR_INPUTLOST)
			hr = g_pJoystick->Acquire();

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 
		return S_OK;
	}

	// Get the input's device state
	if (FAILED(hr = g_pJoystick->GetDeviceState(sizeof(DIJOYSTATE2), &js)))
		return hr; // The device should have been acquired during the Poll()
	

	if (menuMode)
	{
		if (js.lY > 100)
		{
			mainMenu->Next(SDL_GetTicks());
			LoadCurrentSnapshot();
		}
		else if (js.lY < -100)
		{
			mainMenu->Prev(SDL_GetTicks());
			LoadCurrentSnapshot();
		}
		
		if (js.rgbButtons[0] & 0x80)
		{
			menuMode = false;
			//printf ("pushed button 1");
			string gameName = mainMenu->GetCurrentSelectedItem();
			LaunchGame(gameName);
			
		}

		else if (js.rgbButtons[1] & 0x80)
		{
			//this will be the back\cancel button
		}
	}
	//we are in the game, lets keep track of button presses
	else
	{

	}


	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: FreeDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
VOID FreeDirectInput()
{
	// Unacquire the device one last time just in case 
	// the app tried to exit while the device is still acquired.
	if (g_pJoystick)
		g_pJoystick->Unacquire();

	// Release any DirectInput objects.
	SAFE_RELEASE(g_pJoystick);
	SAFE_RELEASE(g_pDI);
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
				newGame.name = desc;
				newGame.manufacturer = manufacturer;
				newGame.year = year;
				string romPath = CFGHelper::romPath +"\\"+ romName+".zip";
				if (FileUtils::DoesPathExist(romPath))
				{
					AllGameListInfo.push_back(newGame);
					masterList << romName << "," << name << "," << manufacturer << "," << year << endl;
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
void FillGameListFromCSV( bool verify = true)
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
		vector<string> tokens = StringUtils::Tokenize(line,",");
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
	}
	
	masterList.close();
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