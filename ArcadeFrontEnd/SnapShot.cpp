#include "stdafx.h"

#include "SnapShot.h"

#include "StringUtils.h"
#include "FileUtils.h"
#include "CFGHelper.h"

using namespace std;

SnapShot::SnapShot()
{
	SnapImgSurface = NULL;
	SnapTexture = NULL;
	SnapImgRect;
	
	menu = NULL;
	renderer = NULL;
}
//-----------------------------------------------------------------------------------------
SnapShot::SnapShot(SDL_Renderer *r, Menu *m, int windowX, int windowY, int windowW, int windowH)
{
	/*SnapImgRect.w = (int)float((SCREEN_WIDTH / 3) * 2);
	SnapImgRect.h = SCREEN_HEIGHT / 2;
	SnapImgRect.x = ((SCREEN_WIDTH - ) / 2) + SnapImgRect.w / 4;
	SnapImgRect.y = (SnapImgRect.h / 2);*/

	SnapImgRect.x = windowX;
	SnapImgRect.y = windowY;
	SnapImgRect.w = windowH;
	SnapImgRect.h = windowW;

	giImgRect.x = 0;
	giImgRect.y = 0;
	giImgRect.w = windowW;
	giImgRect.h = 100;

	
	renderer = r;
	menu = m;
	unsigned int rmask = 0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000, amask = 0xff000000;
	SnapImgSurface = SDL_CreateRGBSurface(0, SnapImgRect.w, SnapImgRect.h, 32, rmask, gmask, bmask, amask);

	//this pixel format is STRANGE! but its the one that works with the mame snaps...
	SnapTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SnapImgRect.w, SnapImgRect.h);
}
//-----------------------------------------------------------------------------------------
SnapShot::~SnapShot()
{
	SDL_DestroyTexture(SnapTexture);
	SDL_FreeSurface(SnapImgSurface);
}
//-----------------------------------------------------------------------------------------
void SnapShot::LoadCurrentSnapshot()
{
	if(menu == NULL)
		return;

	GameInfo game = menu->GetCurrentSelectedItem();

	if (curGamneInfo.id == game.id)
		return;


	//first get the snap shot
	string imgPath = CFGHelper::snapsPath + "\\" + FileUtils::GetFileNameNoExt(game.romName) + ".png";

	SDL_Surface *temp = IMG_Load(imgPath.c_str());
	if (temp != NULL)
	{
		SDL_FillRect(SnapImgSurface, NULL, 0x000000);
		SDL_BlitSurface(temp, NULL, SnapImgSurface, NULL);
		//transfer the completed surface to the texture
		SDL_UpdateTexture(SnapTexture, NULL, SnapImgSurface->pixels, SnapImgSurface->pitch);
		giImgRect.y = temp->h+1;
		
		SDL_FreeSurface(temp);
	}


	//now get the game info
	buffer = new PIXMAP(giImgRect.w, giImgRect.h);

	font.Draw(buffer, game.name, 0, 0);
	font.Draw(buffer, game.manufacturer, 0, 11);
	font.Draw(buffer, game.genre, 0, 22);
	font.Draw(buffer, game.year, 0, 33);
	font.Draw(buffer, "num credits spent: " + to_string(game.numCredits), 0, 44);
	font.Draw(buffer, "total Play time: " + GetReadableTimeFromMilis(game.totalPlayTime), 0, 55);
	font.Draw(buffer, "last played on: "+ game.lastPlayedDate, 0, 66);


	//transfer the completed surface to the texture
	SDL_UpdateTexture(SnapTexture, &giImgRect, buffer->pixels, buffer->w * sizeof(Uint32));

	curGamneInfo = game;
}
//-----------------------------------------------------------------------------------------
string SnapShot::GetReadableTimeFromMilis(unsigned int milis)
{
	int seconds = (int)(milis / 1000) % 60;
	int minutes = (int)((milis / (1000 * 60)) % 60);
	int hours = (int)((milis / (1000 * 60 * 60)) % 24);
	string ret;

	if (hours > 0)
	{
		ret += to_string(hours);
		if(hours > 1) 
			ret += " hours ";
		else
			ret += " hour ";
	}

	if (minutes > 0)
	{
		ret += to_string(minutes);
		if (minutes > 1)
			ret += " minutes ";
		else
			ret += " minute ";
	}

	if (seconds > 0)
	{
		ret += " and " + to_string(minutes);
		if (seconds > 1)
			ret += " seconds";
		else
			ret += " second";
	}

	return ret;
}
//-----------------------------------------------------------------------------------------
void SnapShot::Draw()
{
	SDL_RenderCopy(renderer, SnapTexture, NULL, &SnapImgRect);
	//SDL_RenderCopy(renderer, giTexture, NULL, &giImgRect);
}