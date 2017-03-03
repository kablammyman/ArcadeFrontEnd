#pragma once

#include <string>
#include <vector>
#include "FrontendButtons.h"

using namespace std;
struct FrontEndControls;

//when we need to use a cfg file, we always have these vars and shit, so i made it easy for each project to use the same code
class CFGHelper
{
public:
	static string filePathBase;
	static string dbPath;
	static string romPath;
	static string snapsPath;
	static string mamePath;
	static string mameList;
	static string mameini;
	static string catverPath;
	static vector<string> attractPlayList;

	static string MAIN_JOYSTICK;
	static string	MAIN_SELECT;
	static string	MAIN_SKIP;
	static string	MAIN_OPTIONS;
	static string	COIN1;
	static string	COIN2;
	static string	COIN3;
	static string	COIN4;

	static bool LoadCFGFile(string programBasePath = "");
	static void LoadFrontEndControls(FrontEndControls *fec);
	static bool IsCFGComplete(string &err);
	static string SetProgramPath(std::string argv);
};