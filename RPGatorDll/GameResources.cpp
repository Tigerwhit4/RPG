#include "StdAfx.h"
#include "ForwardDeclaration.h"
#include "SqliteResult.h"
#include "utilities.h"
#include "Render.h"
#include "GameObject.h"
#include "MapObject.h"
#include "MapCell.h"
#include "NPC.h"
#include "Item.h"
#include "Static.h"
#include "Character.h"
#include "Quest.h"
#include "Skill.h"
#include "Game.h"
#include "GameResources.h"

template<class T>
T** GameResources::FilterByTag(T** mapObjects, int mapObjectsCounter, char** tags, int numberOfTags)
{
	int i, j, k;
	T** result;
	int resultSize = 0;
	bool flag = true;

	for (i = 0; i < mapObjectsCounter; i++)
	{
		for (k = 0; k < numberOfTags; k++)	
		{
			flag = false;
			for (j = 0; j < mapObjects[i]->tagsCount; j++)
			{
				if (strcmp(tags[k], mapObjects[i]->tags[j]) != 0)
				{
					flag = true;
					break;
				}
			}

			if (!flag) break;
		}

		if (flag)
		{
			result = (T**)realloc(mapobjects, (++resultSize) * sizeof(T));
			result[resultSize - 1] = mapObjects[i];
		}
	}

	return result;
}

GameResources::GameResources(InitializationType initializationType)
{
	MapObjectsInit<MapCell>(mapCells, mapCellsCount, "MapCell", initializationType);
	MapObjectsInit<NPC>(npcs, npcsCount, "NPC", initializationType);
	MapObjectsInit<Item>(items, itemsCount, "Item", initializationType);
	MapObjectsInit<Static>(statics, staticsCount, "Static", initializationType);
	MapObjectsInit<Character>(characters, charactersCount, "Character", initializationType);
	//GameObjectsInit<Quest>(quests, questsCount, "Quest", initializationType);
	//GameObjectsInit<Skill>(skills, skillsCount, "Skill", initializationType);
	MapObjectsInit<Quest>(quests, questsCount, "Quest", initializationType);
	MapObjectsInit<Skill>(skills, skillsCount, "Skill", initializationType);
}

GameResources::~GameResources(void)
{
	int i;

	for (i = 0; i < mapCellsCount; i++)
		delete mapCells[i];
	delete mapCells;
	for (i = 0; i < npcsCount; i++)
		delete npcs[i];
	delete npcs;
	for (i = 0; i < itemsCount; i++)
		delete items[i];
	delete items;
	for (i = 0; i < staticsCount; i++)
		delete statics[i];
	delete statics;
	for (i = 0; i < charactersCount; i++)
		delete characters[i];
	delete characters;
	for (i = 0; i < questsCount; i++)
		delete quests[i];
	delete quests;
	for (i = 0; i < skillsCount; i++)
		delete skills[i];
	delete skills;
}

template<class T> //T inherits MapObject
void GameResources::MapObjectsInit(T** &mapObjects, int &mapObjectsCount, char* tableName, InitializationType initializationType)
{
	char query[64];
	char* path;
	std::vector<SqliteResult> sqliteResults;
	
	sprintf(query, "SELECT * FROM `%s`;", tableName); //TODO: Get class T name
	sqliteResults = SqliteGetRows(Game::instance->db, query);
	
	mapObjectsCount = 0;
	mapObjects = NULL;

	switch (initializationType)
	{
		case Editor:
			path = new char[256];
			sprintf(path, "editor/%s/model/%s", Game::instance->name, tableName);
			break;
		case Client:
			path = new char[256];
			sprintf(path, "client/%s/model/%s", Game::instance->name, tableName);
			break;
		case Server:
			path = NULL;
			break;
	}
	
	int rowsCount = sqliteResults.size();
	while (mapObjectsCount < rowsCount)
	{
		mapObjectsCount++;
		mapObjects = (T**)realloc(mapObjects, mapObjectsCount * sizeof(T*));
		mapObjects[mapObjectsCount - 1] = new T(sqliteResults[mapObjectsCount - 1], path);
	}

	if (path)
		delete path;
}
/*
template<class T> //T inherits MapObject
void GameResources::GameObjectsInit(T** &mapObjects, int &mapObjectsCount, char* tableName, InitializationType initializationType)
{
	char query[64];
	std::vector<SqliteResult> sqliteResults;
	
	sprintf(query, "SELECT * FROM %s;", tableName); //TODO: Get class T name
	sqliteResults = SqliteGetRows(Game::instance->db, query);
	
	mapObjectsCount = 0;
	mapObjects = NULL;

	int rowsCount = sqliteResults.size();
	while (mapObjectsCount < rowsCount)
	{
		mapObjectsCount++;
		mapObjects = (T**)realloc(mapObjects, mapObjectsCount * sizeof(T*));
		mapObjects[mapObjectsCount - 1] = new T(sqliteResults[mapObjectsCount - 1]);
	}
}
*/
int GameResources::GetMapObjectsTags(MapObject** mapObjects, int mapObjectsCount, char** &tags)
{
	int tagsCount;
	int i, j;

	tagsCount = 0;
	tags = NULL;
	for (i = 0; i < mapObjectsCount; i++)
	{
		for (j = 0; j < mapObjects[i]->tagsCount; j++)
		{
			if (std::find(tags, tags + tagsCount, mapObjects[i]->tags[j]) == (tags + tagsCount))
			{
				tagsCount++;
				tags = (char**)realloc(tags, tagsCount * sizeof(char*));
				tags[tagsCount - 1] = mapObjects[i]->tags[j];
			}
		}
	}
	return tagsCount;
}

MapCell* GameResources::GetMapCell(int id)
{
	return GetMapObject<MapCell>(mapCells, mapCellsCount, id);
}

NPC* GameResources::GetNPC(int id)
{
	return GetMapObject<NPC>(npcs, npcsCount, id);
}

Static* GameResources::GetStatic(int id)
{
	return GetMapObject<Static>(statics, staticsCount, id);
}

Item* GameResources::GetItem(int id)
{
	return GetMapObject<Item>(items, itemsCount, id);
}

Character* GameResources::GetCharacter(int id)
{
	return GetMapObject<Character>(characters, charactersCount, id);
}

Skill* GameResources::GetSkill(int id)
{
	return GetMapObject<Skill>(skills, skillsCount, id);
}

NPC* GameResources::AddNPC(char* name, char* tags, char* modelPath, char* texturePath)
{
	char query[256];

	sprintf(query, "INSERT INTO NPC(name, tags, scale) VALUES('%s', '%s', 3)", name, tags);
	sqlite3_exec(Game::instance->db, query, NULL, NULL, NULL);

	return AddMapObject<NPC>(npcs, npcsCount, "NPC", modelPath);
}

Static* GameResources::AddStatic(char* name, char* tags, char* modelPath, char* texturePath)
{
	char query[256];

	sprintf(query, "INSERT INTO Static(name, tags, scale) VALUES('%s', '%s', 3)", name, tags);
	sqlite3_exec(Game::instance->db, query, NULL, NULL, NULL);

	return AddMapObject<Static>(statics, staticsCount, "Static", modelPath);
}

Item* GameResources::AddItem(char* name, char* tags, char* modelPath, char* texturePath)
{
	char query[256];

	sprintf(query, "INSERT INTO Item(name, tags, scale) VALUES('%s', '%s', 3)", name, tags);
	sqlite3_exec(Game::instance->db, query, NULL, NULL, NULL);

	return AddMapObject<Item>(items, itemsCount, "Item", modelPath);
}

Character* GameResources::AddCharacter(char* name, char* tags, char* modelPath, char* texturePath)
{
	char query[256];

	sprintf(query, "INSERT INTO Character(name, tags, scale) VALUES('%s', '%s', 3)", name, tags);
	sqlite3_exec(Game::instance->db, query, NULL, NULL, NULL);

	return AddMapObject<Character>(characters, charactersCount, "Character", modelPath);
}

template<class T>
void GameResources::SpawnMapObject(T** &mapObjects, int &mapObjectsCount, T* mapObject)
{
	mapObjectsCount++;
	mapObjects = (T**)realloc(mapObjects, mapObjectsCount * sizeof(T*));
	mapObjects[mapObjectsCount - 1] = mapObject;
}

template<class T>
T* GameResources::AddMapObject(T** &mapObjects, int &mapObjectsCount, char* tableName, char* modelPath)
{
	char path[256];
	char query[256];

	sprintf(query, "SELECT * FROM `%s` WHERE id=%d", tableName, sqlite3_last_insert_rowid(Game::instance->db));
	SqliteResult sqliteResult = SqliteGetRows(Game::instance->db, query)[0];
	
	//Warning! 'Double' model files in 'path' (e. g. if we put *.3ds after *.md2 that loaded with an editor) can cause 'any model loading' (e. g. *.3ds instead of *.md2, that we loaded earlier in editor)
	sprintf(path, "editor/%s/model/%s", Game::instance->name, tableName);
	ImportModel(modelPath, path, sqliteResult.integers["id"]);

	T* mapObject = new T(sqliteResult, path);
	SpawnMapObject<T>(mapObjects, mapObjectsCount, mapObject);
	return mapObject;
}
