#include<iostream>
#include"manager/manager.h"
#include "manager/Game_manager.h"

#include <Windows.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return GameManager::instance()->run(__argc, __argv); // 运行游戏
}

