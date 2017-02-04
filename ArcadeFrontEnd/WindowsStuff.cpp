#include "stdafx.h"
#include"WindowsStuff.h"
#include "MainScreen.h"
#include "CFGHelper.h"

HWND mameWindowHandle = NULL;
HANDLE mameProc = NULL;
HANDLE mameHandle = NULL;
HWND mainWindowHandle = NULL;
MainScreen *mainscreenPointer = NULL;

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
bool myCreateProcess(const char * pathAndName, const char * args, HANDLE &processCreated)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	ZeroMemory(&pi, sizeof(pi));
	if (!
		CreateProcessA(
			pathAndName,
			(LPSTR)args,
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
void LaunchGame(std::string gameName, std::string exeName)
{


	std::string command = CFGHelper::mamePath;
	//without the exe name as the first param, CreateProcess doesnt work right....CreateProcess allows you to override
	//nornmal cmd behavior for some dumb ass reason!
	std::string args = exeName + " " +
		gameName + " -rp \"" + CFGHelper::romPath + "\" -inipath \"" + CFGHelper::mameini + "\" -joystick";


	myCreateProcess(command.c_str(), args.c_str(), mameProc);
	DWORD dwProcessID = GetProcessId(mameProc);
	HANDLE hProcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);



	RegisterWaitForSingleObject(&mameHandle, hProcHandle, WaitOrTimerCallback, NULL, INFINITE, WT_EXECUTEONLYONCE);
	Sleep(500);
	if (!EnumWindows(EnumWindowsProc, dwProcessID) && (GetLastError() == ERROR_SUCCESS)) {
		//nopw that we have our main window, lets find child window
		if (!EnumChildWindows(mameWindowHandle, EnumWindowsProc, 0) && (GetLastError() == ERROR_SUCCESS))
		{

		}
	}


}

//-----------------------------------------------------------------------------------------
VOID CALLBACK WaitOrTimerCallback(
	_In_  PVOID lpParameter,
	_In_  BOOLEAN TimerOrWaitFired
)
{
	//MessageBox(0, L"The process has exited.", L"INFO", MB_OK);
	SDL_RestoreWindow(MainScreen::window);
	mainscreenPointer->ReturnFromGame();
	return;
}