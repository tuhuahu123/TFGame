#pragma once

#include "timer.h"
#include "manager.h"
#include "config_manager.h"
#include "enemy_manager.h"
#include "coin/coin_manager.h"

/**
 * @class WaveManager
 * @brief 波次管理器 - 控制游戏中敌人波次的生成和管理
 *
 * 波次管理器负责按照配置依次生成不同波次的敌人，处理波次间隔，
 * 并在每波敌人被消灭后给予玩家奖励。该类使用单例模式实现。
 */
class WaveManager : public Manager<WaveManager>
{
	// 允许Manager基类访问本类的私有成员，实现单例模式
	friend class Manager<WaveManager>;

public:
	/**
	 * @brief 更新波次管理器状态
	 *
	 * 每帧调用此函数以更新波次计时器和敌人生成计时器，
	 * 并检查当前波次是否结束，处理波次切换逻辑
	 *
	 * @param delta 当前帧与上一帧的时间差（秒）
	 */
	void on_update(double delta)
	{
		// 获取配置管理器实例
		static ConfigManager* instance = ConfigManager::instance();

		// 如果游戏已结束，不再处理波次逻辑
		if (instance->is_game_over)
			return;

		// 根据当前状态更新不同的计时器
		if (!is_wave_started)
			timer_start_wave.on_update(delta); // 更新波次开始计时器
		else
			timer_spawn_enemy.on_update(delta); // 更新敌人生成计时器

		// 检查当前波次是否已完成：最后一个敌人已生成且所有敌人都被清除
		if (is_spawned_last_enemy && EnemyManager::instance()->check_cleared())
		{
			// 给予玩家当前波次的奖励金币
			CoinManager::instance()->increase_coin(instance->wave_list[idx_wave].rawards);

			// 切换到下一波次
			idx_wave++;

			// 检查是否已经是最后一波
			if (idx_wave >= instance->wave_list.size())
			{
				// 所有波次完成，游戏胜利
				instance->is_game_win = true;
				instance->is_game_over = true;
			}
			else
			{
				// 重置下一波次的状态
				idx_spawn_event = 0;         // 重置生成事件索引
				is_wave_started = true;      // 标记新波次已开始
				is_spawned_last_enemy = false; // 重置敌人生成状态

				// 设置新波次的间隔时间
				const Wave& wave = instance->wave_list[idx_wave];
				timer_start_wave.set_wait_time(wave.interval);
				timer_start_wave.restart();
			}
		}
	}

protected:
	/**
	 * @brief 构造函数 - 初始化波次管理器
	 *
	 * 设置波次开始计时器和敌人生成计时器的参数和回调函数
	 * 由于使用单例模式，此构造函数被设为protected防止直接实例化
	 */
	WaveManager()
	{
		// 获取所有波次配置
		static const std::vector<Wave>& wave_list = ConfigManager::instance()->wave_list;

		// 设置波次开始计时器
		timer_start_wave.set_one_shot(true); // 设置为单次触发
		timer_start_wave.set_wait_time(wave_list[0].interval); // 设置第一波等待时间
		timer_start_wave.set_on_timeout(
			[&]() // 设置计时器触发时的回调函数
			{
				// 标记波次已开始
				is_wave_started = true;
				// 设置敌人生成计时器，准备生成第一个敌人
				timer_spawn_enemy.set_wait_time(wave_list[idx_wave].spawn_event_list[0].interval);
				timer_spawn_enemy.restart();
			}
		);

		// 设置敌人生成计时器
		timer_spawn_enemy.set_one_shot(true); // 设置为单次触发
		timer_spawn_enemy.set_on_timeout(
			[&]() // 设置计时器触发时的回调函数
			{
				// 获取当前波次的所有生成事件
				const std::vector<Wave::SpawnEvent>& spawn_event_list = wave_list[idx_wave].spawn_event_list;
				// 获取当前要执行的生成事件
				const Wave::SpawnEvent& spawn_event = spawn_event_list[idx_spawn_event];

				// 生成指定类型的敌人在指定生成点
				EnemyManager::instance()->spawn_enemy(spawn_event.enemy_type, spawn_event.spawn_point);

				// 移动到下一个生成事件
				idx_spawn_event++;

				// 检查是否已经是当前波次的最后一个生成事件
				if (idx_spawn_event >= spawn_event_list.size())
				{
					// 标记当前波次的所有敌人已生成
					is_spawned_last_enemy = true;
					return;
				}

				// 设置下一个敌人的生成间隔
				timer_spawn_enemy.set_wait_time(spawn_event_list[idx_spawn_event].interval);
				timer_spawn_enemy.restart();
			}
		);
	}

	// 析构函数使用默认实现
	~WaveManager() = default;

private:
	int idx_wave = 0;					 // 当前波次索引
	int idx_spawn_event = 0;			 // 当前敌人生成事件的索引
	Timer timer_start_wave;				 // 波次开始计时器，控制波次之间的间隔
	Timer timer_spawn_enemy;			 // 敌人生成计时器，控制同一波次中敌人生成的间隔
	bool is_wave_started = false;		// 标记当前波次是否已开始
	bool is_spawned_last_enemy = false; // 标记当前波次的最后一个敌人是否已生成
};