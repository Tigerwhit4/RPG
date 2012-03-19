// stdafx.h: ���������� ���� ��� ����������� ��������� ���������� ������
// ��� ���������� ������ ��� ����������� �������, ������� ����� ������������, ��
// �� ����� ����������
//

#pragma once

// TODO: ���������� ����� ������ �� �������������� ���������, ����������� ��� ���������

#include "targetver.h"
#include <stdio.h>
#include <windows.h>
#include <vector>
#include <map>
#include <string>

//Include

#include "../Include/gl/GL.h"
#include "../Include/gl/GLU.h"
#include "../Include/gl/GLAUX.h"
#include "../Include/SDL/SDL.h"
#include "../Include/SDL/SDL_image.h"
#include "../Include/sqlite3.h"
#include "../Include/guichan/guichan.hpp"
#include "../Include/guichan/guichan/opengl.hpp"
#include "../Include/guichan/guichan/sdl.hpp"
#include "../Include/guichan/guichan/opengl/openglsdlimageloader.hpp"

#include "../RPGatorDll/RPGatorDll.h"

//Lib

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "../Lib/opengl32.lib")
#pragma comment(lib, "../Lib/glu32.lib")
#pragma comment(lib, "../Lib/GLAUX.lib")
#pragma comment(lib, "../Lib/SDL.lib")
#pragma comment(lib, "../Lib/SDLmain.lib")
#pragma comment(lib, "../Lib/SDL_image.lib")
#pragma comment(lib, "../Lib/sqlite3.lib")
#pragma comment(lib, "../Lib/guichan.lib")

#pragma comment(lib, "../Release/RPGator.lib")
