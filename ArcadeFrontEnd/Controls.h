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



struct JoysticFlags
{
	unsigned int joystick;
	unsigned int buttons;

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
};

//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
BOOL CALLBACK    EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext);
BOOL CALLBACK    EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=nullptr; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=nullptr; } }
LPDIRECTINPUT8          g_pDI = nullptr;

//need an array of these to check for button presses in game from all joysticks
LPDIRECTINPUTDEVICE8    g_pJoystick = nullptr;

JoysticFlags P1Controls;
JoysticFlags P2Controls;
JoysticFlags P3Controls;
JoysticFlags P4Controls;


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
	DIJOYSTATE2 joystick1;           // DInput joystick state 
	P1Controls.ClearFlags();

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
	if (FAILED(hr = g_pJoystick->GetDeviceState(sizeof(DIJOYSTATE2), &joystick1)))
		return hr; // The device should have been acquired during the Poll()

	

	if (joystick1.lY < -100)
		P1Controls.AddToJoystickFlag(JOY_UP);
	else if (joystick1.lY > 100)
		P1Controls.AddToJoystickFlag(JOY_DOWN);
	
	if (joystick1.lX < -100)
		P1Controls.AddToJoystickFlag(JOY_LEFT);
	else if (joystick1.lX > 100)
		P1Controls.AddToJoystickFlag(JOY_RIGHT);


	for (int i = 0; i < 10; i++)
	{
		if (joystick1.rgbButtons[i] & 0x80)
		{
			P1Controls.AddToButtonFlag(1 << i);
		}
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