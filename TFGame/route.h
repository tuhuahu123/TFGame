#pragma once
#include"tile.h"
#include <SDL_video.h>



class Route  //怪物路径
{
	public:
		typedef std::vector<SDL_Point> IdxList; 

		Route() = default;


		//构建怪物路径：根据地图指示连接起点到终点。
		Route(const TileMap& map, const SDL_Point& idx_orgin) 
		{
			size_t width_map = map[0].size();  //地图宽度
			size_t height_map = map.size();    //地图高度
			SDL_Point idx_next = idx_orgin;	  //当前索引

			while (true)
			{
				// 边界检查：确保坐标在地图范围内
				if (idx_next.x >= width_map || idx_next.y >= height_map) 
				{
					break;
				}

				// 循环检查：防止路径形成环路
				if (check_duplicate_idx(idx_next))
				{
					break;
				}		
				else
				{
					
					idx_list.push_back(idx_next);	// 将当前位置添加到路径列表中
				}

				bool is_next_dir_exist = true; //下一个方向是否存在 
				const Tile& tile = map[idx_next.y][idx_next.x]; //当前地图块

				//碰到房屋退出
				if (tile.special_flag==0) 
				{
					break;
				}

				switch (tile.direction) //方向
				{
				case Tile::Direction::Up:    //上
					idx_next.y--;
					break;
				case Tile::Direction::Down:  //下
					idx_next.y++;
					break;
				case Tile::Direction::Left:  //左
					idx_next.x--;
					break;
				case Tile::Direction::Right: //右
					idx_next.x++;
					break;
				default:
					is_next_dir_exist = false; //没有路标
					break;
				}

				// 路径连续性检查：如果没有有效方向，终止路径生成
				if(!is_next_dir_exist) 
				{
					break;
				}
			}
		}


		~Route() = default;

		//获取索引列表
		const IdxList& get_idx_list() const 
		{
			return idx_list;
		}

	private:
		IdxList idx_list; //索引列表


		//检查重复
		bool check_duplicate_idx(const SDL_Point& idx) 
		{
			for (const auto& idx_item : idx_list)  //遍历索引列表
			{
				if (idx_item.x == idx.x && idx_item.y == idx.y)  //重复
				{
					return true;
				}
			}
			return false;
		}

};