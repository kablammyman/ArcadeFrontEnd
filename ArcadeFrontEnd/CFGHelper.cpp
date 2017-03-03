#include "stdafx.h"
#include "CFGHelper.h"
#include "CFGUtils.h"
#include "StringUtils.h"


string CFGHelper::filePathBase;
string CFGHelper::dbPath;
string CFGHelper::romPath;
string CFGHelper::mameList;
string CFGHelper::mamePath;
string CFGHelper::snapsPath;
string CFGHelper::mameini;
string CFGHelper::catverPath;

 string CFGHelper::MAIN_JOYSTICK;
 string	CFGHelper::MAIN_SELECT;
 string	CFGHelper::MAIN_SKIP;
 string	CFGHelper::MAIN_OPTIONS;
 string	CFGHelper::COIN1;
 string	CFGHelper::COIN2;
 string	CFGHelper::COIN3;
 string	CFGHelper::COIN4;

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
	catverPath = CFGUtils::GetCfgStringValue("CATVER");
	
	MAIN_JOYSTICK = CFGUtils::GetCfgStringValue("MAIN_JOYSTICK");
	MAIN_SELECT = CFGUtils::GetCfgStringValue("MAIN_SELECT");
	MAIN_SKIP = CFGUtils::GetCfgStringValue("MAIN_SKIP");
	MAIN_OPTIONS = CFGUtils::GetCfgStringValue("MAIN_OPTIONS");
	COIN1 = CFGUtils::GetCfgStringValue("COIN1");
	COIN2 = CFGUtils::GetCfgStringValue("COIN2");
	COIN3 = CFGUtils::GetCfgStringValue("COIN3");
	COIN4 = CFGUtils::GetCfgStringValue("COIN4");

	return true;
}
void CFGHelper::LoadFrontEndControls(FrontEndControls *fec)
{
	int joyIndex = stoi(MAIN_JOYSTICK);
	fec->MAIN_JOYSTICK.joystick = joyIndex;

	fec->SELECT.joystick = joyIndex;
	fec->SELECT.button = stoi(MAIN_SELECT);

	fec->SKIP.joystick = joyIndex;
	fec->SKIP.button = stoi(MAIN_SKIP);
	
	fec->OPTIONS.joystick = joyIndex;
	fec->OPTIONS.button = stoi(MAIN_OPTIONS);

	vector<string> temp = StringUtils::Tokenize(COIN1,",");
	fec->COIN1.joystick = stoi(temp[0]);
	fec->COIN1.button = stoi(temp[1]);
	temp.clear();

	temp = StringUtils::Tokenize(COIN2, ",");
	fec->COIN2.joystick = stoi(temp[0]);
	fec->COIN2.button = stoi(temp[1]);
	temp.clear();

	temp = StringUtils::Tokenize(COIN3, ",");
	fec->COIN3.joystick = stoi(temp[0]);
	fec->COIN3.button = stoi(temp[1]);
	temp.clear();

	temp = StringUtils::Tokenize(COIN4, ",");
	fec->COIN4.joystick = stoi(temp[0]);
	fec->COIN4.button = stoi(temp[1]);

	
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