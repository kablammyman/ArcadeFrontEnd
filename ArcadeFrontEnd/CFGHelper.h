#pragma once

#include <string>
#include <vector>

using namespace std;

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

	static vector<string> attractPlayList;

	static bool LoadCFGFile(string programBasePath = "");

	static bool IsCFGComplete(string &err);
	static string SetProgramPath(std::string argv);
};