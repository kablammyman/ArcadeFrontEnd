#pragma once

#include <string>
#include <Windows.h>

#include "MainScreen.h"
BOOL CALLBACK	 EnumWindowsProc(HWND hwnd, LPARAM param);
VOID CALLBACK WaitOrTimerCallback(_In_  PVOID lpParameter, _In_  BOOLEAN TimerOrWaitFired);
bool myCreateProcess(const char * pathAndName, const char * args, HANDLE &processCreated);
void LaunchGame(std::string gameName,std::string exeName);

extern HWND mameWindowHandle;
extern HANDLE mameProc;
extern HANDLE mameHandle;
extern HWND mainWindowHandle;
extern MainScreen *mainscreenPointer;