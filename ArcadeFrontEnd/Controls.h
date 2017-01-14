#pragma once
#include <Windows.h>
#pragma warning(push)
#pragma warning(disable:6000 28251)
#include <dinput.h>
#pragma warning(pop)

#include <dinputd.h>

#define JOY_UP    1
#define JOY_DOWN  2
#define JOY_LEFT  4
#define JOY_RIGHT 8

#define BUTTON1 1
#define BUTTON2 2
#define BUTTON3 4
#define BUTTON4 8
#define BUTTON5 16
#define BUTTON6 32
#define BUTTON_COIN 64
#define BUTTON_START 128

#define MAX_JOYSTICKS 10

struct JoysticFlags
{
	unsigned int joystick;
	unsigned int buttons;
	unsigned int lastJoyState;
	unsigned int lastButtonState;
	bool b2pressed;
	inline void ClearFlags()
	{
		joystick =0;
		buttons = 0;
	}

	inline void AddToJoystickFlag(unsigned int flag)
	{
		joystick = joystick| flag;
	}
	inline void AddToButtonFlag(unsigned int flag)
	{
		buttons = buttons | flag;
	}
	inline bool CheckJoystickFlag(unsigned int flag)
	{
		return (bool)(joystick & flag);
	}
	inline bool CheckButtonFlag(unsigned int flag)
	{
		return (bool)(buttons & flag);
	}
	inline bool CheckForAllInputFlags()
	{
		return (bool)(joystick > 0) ||(buttons > 0);
	}
	inline bool CheckButtonReleaseFlag(unsigned int flag)
	{

		if (lastButtonState & flag)
		{
			if (!(buttons & flag))
			{
				lastButtonState &= ~flag;
				return true;
			}
		}
		return false;

	}
};

// DirectInput Variables
LPDIRECTINPUT8 fDI; // Root DirectInput Interface
LPDIRECTINPUTDEVICE8 fDIKeyboard; // The keyboard device
unsigned char fDIKeyboardState[256];

struct Joy
{
	LPDIRECTINPUTDEVICE8    g_pJoystick = nullptr;
	JoysticFlags PlayerControls;
	DIJOYSTATE2 state;// DInput joystick state  for update input
};

Joy joystick[MAX_JOYSTICKS];
int enumCurJoystickIndex = 0;
int curIndex = 0;
//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
BOOL CALLBACK    EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext);
BOOL CALLBACK    EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=nullptr; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=nullptr; } }
LPDIRECTINPUT8          g_pDI = nullptr;



struct DI_ENUM_CONTEXT
{
	DIJOYCONFIG* pPreferredJoyCfg;
	bool bPreferredJoyCfgValid;
};

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
	//keyboard inpout...mostly used for testing 
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION,
		IID_IDirectInput8, (VOID**)&fDI, nullptr)))
		return hr;

	// Create the connection to the keyboard device
	fDI->CreateDevice(GUID_SysKeyboard, &fDIKeyboard, NULL);


	if (fDIKeyboard)
	{
		fDIKeyboard->SetDataFormat(&c_dfDIKeyboard);
		fDIKeyboard->SetCooperativeLevel(hDlg, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
		fDIKeyboard->Acquire();
	}
	else
	{
		/*MessageBox(nullptr, TEXT("DirectInput Keyboard initialization Failed!"),
		TEXT("DirectInput Sample"),
		MB_ICONERROR | MB_OK);
		EndDialog(hDlg, 0);*/
	}

	DIJOYCONFIG PreferredJoyCfg = { 0 };
	DI_ENUM_CONTEXT enumContext;
	enumContext.pPreferredJoyCfg = &PreferredJoyCfg;
	enumContext.bPreferredJoyCfgValid = false;

	IDirectInputJoyConfig8* pJoyConfig = nullptr;
	if (FAILED(hr = g_pDI->QueryInterface(IID_IDirectInputJoyConfig8, (void**)&pJoyConfig)))
		return hr;

	PreferredJoyCfg.dwSize = sizeof(PreferredJoyCfg);
	if (SUCCEEDED(pJoyConfig->GetConfig(0, &PreferredJoyCfg, DIJC_GUIDINSTANCE))) // This function is expected to fail if no joystick is attached
		enumContext.bPreferredJoyCfgValid = false; //we dont want to skip any joysticks
	SAFE_RELEASE(pJoyConfig);

	// Look for a simple joystick we can use for this sample program.
	if (FAILED(hr = g_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL,
		EnumJoysticksCallback,
		&enumContext, DIEDFL_ATTACHEDONLY)))
		return hr;


	// Make sure we got a joystick
	if (!joystick[0].g_pJoystick)
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
	for (int i = 0; i < enumCurJoystickIndex; i++)
	{
		if (FAILED(hr = joystick[i].g_pJoystick->SetDataFormat(&c_dfDIJoystick2)))
			return hr;

		// Set the cooperative level to let DInput know how this device should
		// interact with the system and with other DInput applications.
		if (FAILED(hr = joystick[i].g_pJoystick->SetCooperativeLevel(hDlg, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
			//if( FAILED( hr = g_pJoystick->SetCooperativeLevel( hDlg, DISCL_EXCLUSIVE | DISCL_FOREGROUND ) ) )
			return hr;

		// Enumerate the joystick objects. The callback function enabled user
		// interface elements for objects that are found, and sets the min/max
		// values property for discovered axes.
		
		//ugh, stupid call backs! i gotta do this hack!
		curIndex = i;
		if (FAILED(hr = joystick[i].g_pJoystick->EnumObjects(EnumObjectsCallback, (VOID*)hDlg, DIDFT_ALL)))
			return hr;
	}

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
	hr = g_pDI->CreateDevice(pdidInstance->guidInstance, &joystick[enumCurJoystickIndex].g_pJoystick, nullptr);
	
	
	// If it failed, then we can't use this joystick. (Maybe the user unplugged
	// it while we were in the middle of enumerating it.)
	if (FAILED(hr))
		return DIENUM_CONTINUE;
	else
		enumCurJoystickIndex++;

	//we want all the joysticks!
	//return DIENUM_STOP;

	return DIENUM_CONTINUE;
}

//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//       joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi,VOID* pContext)
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
		if (FAILED(joystick[curIndex].g_pJoystick->SetProperty(DIPROP_RANGE, &diprg.diph)))
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
	hr = fDIKeyboard->GetDeviceState(sizeof(fDIKeyboardState),(LPVOID)&fDIKeyboardState);
	
	if (fDIKeyboardState[DIK_5] & 0x80)
	{
		while (fDIKeyboardState[DIK_5] & 0x80) {}
		//BUTTON_COIN
		//player 1 coin
	}
	if (fDIKeyboardState[DIK_6] & 0x80)
	{
		while (fDIKeyboardState[DIK_6] & 0x80) {}
		//player 2 coin
	}

	for (int i = 0; i < enumCurJoystickIndex; i++)
	{
		joystick[i].PlayerControls.ClearFlags();

		if (!joystick[i].g_pJoystick)
			return S_OK;

		// Poll the device to read the current state
		hr = joystick[i].g_pJoystick->Poll();
		if (FAILED(hr))
		{
			// DInput is telling us that the input stream has been
			// interrupted. We aren't tracking any state between polls, so
			// we don't have any special reset that needs to be done. We
			// just re-acquire and try again.
			hr = joystick[i].g_pJoystick->Acquire();
			while (hr == DIERR_INPUTLOST)
				hr = joystick[i].g_pJoystick->Acquire();

			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
			// may occur when the app is minimized or in the process of 
			// switching, so just try again later 
			return S_OK;
		}

		// Get the input's device state

		if (FAILED(hr = joystick[i].g_pJoystick->GetDeviceState(sizeof(DIJOYSTATE2), &joystick[i].state) && i == 0))
			return hr; // The device should have been acquired during the Poll()

		if (joystick[i].state.lY < -100)
			joystick[i].PlayerControls.AddToJoystickFlag(JOY_UP);
		else if (joystick[i].state.lY > 100)
			joystick[i].PlayerControls.AddToJoystickFlag(JOY_DOWN);

		if (joystick[i].state.lX < -100)
			joystick[i].PlayerControls.AddToJoystickFlag(JOY_LEFT);
		else if (joystick[i].state.lX > 100)
			joystick[i].PlayerControls.AddToJoystickFlag(JOY_RIGHT);


		for (int j = 0; j < 10; j++)
		{
			if (joystick[i].state.rgbButtons[j] & 0x80)
			{
				joystick[i].PlayerControls.AddToButtonFlag(1 << j);
				joystick[i].PlayerControls.lastButtonState = joystick[i].PlayerControls.buttons;
				//if(j == 1) 
				//	joystick[i].PlayerControls.b2pressed = true;
			}
		}

		joystick[i].PlayerControls.lastJoyState = joystick[i].PlayerControls.joystick;
		
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
	for (int i = 0; i < enumCurJoystickIndex; i++)
	{
		if (joystick[i].g_pJoystick)
			joystick[i].g_pJoystick->Unacquire();
		SAFE_RELEASE(joystick[i].g_pJoystick);
	}
	// Release any DirectInput objects.
	SAFE_RELEASE(g_pDI);
}