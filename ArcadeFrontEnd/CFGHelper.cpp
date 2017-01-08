#include "stdafx.h"
#include "CFGHelper.h"
#include "CFGUtils.h"


string CFGHelper::filePathBase;
string CFGHelper::dbPath;
string CFGHelper::romPath;
string CFGHelper::mameList;
string CFGHelper::mamePath;
string CFGHelper::snapsPath;
string CFGHelper::mameini;
vector<string> CFGHelper::attractPlayList;


std::string CFGHelper::SetProgramPath(std::string argv)
{
	//MAX_PATH???
	char full[260];
	if (!argv.empty())
	{
		std::string temp = _fullpath(full, argv.c_str(), 260);
		size_t found = temp.find_last_of("/\\");
		return temp.substr(0, found);
	}
	else
	{
		std::string temp = _fullpath(full, argv.c_str(), 260);
		return temp;
	}
}


bool CFGHelper::LoadCFGFile(string programBasePath)
{
	string cfgPath;
	if (programBasePath.empty())
		cfgPath = CFGHelper::filePathBase + "\\frontendCfg.txt";
	else
		cfgPath = programBasePath + "\\frontendCfg.txt";

	if (!CFGUtils::ReadCfgFile(cfgPath, '|'))
	{
		string errorMsg = "Error opening :";
		errorMsg += cfgPath;
		//cout << errorMsg << "\nno cfg text file" << endl;
		return false;
	}

	dbPath = CFGUtils::GetCfgStringValue("DBPath");
	if (dbPath == "")
	{
		dbPath = CFGHelper::filePathBase + "\\ArcadeFrontend.db";
	}

	mameList = CFGUtils::GetCfgStringValue("MAMELIST");
	romPath = CFGUtils::GetCfgStringValue("ROMS");
	mamePath = CFGUtils::GetCfgStringValue("MAME");
	mameini = CFGUtils::GetCfgStringValue("MAMEINI");
	snapsPath = CFGUtils::GetCfgStringValue("SNAP");
	return true;
}
//check to make sure everything loaded
bool CFGHelper::IsCFGComplete(string &err)
{
	
	if (romPath == "")
	{
		err = "rom path is missing in the cfg";
		return false;
	}
	if (mamePath == "")
	{
		err = "mamePath is missing in the cfg";
		return false;
	}
	if (mameini == "")
	{
		err = "mame ini path is missing in the cfg";
		return false;
	}
	
	return true;
}