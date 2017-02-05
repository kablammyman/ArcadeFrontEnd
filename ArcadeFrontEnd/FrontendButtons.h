#pragma once

struct FRONTEND_BUTTON
{
	int joystick;
	int button;

	void SetButton(int j, int b)
	{
		joystick = j;
		button = b;
	}
};

//how can i do a combo of buttons?
struct FrontEndControls
{
	FRONTEND_BUTTON MAIN_JOYSTICK;
	FRONTEND_BUTTON SELECT;
	FRONTEND_BUTTON SKIP;
	FRONTEND_BUTTON OPTIONS;
	FRONTEND_BUTTON COIN1;
	FRONTEND_BUTTON COIN2;
	FRONTEND_BUTTON COIN3;
	FRONTEND_BUTTON COIN4;
	FRONTEND_BUTTON START1;
	FRONTEND_BUTTON START2;
	FRONTEND_BUTTON START3;
	FRONTEND_BUTTON START4;

};
extern FrontEndControls feControls;