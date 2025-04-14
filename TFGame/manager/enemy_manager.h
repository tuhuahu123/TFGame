#pragma once               // 确保此头文件在编译过程中只被包含一次

#include "enemy/enemy.h"          // 敌人基类声明
#include "manager.h"        // 管理器模板类，提供单例模式
#include "config_manager.h" // 配置管理器，获取游戏配置
#include "home_manager.h"   // 基地管理器，用于处理敌人到达基地的逻辑
#include "enemy/enemy_slim.h"     // 史莱姆敌人类
#include "enemy/enemy_king_slim.h"// 史莱姆王敌人类
#include "enemy/enemy_skeleton.h" // 骷髅敌人类
#include "enemy/enemy_goblin.h"   // 哥布林敌人类
#include "enemy/enemy_goblin_priest.h" // 哥布林牧师敌人类
#include "bullet/bullet_manager.h" // 子弹管理器，用于处理子弹碰撞
#include "coin/coin_manager.h"   // 金币管理器，用于生成金币掉落

#include <vector>           // 使用标准模板库的向量容器
#include <SDL.h>            // SDL库，用于图形渲染

/**
 * EnemyManager类 - 敌人管理器
 *
 * 负责创建、更新和管理游戏中的所有敌人单位
 * 处理敌人的移动、碰撞检测、生命周期管理等
 */
class EnemyManager : public Manager<EnemyManager>
{
	friend class Manager<EnemyManager>; // 允许Manager模板类访问私有成员，实现单例模式

public:
	typedef std::vector<Enemy*> EnemyList; // 定义敌人列表类型，用于存储所有活动敌人

public:
	/**
	 * 更新所有敌人状态
	 *
	 * @param delta 帧时间间隔(秒)
	 */
	void on_update(double delta)
	{
		// 更新每个敌人的状态
		for (Enemy* enemy : enemy_list)
			enemy->on_update(delta);

		// 处理敌人与基地的碰撞
		process_home_collision();
		// 处理敌人与子弹的碰撞
		process_bullet_collision();

		// 移除无效的敌人（已死亡或到达终点）
		remove_invalid_enemy();
	}

	/**
	 * 渲染所有敌人
	 *
	 * @param renderer SDL渲染器
	 */
	void on_render(SDL_Renderer* renderer)
	{
		for (Enemy* enemy : enemy_list)
			enemy->on_render(renderer);
	}

	/**
	 * 在指定生成点创建特定类型的敌人
	 *
	 * @param type 敌人类型枚举
	 * @param idx_spawn_point 生成点索引
	 */
	void spawn_enemy(EnemyType type, int idx_spawn_point)
	{
		// 静态变量提高效率，避免重复创建
		static Vector2 position;
		// 获取地图区域
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		// 获取所有生成点和路径的映射
		static const Map::SpawnerRoutePool& spawner_route_pool
			= ConfigManager::instance()->map.get_idx_spawner_pool();

		// 查找指定生成点的路径
		const auto& itor = spawner_route_pool.find(idx_spawn_point);
		if (itor == spawner_route_pool.end())
			return; // 如果找不到对应生成点，则退出

		Enemy* enemy = nullptr;

		// 根据指定的类型创建对应的敌人
		switch (type)
		{
		case EnemyType::Slim:
			enemy = new SlimEnemy();
			break;
		case EnemyType::KingSlim:
			enemy = new KingSlimeEnemy();
			break;
		case EnemyType::Skeleton:
			enemy = new SkeletonEnemy();
			break;
		case EnemyType::Goblin:
			enemy = new GoblinEnemy();
			break;
		case EnemyType::GoblinPriest:
			enemy = new GoblinPriestEnemy();
			break;
		default:
			enemy = new SlimEnemy(); // 默认创建史莱姆
			break;
		}

		// 设置敌人的技能释放回调
		enemy->set_on_skill_released(
			[&](Enemy* enemy_src)
			{
				// 获取治疗范围
				double recover_raduis = enemy_src->get_recover_radius();
				if (recover_raduis < 0) return; // 负值表示无治疗能力

				// 获取释放技能敌人的位置
				const Vector2 pos_src = enemy_src->get_position();
				// 为范围内的所有敌人提供治疗
				for (Enemy* enemy_dst : enemy_list)
				{
					const Vector2& pos_dst = enemy_dst->get_position();
					double distance = (pos_dst - pos_src).length();
					if (distance <= recover_raduis)
						enemy_dst->increase_hp(enemy_src->get_recover_intensity());
				}
			});

		// 获取路径的第一个点作为敌人的起始位置
		const Route::IdxList& idx_list = itor->second.get_idx_list();
		// 计算像素坐标（从网格坐标转换，并考虑地图偏移和格子中心点）
		position.x = rect_tile_map.x + idx_list[0].x * SIZE_TILE + SIZE_TILE / 2;
		position.y = rect_tile_map.y + idx_list[0].y * SIZE_TILE + SIZE_TILE / 2;

		// 设置敌人位置和路径
		enemy->set_position(position);
		enemy->set_route(&itor->second);

		// 将新敌人添加到敌人列表
		enemy_list.push_back(enemy);
	}

	/**
	 * 检查是否所有敌人都被清除
	 *
	 * @return 敌人列表为空时返回true
	 */
	bool check_cleared()
	{
		return enemy_list.empty();
	}

	/**
	 * 获取敌人列表引用
	 *
	 * @return 敌人列表的引用
	 */
	EnemyManager::EnemyList& get_enemy_list()
	{
		return enemy_list;
	}

protected:
	// 构造函数使用默认实现，私有以支持单例模式
	EnemyManager() = default;

	// 析构函数，清理所有敌人资源
	~EnemyManager()
	{
		// 删除所有敌人对象，避免内存泄漏
		for (Enemy* enemy : enemy_list)
			delete enemy;
	}

private:
	EnemyList enemy_list; // 存储所有活动的敌人

private:
	/**
	 * 处理敌人与基地的碰撞
	 * 当敌人到达基地时，对基地造成伤害并移除敌人
	 */
	void process_home_collision()
	{
		// 获取基地在地图上的索引位置
		static const SDL_Point& idx_home = ConfigManager::instance()->map.get_idx_home();
		// 获取地图区域
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		// 计算基地在屏幕上的像素坐标
		static const Vector2 position_home_tile =
		{
			// 根据基地索引位置计算像素坐标 
			(double)rect_tile_map.x + idx_home.x * SIZE_TILE,
			(double)rect_tile_map.y + idx_home.y * SIZE_TILE
		};

		// 遍历所有敌人
		for (Enemy* enemy : enemy_list)
		{
			if (enemy->can_remove()) continue; // 跳过已标记为移除的敌人

			const Vector2& position = enemy->get_position();

			// 检查敌人是否进入基地区域
			if (position.x >= position_home_tile.x
				&& position.y >= position_home_tile.y
				&& position.x <= position_home_tile.x + SIZE_TILE
				&& position.y <= position_home_tile.y + SIZE_TILE)
			{
				enemy->make_invalid(); // 标记敌人为无效（将被移除）

				// 对基地造成伤害
				HomeManager::instance()->decrease_hp(enemy->get_damage());
			}
		}
	}

	/**
	 * 处理敌人与子弹的碰撞
	 * 检测子弹与敌人的碰撞，计算伤害和范围伤害
	 */
	void process_bullet_collision()
	{
		// 获取所有活动子弹
		static BulletManager::BulletList& bullet_list
			= BulletManager::instance()->get_bullet_list();

		// 遍历所有敌人
		for (Enemy* enemy : enemy_list)
		{
			if (enemy->can_remove()) continue; // 跳过已标记为移除的敌人

			const Vector2& size_enemy = enemy->get_size(); // 敌人碰撞箱大小
			const Vector2& pos_enemy = enemy->get_position(); // 敌人位置

			// 遍历所有子弹
			for (Bullet* bullet : bullet_list)
			{
				if (!bullet->can_collide()) continue; // 跳过不能碰撞的子弹

				const Vector2& pos_bullet = bullet->get_position(); // 子弹位置

				// 检查子弹是否与敌人碰撞（AABB碰撞检测）
				if (pos_bullet.x >= pos_enemy.x - size_enemy.x / 2
					&& pos_bullet.y >= pos_enemy.y - size_enemy.y / 2
					&& pos_bullet.x <= pos_enemy.x + size_enemy.x / 2
					&& pos_bullet.y <= pos_enemy.y + size_enemy.y / 2)
				{
					double damage = bullet->get_damage(); // 子弹伤害
					double damage_range = bullet->get_damage_range(); // 伤害范围

					if (damage_range < 0)
					{
						// 单体伤害（无范围伤害）
						enemy->decrease_hp(damage);
						if (enemy->can_remove())
							try_spawn_coin_prop(pos_enemy, enemy->get_reward_ratio());
					}
					else
					{
						// 范围伤害，影响碰撞点周围一定范围内的所有敌人
						for (Enemy* target_enemy : enemy_list)
						{
							const Vector2& pos_target_enemy = target_enemy->get_position();
							// 计算距离，检查是否在伤害范围内
							if ((pos_target_enemy - pos_bullet).length() <= damage_range)
							{
								target_enemy->decrease_hp(damage);
								if (target_enemy->can_remove())
									try_spawn_coin_prop(pos_target_enemy, enemy->get_reward_ratio());
							}
						}
					}

					// 通知子弹发生了碰撞（可能导致子弹消失）
					bullet->on_collide(enemy);
				}
			}
		}
	}

	/**
	 * 从列表中移除无效敌人
	 * 使用remove_if和erase的组合删除并释放无效敌人的内存
	 */
	void remove_invalid_enemy()
	{
		enemy_list.erase(std::remove_if(enemy_list.begin(), enemy_list.end(),
			[](const Enemy* enemy)
			{
				bool deletable = enemy->can_remove();
				if (deletable) delete enemy; // 释放内存
				return deletable;
			}), enemy_list.end());
	}

	/**
	 * 尝试在指定位置生成金币道具
	 *
	 * @param position 生成位置
	 * @param ratio 概率比率（0-1之间的值）
	 */
	void try_spawn_coin_prop(const Vector2& position, double ratio)
	{
		static CoinManager* instance = CoinManager::instance();

		// 按概率决定是否生成金币
		if ((double)(rand() % 100) / 100 <= ratio)
			instance->spawn_coin_prop(position);
	}

};
