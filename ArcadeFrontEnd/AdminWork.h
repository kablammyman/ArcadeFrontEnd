#pragma once
//all of the methods and stuff that helps clean and organize the db
//its in its own class so theres no screen afflaited with it
#include <string>
#include <vector>
#include <thread>

#include "Observee.h"

#include "SQLiteUtils.h"
#include "GameInfo.h"


class AdminWork : public Observee
{
	SQLiteUtils *db;
	std::vector<GameInfo> *AllGameListInfo;

	std::thread *backgroundWork;

	void GenerateGameList(std::string mameListPath, std::string catverListPath);
	void FillGameListFromDB(bool verify = false);
	bool IsStringInVector(std::vector<std::string> & list, std::string keyword);
	void RemoveCrappyROMS(bool deleteZip = false);
	
	void Test(bool something);

public:
	std::string tableName;
	AdminWork(std::vector<GameInfo> *a, SQLiteUtils *d);
	
	void AdminWork::DoGenerateGameList();
	void AdminWork::DoFillGameList();
	void AdminWork::DoRemoveCrappyRoms();

	void AdminWork::DoTest();
};