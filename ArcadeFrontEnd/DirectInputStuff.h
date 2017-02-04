#pragma once
#include <Windows.h>
#pragma warning(push)
#pragma warning(disable:6000 28251)
#include <dinput.h>
#pragma warning(pop)

#include <dinputd.h>

#include "Input.h"


#define MAX_JOYSTICKS 10
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=nullptr; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=nullptr; } }


struct Joy
{
	LPDIRECTINPUTDEVICE8    g_pJoystick = nullptr;
	Input PlayerControls;
	DIJOYSTATE2 state;// DInput joystick state  for update input
};
struct DI_ENUM_CONTEXT
{
	DIJOYCONFIG* pPreferredJoyCfg;
	bool bPreferredJoyCfgValid;
};

HRESULT InitDirectInput(HWND hDlg);
VOID FreeDirectInput();
HRESULT UpdateInputState();
BOOL CALLBACK    EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext);
BOOL CALLBACK    EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);

// DirectInput Variables
extern LPDIRECTINPUT8 fDI; // Root DirectInput Interface
extern LPDIRECTINPUTDEVICE8 fDIKeyboard; // The keyboard device
extern unsigned char fDIKeyboardState[256];

extern Joy joystick[MAX_JOYSTICKS];
extern int enumCurJoystickIndex;
extern int curIndex;
extern  LPDIRECTINPUT8   g_pDI;