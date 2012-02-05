#include "Location.h"
#include "Npc.h"
#include "Player.h"
#include "Item.h"
#include "MapObject.h"
#pragma once
using namespace std;

class Universe
{
public:
	Location* currentLocation;
	Location** locations;
	vector<Npc*> npcs;
	vector<Player*> players;
	vector<Item*> items;
	vector<MapObject*> mapObjects;
	int cameraX, cameraY; //pixels
	int cursorX, cursorY; //points
	int screenWidth, screenHeight; //pixels
	int toolbarWidth; //pixels
	int cellSize; //pixels
	int locationsCount;
	CellProperty currentBrush;
	char* gameName;
	gcn::Gui* toolbar;
	
	bool GraphicsInit();
	bool GUIInit(gcn::SDLInput* &GUIInput);
	bool LocationsInit();
	void SelectLocation(Location* location);
	void DrawScene();
	void Run();
	void CameraMove(int x, int y);
	int Pix2Index(int pos);
	int Index2Pix(int pos);
	int PixRound(int pos);
	
	Universe(void);
	~Universe(void);
};
