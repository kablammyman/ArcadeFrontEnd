#pragma once

#include <string>

struct GameInfo
{
	std::string name;
	std::string romName;
	std::string manufacturer;
	std::string year;
	std::string genre;
	int id;

	static bool sortFn(GameInfo &i, GameInfo &j) { return (i.name<j.name); }
};