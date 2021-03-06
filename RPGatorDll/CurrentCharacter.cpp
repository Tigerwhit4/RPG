#include "StdAfx.h"
#include "ForwardDeclaration.h"
#include "SqliteResult.h"
#include "utilities.h"
#include "Render.h"
#include "GameObject.h"
#include "MapObject.h"
#include "Skill.h"
#include "Item.h"
#include "Character.h"
#include "CurrentGameObject.h"
#include "CurrentMapObject.h"
#include "CurrentItem.h"
#include "CurrentQuest.h"
#include "CurrentSkill.h"
#include "Location.h"
#include "GameResources.h"
#include "Game.h"
#include "ConnectSocket.h"
#include "CurrentCharacter.h"

//Initialization from DB (Editor, Server)
CurrentCharacter::CurrentCharacter(SqliteResult sqliteResult, Location* location) :
	CurrentMapObject<Character>::CurrentMapObject(sqliteResult, Game::instance->resources->characters, Game::instance->resources->charactersCount, location)
{
	movingX = x;
	movingY = y;

	strcpy(login, sqliteResult.strings["login"].c_str());
	strcpy(password, sqliteResult.strings["password"].c_str());

	hp = 100;

	char query[256];
	int rowsCount;
	std::vector<SqliteResult> sqliteResultsChildren;

	if (node)
		setTitle(login);

	//CurrentItems
	sprintf(query, "SELECT * FROM CurrentItem WHERE locationId=0 AND currentCharacterId=%d", id);
	sqliteResultsChildren = SqliteGetRows(Game::instance->db, query);
	currentItems = NULL;
	currentItemsCount = 0;
	rowsCount = sqliteResultsChildren.size();
	while (currentItemsCount < rowsCount)
	{
		SpawnItem(new CurrentItem(sqliteResultsChildren[currentItemsCount], NULL, this));
	}

	//CurrentQuests
	sprintf(query, "SELECT * FROM CurrentQuest WHERE currentCharacterId=%d", id);
	sqliteResultsChildren = SqliteGetRows(Game::instance->db, query);
	currentQuests = NULL;
	currentQuestsCount = 0;
	rowsCount = sqliteResultsChildren.size();
	while (currentQuestsCount < rowsCount)
	{
		currentQuestsCount++;
		currentQuests = (CurrentQuest**)realloc(currentQuests, currentQuestsCount * sizeof(CurrentQuest*));
		currentQuests[currentQuestsCount - 1] = new CurrentQuest(sqliteResultsChildren[currentQuestsCount - 1], this);
	}

	//CurrentSkills
	sprintf(query, "SELECT * FROM CurrentSkill WHERE currentCharacterId=%d", id);
	sqliteResultsChildren = SqliteGetRows(Game::instance->db, query);
	currentSkills = NULL;
	currentSkillsCount = 0;
	rowsCount = sqliteResultsChildren.size();
	while (currentSkillsCount < rowsCount)
	{
		SpawnSkill(new CurrentSkill(sqliteResultsChildren[currentSkillsCount], this));
	}
}

//Initialization from incoming packet (Client)
CurrentCharacter::CurrentCharacter(char* currentMapObjectSpawnedPacket) :
	CurrentMapObject<Character>::CurrentMapObject(currentMapObjectSpawnedPacket, Game::instance->resources->characters, Game::instance->resources->charactersCount)
{
	movingX = x;
	movingY = y;

	ScanPacket(currentMapObjectSpawnedPacket, "%i%i%f%f%s", NULL, NULL, NULL, NULL, login);
	password[0] = '\0'; //strcpy(password, "");
	
	if (node)
		setTitle(login);

	currentItems = NULL;
	currentItemsCount = 0;

	currentQuests = NULL;
	currentQuestsCount = 0;

	currentSkills = NULL;
	currentSkillsCount = 0;
}

CurrentCharacter::~CurrentCharacter(void)
{
}

CurrentItem* CurrentCharacter::GetItem(int id)
{
	for (int i = 0; i < currentItemsCount; i++)
		if (currentItems[i]->id == id)
			return currentItems[i];
	return NULL;
}

CurrentQuest* CurrentCharacter::GetQuest(int id)
{
	for (int i = 0; i < currentQuestsCount; i++)
		if (currentQuests[i]->id == id)
			return currentQuests[i];
	return NULL;
}

CurrentSkill* CurrentCharacter::GetSkill(int id)
{
	for (int i = 0; i < currentSkillsCount; i++)
		if (currentSkills[i]->id == id)
			return currentSkills[i];
	return NULL;
}

void CurrentCharacter::SpawnItem(CurrentItem* currentItem)
{
	currentItemsCount++;
	currentItems = (CurrentItem**)realloc(currentItems, currentItemsCount * sizeof(CurrentItem*));
	currentItems[currentItemsCount - 1] = currentItem;
}

void CurrentCharacter::SpawnSkill(CurrentSkill* currentSkill)
{
	currentSkillsCount++;
	currentSkills = (CurrentSkill**)realloc(currentSkills, currentSkillsCount * sizeof(CurrentSkill*));
	currentSkills[currentSkillsCount - 1] = currentSkill;
}

CurrentItem* CurrentCharacter::AddItem(Item* base, int count)
{
	char query[256];

	sprintf(query, "INSERT INTO CurrentItem(baseId, x, y, locationId, currentCharacterId, `count`) VALUES(%d, 0, 0, NULL, %d, %d)", base->id, this->id, count);
	sqlite3_exec(Game::instance->db, query, NULL, NULL, NULL);
	sprintf(query, "SELECT * FROM CurrentItem WHERE id=%d", sqlite3_last_insert_rowid(Game::instance->db));
	SqliteResult sqliteResult = SqliteGetRows(Game::instance->db, query)[0];
	CurrentItem* currentItem = new CurrentItem(sqliteResult, NULL, this);
	SpawnItem(currentItem);
	return currentItem;
}

CurrentSkill* CurrentCharacter::AddSkill(Skill* base)
{
	char query[256];

	sprintf(query, "INSERT INTO CurrentSkill(baseId, currentCharacterId) VALUES(%d, %d)", base->id, this->id);
	sqlite3_exec(Game::instance->db, query, NULL, NULL, NULL);
	sprintf(query, "SELECT * FROM CurrentSkill WHERE id=%d", sqlite3_last_insert_rowid(Game::instance->db));
	SqliteResult sqliteResult = SqliteGetRows(Game::instance->db, query)[0];
	CurrentSkill* currentSkill = new CurrentSkill(sqliteResult, this);
	SpawnSkill(currentSkill);
	return currentSkill;
}

void CurrentCharacter::Update()
{
	char sql[256];
	sprintf(sql, "UPDATE CurrentCharacter SET x=%.f, y=%.f, locationId=%d, login='%s', password='%s' WHERE id=%d;", x, y, currentLocation->id, login, password, id);
	sqlite3_exec(Game::instance->db, sql, NULL, NULL, NULL);
	for (int i = 0; i < currentItemsCount; i++)
		currentItems[i]->Update();
	for (int i = 0; i < currentQuestsCount; i++)
		currentQuests[i]->Update();
	for (int i = 0; i < currentSkillsCount; i++)
		currentSkills[i]->Update();
}

void CurrentCharacter::RecalculateDelta()
{
	f32 distance = vector2d<f32>(x, y).getDistanceFrom(vector2d<f32>(movingX, movingY));
	f32 cosL = -(x - movingX) / distance;
	f32 sinL = -(y - movingY) / distance;
	deltaX = base->speed * cosL;
	deltaY = base->speed * sinL;
}
