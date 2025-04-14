#pragma once

#include <vector>

#define SIZE_TILE 48 //地图块大小

struct Tile
{
	enum class Direction
	{
		None=0,
		Up,
		Down,
		Left,
		Right
	};

	int terrian = 0;						//地形
	int decoration = -1;					//装饰
	int special_flag = -1;					//特殊
	bool has_tower = false;					//是否有塔
	Direction direction = Direction::None;  // 方向
};


typedef std::vector<std::vector<Tile>> TileMap; //地图


