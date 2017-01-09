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
	
	curSnap = "";
	menu = NULL;
	renderer = NULL;
}
//-----------------------------------------------------------------------------------------
SnapShot::SnapShot(SDL_Renderer *r, Menu *m, int windowX, int windowY, int windowW, int windowH)
{
	SnapImgRect.x = windowX;
	SnapImgRect.y = windowY;
	SnapImgRect.w = windowH;
	SnapImgRect.h = windowW;
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
	string romName = menu->GetCurrentSelectedItemRomName();


	if (curSnap == romName)
		return;

	string imgPath = CFGHelper::snapsPath + "\\" + FileUtils::GetFileNameNoExt(romName) + ".png";

	SDL_Surface *temp = IMG_Load(imgPath.c_str());
	if (temp == NULL)
		return;

	SDL_FillRect(SnapImgSurface, NULL, 0x000000);
	SDL_BlitSurface(temp, NULL, SnapImgSurface, NULL);

	//transfer the completed surface to the texture
	SDL_UpdateTexture(SnapTexture, NULL, SnapImgSurface->pixels, SnapImgSurface->pitch);

	SDL_FreeSurface(temp);
	curSnap = romName;
}
//-----------------------------------------------------------------------------------------
void SnapShot::Draw()
{
	SDL_RenderCopy(renderer, SnapTexture, NULL, &SnapImgRect);
}