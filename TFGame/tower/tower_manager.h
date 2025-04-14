#pragma once  

#include "tower.h"                     // 防御塔基类
#include "tower_type.h"                // 防御塔类型枚举
#include "../manager/manager.h"        // 管理器基类模板
#include "archer_tower.h"              // 弓箭手防御塔
#include "axeman_tower.h"              // 斧头兵防御塔
#include "gunner_tower.h"              // 炮手防御塔
#include "../manager/config_manager.h" // 配置管理器
#include "../manager/resources_manager.h" // 资源管理器

#include <vector>  // 用于存储防御塔列表

/**
 * TowerManager类 - 防御塔管理器
 *
 * 负责创建、更新和管理游戏中的所有防御塔
 * 实现了单例模式，确保全局只有一个实例
 */
class TowerManager : public Manager<TowerManager>  // 继承自Manager模板类，实现单例模式
{
	friend class Manager<TowerManager>;  // 允许Manager基类访问私有成员，用于单例实现

public:
	/**
	 * 更新所有防御塔状态
	 * @param delta 上一帧到当前帧的时间间隔(秒)
	 */
	void on_update(double delta)
	{
		// 遍历所有防御塔，调用它们的更新方法
		for (Tower* tower : tower_list)
			tower->on_update(delta);
	}

	/**
	 * 渲染所有防御塔
	 * @param renderer SDL渲染器
	 */
	void on_render(SDL_Renderer* renderer)
	{
		// 遍历所有防御塔，调用它们的渲染方法
		for (Tower* tower : tower_list)
			tower->on_render(renderer);
	}

	/**
	 * 获取指定类型防御塔的建造成本
	 * @param type 防御塔类型
	 * @return 返回建造成本（金币数量）
	 */
	double get_place_cost(TowerType type)
	{
		// 获取配置管理器实例（使用静态变量避免重复获取）
		static ConfigManager* instance = ConfigManager::instance();

		// 根据塔类型返回对应的建造成本
		switch (type)
		{
		case Archer:  // 弓箭手塔
			return instance->archer_template.cost[instance->level_archere];
			break;
		case Axeman:  // 斧头兵塔
			return instance->axeman_template.cost[instance->level_axeman];
			break;
		case Gunner:  // 炮手塔
			return instance->gunner_template.cost[instance->level_gunner];
			break;
		}

		return 0;  // 默认返回0
	}

	/**
	 * 获取指定类型防御塔的升级成本
	 * @param type 防御塔类型
	 * @return 返回升级成本，如果已达最高等级则返回-1
	 */
	double get_upgrade_cost(TowerType type)
	{
		// 获取配置管理器实例
		static ConfigManager* instance = ConfigManager::instance();

		// 根据塔类型返回对应的升级成本
		switch (type)
		{
		case Archer:  // 弓箭手塔
			// 如果已达最高等级(9)，返回-1，否则返回对应等级的升级成本
			return instance->level_archere == 9 ? -1 :
				instance->archer_template.upgrade_cost[instance->level_archere];
			break;
		case Axeman:  // 斧头兵塔
			return instance->level_axeman == 9 ? -1 :
				instance->axeman_template.upgrade_cost[instance->level_axeman];
			break;
		case Gunner:  // 炮手塔
			return instance->level_gunner == 9 ? -1 :
				instance->gunner_template.upgrade_cost[instance->level_gunner];
			break;
		}

		return 0;  // 默认返回0
	}

	/**
	 * 获取指定类型防御塔的攻击范围
	 * @param type 防御塔类型
	 * @return 返回攻击范围（格子数）
	 */
	double get_damage_range(TowerType type)
	{
		// 获取配置管理器实例
		static ConfigManager* instance = ConfigManager::instance();

		// 根据塔类型返回对应的攻击范围
		switch (type)
		{
		case Archer:  // 弓箭手塔
			return instance->archer_template.view_range[instance->level_archere];
			break;
		case Axeman:  // 斧头兵塔
			return instance->axeman_template.view_range[instance->level_axeman];
			break;
		case Gunner:  // 炮手塔
			return instance->gunner_template.view_range[instance->level_gunner];
			break;
		}

		return 0;  // 默认返回0
	}

	/**
	 * 在指定位置放置防御塔
	 * @param type 防御塔类型
	 * @param idx 地图格子索引（行列坐标）
	 */
	void place_tower(TowerType type, const SDL_Point& idx)
	{
		// 声明防御塔指针
		Tower* tower = nullptr;

		// 根据类型创建对应的防御塔对象
		switch (type)
		{
		case Archer:  // 弓箭手塔
			tower = new ArcherTower();
			break;
		case Axeman:  // 斧头兵塔
			tower = new AxemanTower();
			break;
		case Gunner:  // 炮手塔
			tower = new GunnerTower();
			break;
		default:  // 默认创建弓箭手塔
			tower = new ArcherTower();
			break;
		}

		// 声明位置变量和获取地图区域信息
		static Vector2 position;
		static const SDL_Rect& rect = ConfigManager::instance()->rect_tile_map;

		// 计算防御塔的像素坐标（将格子索引转换为屏幕坐标）
		// 格子中心 = 地图左上角 + 格子索引 * 格子大小 + 格子大小的一半
		position.x = rect.x + idx.x * SIZE_TILE + SIZE_TILE / 2;
		position.y = rect.y + idx.y * SIZE_TILE + SIZE_TILE / 2;

		// 设置防御塔位置并添加到防御塔列表
		tower->set_position(position);
		tower_list.push_back(tower);

		// 在地图中标记该位置已放置防御塔
		ConfigManager::instance()->map.place_tower(idx);

		// 播放放置防御塔的音效
		static const ResourcesManager::SoundPool& sound_pool
			= ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_PlaceTower)->second, 0);
	}

	/**
	 * 升级指定类型的防御塔
	 * @param type 要升级的防御塔类型
	 */
	void upgrade_tower(TowerType type)
	{
		// 获取配置管理器实例
		static ConfigManager* instance = ConfigManager::instance();

		// 根据类型升级对应的防御塔等级
		switch (type)
		{
		case Archer:  // 弓箭手塔
			// 如果等级已达9级，保持不变，否则等级+1
			instance->level_archere = instance->level_archere >= 9 ? 9 : instance->level_archere + 1;
			break;
		case Axeman:  // 斧头兵塔
			instance->level_axeman = instance->level_axeman >= 9 ? 9 : instance->level_axeman + 1;
			break;
		case Gunner:  // 炮手塔
			instance->level_gunner = instance->level_gunner >= 9 ? 9 : instance->level_gunner + 1;
			break;
		}

		// 播放防御塔升级的音效
		static const ResourcesManager::SoundPool& sound_pool
			= ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_TowerLevelUp)->second, 0);
	}

protected:
	// 构造函数和析构函数使用默认实现
	// 设为protected防止外部直接创建实例，支持单例模式
	TowerManager() = default;
	~TowerManager() = default;

private:
	std::vector<Tower*> tower_list;  // 存储所有防御塔的列表
};

