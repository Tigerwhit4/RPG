// MapEditor.cpp: ���������� ����� ����� ��� ����������� ����������.
//

#include "stdafx.h"
#include "Universe.h"

int main(int argc, char* argv[])
{
	Universe* universe;
	char* gameName;

	universe = new Universe();

	
		universe->Run("testgame");
		//delete gameName;


	return 0;
}

