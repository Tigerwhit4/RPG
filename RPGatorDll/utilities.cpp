#include "StdAfx.h"
#include "ForwardDeclaration.h"
#include "SqliteResult.h"
#include "utilities.h"
#include "Render.h"


extern "C" __declspec(dllexport) int ReadDir(const char* path, char** &elements, bool directoriesOnly)
	/************************************************************************/
	/* Creates array of filenames which are in directory                    */
	/* path: directory to analyze. "dir1/.../dir2" (no '/' in the end)      */
	/* elements: output array for filenames                                 */
	/* directoriesOnly: If 'true' only directories will be counted.         */
	/*                  If 'false' directories and files will be counted    */
	/*                                                                      */
	/* Return value: number of counted elements                             */
	/************************************************************************/
{
	int res, i, count;
	char* pathMask;
	HANDLE elementHandle;
	WIN32_FIND_DATA dat;
	std::vector<std::string> elementsV;

	pathMask = new char[strlen(path) + 3];
	strcpy(pathMask, path);
	strcat(pathMask, "/*");
	
	elementHandle = FindFirstFile(pathMask, &dat); //Project->Properties->Configuration Properties->General->Character Set //MultiByteToWideChar
	res = FindNextFile(elementHandle, &dat); //Skipping '.'
	res = FindNextFile(elementHandle, &dat); //Skipping '..'
	while (res)
	{
		if (directoriesOnly ? dat.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY : 1)
			elementsV.push_back((char*)(dat.cFileName));
		res = FindNextFile(elementHandle, &dat);
	}
	FindClose(elementHandle);
	count = elementsV.size();
	
	elements = new char*[count];
	for (i = 0; i < count; i++)
	{
		elements[i] = new char[elementsV[i].length() + 1];
		strcpy(elements[i], elementsV[i].c_str());
	}
	delete pathMask;
	elementsV.clear();
	return count;
}

extern "C" __declspec(dllexport) void ClearDir(const char* path)
{
	char** paths;
	int i;

	int numOfDirs = ReadDir(path, paths, true);

	if (numOfDirs != 0)
	{
		for (i = 0; i < numOfDirs; i++)
		{
			ClearDir( ((std::string)path + "/" + (std::string)paths[i]).c_str() );
			printf("Deleting directory: %s\n", paths[i]);
			RemoveDirectory(((std::string)path + "/" + (std::string)paths[i]).c_str());
			delete paths[i];
		}
	}

	delete paths;
	int numOfFiles = ReadDir(path, paths, false);

	if (numOfFiles != 0)
	{
		for (i = 0; i < numOfFiles; i++)
		{
			printf("Deleting file: %s\n",paths[i]);
			DeleteFile(((std::string)path + "/" + (std::string)paths[i]).c_str());
		}
	}
}


extern "C" __declspec(dllexport) void SetPacketLength(char* packet, int length)
{
	*((short int*)packet) = length;
}

extern "C" __declspec(dllexport) int GetPacketLength(char* packet)
{
	return *((short int*)packet);
}

extern "C" __declspec(dllexport) void IncreasePacketLength(char* packet, int length)
{
	SetPacketLength(packet, GetPacketLength(packet) + length);
}

extern "C" __declspec(dllexport) void PacketAddString(char* packet, char* str)
{
	strcpy(packet + GetPacketLength(packet) + 2, str);
	IncreasePacketLength(packet, strlen(str) + 1);
}

extern "C" __declspec(dllexport) void PacketAddInt(char* packet, int n)
{
	memcpy(packet + GetPacketLength(packet) + 2, (char*)&n, 4);
	IncreasePacketLength(packet, 4);
}

extern "C" __declspec(dllexport) void PacketAddDouble(char* packet, f32 n)
{
	memcpy(packet + GetPacketLength(packet) + 2, (char*)&n, sizeof(f32));
	IncreasePacketLength(packet, sizeof(f32));
}

extern "C" __declspec(dllexport) void PacketAddByte(char* packet, char n)
{
	packet[GetPacketLength(packet) + 2] = n;
	IncreasePacketLength(packet, 1);
}

extern "C" __declspec(dllexport) char* PacketGetString(char* packet, int pos)
{
	return packet + 2 + pos;
}

extern "C" __declspec(dllexport) int PacketGetInt(char* packet, int pos)
{
	return *((int*)(packet + 2 + pos));
}

extern "C" __declspec(dllexport) f32 PacketGetDouble(char* packet, int pos)
{
	return *((f32*)(packet + 2 + pos));
}

extern "C" __declspec(dllexport) char PacketGetByte(char* packet, int pos)
{
	return packet[2 + pos];
}

extern "C" __declspec(dllexport) void SetPacketType(char* packet, Packet type)
{
	packet[2] = type;
}

extern "C" __declspec(dllexport) Packet GetPacketType(char* packet)
{
	return (Packet)packet[2];
}

void CreatePacket( char* packet, Packet packetType, char* formatStr, ... )
{
	SetPacketLength(packet, 1);
	SetPacketType(packet, packetType);

	va_list params;

	va_start(params, formatStr);
	
	char tmp[256];
	strcpy(tmp, formatStr);
	char* token;
	char* nextToken;
	token = strtok_s(tmp,"% ", &nextToken);
	
	while(token != NULL)
	{
		if (!strcmp(token,"i")) 
			PacketAddInt(packet, va_arg(params, int));
		else if (!strcmp(token,"b"))
			PacketAddByte(packet, va_arg(params, char));
		else if (!strcmp(token,"s")) 
			PacketAddString(packet, va_arg(params, char*));
		else if (!strcmp(token,"f")) 
		{
			PacketAddDouble(packet, (f32)va_arg(params, double));
			//PacketAddDouble(packet, va_arg(params, f32)); //TODO: Why it's not working?
		}
		else if (!strcmp(token,"ws")) 
		{
			//Warning! It only converts wchar_t* to char*. Does not insert wide string.
			wchar_t* wstr = va_arg(params, wchar_t*);
			int length = wcslen(wstr);
			char* str = new char[length + 1];
			wcstombs(str, wstr, length + 1);
			PacketAddString(packet, str);
			delete str;
		}
		token = strtok_s(NULL, "% ", &nextToken);
	}

	va_end(params);
}


void ScanPacket( char* packet, char* formatStr, ... )
{
	va_list params;

	va_start(params, formatStr);
	
	char tmp[256];
	strcpy(tmp, formatStr);
	char* token;
	char* nextToken;
	token = strtok_s(tmp,"% ", &nextToken);
	int currentPosition = 1;

	while(token != NULL)
	{
		if (!strcmp(token,"i")) 
		{
			int * p = va_arg(params, int*);
			if (p)
				*p = PacketGetInt(packet, currentPosition);
			currentPosition += 4;
		}
		else if (!strcmp(token,"b"))
		{
			char* p = va_arg(params, char*);
			if (p)
				*p = PacketGetByte(packet, currentPosition);
			currentPosition += 1;
		}
		else if (!strcmp(token,"s"))
		{
			char* str = PacketGetString(packet, currentPosition);

			char* p = va_arg(params, char*);
			if (p)
				strcpy(p, str);
			currentPosition += strlen(str) + 1;
		}
		else if (!strcmp(token,"f"))
		{
			f32* p = va_arg(params, f32*);
			if (p)
				*p = (f32)PacketGetDouble(packet, currentPosition);
			currentPosition += sizeof(f32);
		}
		else if (!strcmp(token,"ws")) 
		{
			char *str = PacketGetString(packet, currentPosition);

			wchar_t* p = va_arg(params, wchar_t*);
			if (p)
				mbstowcs(p, str, strlen(str) + 1);
			currentPosition += strlen(str) + 1;
		}
		token = strtok_s(NULL, "% ", &nextToken);
	}

	va_end(params);
}


extern "C++" __declspec(dllexport) std::vector<SqliteResult> SqliteGetRows(sqlite3* db, char* query)
{
	int result, i, columnsCount, rowsCount;
	sqlite3_stmt *stmt;
	std::string columnName;
	std::vector<SqliteResult> sqliteResults;

	if (sqlite3_prepare(db, query, -1, &stmt, NULL) != SQLITE_OK)
	{
		printf("Couldn't execute query '%s'\n", query);
		return sqliteResults;
	}
	
	rowsCount = 0;
	while ((result = sqlite3_step(stmt)) != SQLITE_DONE)
	{
		if (result == SQLITE_ROW)
		{
			sqliteResults.push_back(SqliteResult());
			
			columnsCount = sqlite3_column_count(stmt);
			for (i = 0; i < columnsCount; i++)
			{
				columnName = sqlite3_column_name(stmt, i);
				switch (sqlite3_column_type(stmt, i))
				{
					case SQLITE_BLOB:
						sqliteResults[rowsCount].strings[columnName] = (std::string)(char*)sqlite3_column_blob(stmt, i);
						break;
					case SQLITE_TEXT:
						sqliteResults[rowsCount].strings[columnName] = (std::string)(char*)sqlite3_column_text(stmt, i);
						break;
					case SQLITE_INTEGER:
						sqliteResults[rowsCount].integers[columnName] = sqlite3_column_int(stmt, i);
						break;
					case SQLITE_FLOAT:
						sqliteResults[rowsCount].doubles[columnName] = (f32)sqlite3_column_double(stmt, i);
						break;
				}
			}
			rowsCount++;
		}
		else
		{
			printf("SQLite error. Code: %d\n", result);
			sqlite3_finalize(stmt);
			return sqliteResults;
		}
	}
	sqlite3_finalize(stmt);

	return sqliteResults;
}

int Pix2Index(int pos)
{
	return (int)(pos / CELL_SIZE);
}

int Index2Pix(int pos)
{
	return (int)(pos * CELL_SIZE);
}

int PixRound(int pos)
{
	return Index2Pix(Pix2Index(pos));
}

bool FileExists(char* path)
{
	FILE* f = fopen(path, "rb");
	if (!f)
		return false;
	fclose(f);
	return true;
}

void ImportModel(char* source, char* destination, int id) //TODO: move this function into editor
{
	char path[256];
	char tPath[256];
	char extension[8];
	
	//Delete old model
	sprintf(path, "%s/%d.3ds", destination, id);
	if (FileExists(path))
		DeleteFile(path);
	else
	{
		sprintf(path, "%s/%d.md2", destination, id);
		if (FileExists(path))
			DeleteFile(path);
		else
		{/*
			sprintf(path, "%s/%d.x", destination, id);
			if (FileExists(path))
				DeleteFile(path);*/
		}
	}

	int i = strlen(source) - 1;
	while (source[i] != '.' && i > 0)
		i--;
	strcpy(extension, source + i + 1);
	
	//Model
	sprintf(path, "%s/%d.%s", destination, id, extension);
	CopyFile(source, path, false);

	//Texture
	strcpy(tPath, source);
	tPath[i] = '\0';
	strcat(tPath, ".jpg");
	sprintf(path, "%s/%d.jpg", destination, id);
	if (FileExists(path))
		DeleteFile(path);
	CopyFile(tPath, path, false);
}

u32 GetCurentTime(  )
{
	return 0;
}
