#pragma once
#include "timer.h"
#include "manager/manager.h"
#include "manager/config_manager.h"
#include "enemy_manager.h"

#include "coin/coin_manager.h"
#include <Instance.h>


class WaveManager : public Manager<WaveManager>
{
	// 允许Manager<WaveManager>访问WaveManager的私有成员
	friend class Manager<WaveManager>;

public:
	/**
	 * @brief 更新波次管理器状态
	 *
	 * 这个函数在每一帧被调用，用于更新波次状态、生成敌人和检查当前波次是否结束
	 *
	 * @param delta 当前帧与上一帧的时间差（秒）
	 */
	void on_update(double delta)
	{
		// 获取配置管理器实例
		static ConfigManager* instance = ConfigManager::instance();

		// 如果游戏已经结束，则不再更新波次
		if (instance->is_game_over)
			return;

		// 根据波次状态更新相应的计时器
		if (!is_wave_started)
			timer_start_wave.on_update(delta);  // 更新波次开始计时器
		else
			timer_spawn_enemy.on_update(delta); // 更新敌人生成计时器

		// 检查当前波次是否已经完成（最后一个敌人已生成且所有敌人被消灭）
		if (is_spawned_last_enemy && EnemyManager::instance()->check_cleared())
		{
			// 给予玩家当前波次的金币奖励
			CoinManager::instance()->increase_coin(instance->wave_list[idx_wave].rawards);

			// 进入下一波次
			idx_wave++;

			// 检查是否所有波次都已完成
			if (idx_wave >= instance->wave_list.size())
			{
				// 所有波次完成，玩家获胜
				instance->is_game_win = true;
				instance->is_game_over = true;
			}
			else
			{
				// 准备开始下一波次
				idx_spawn_event = 0;               // 重置生成事件索引
				is_wave_started = true;            // 设置波次已开始状态
				is_spawned_last_enemy = false;     // 重置最后敌人生成标记

				// 配置下一波次的时间间隔
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
		// 获取配置中的波次列表
		static const std::vector<Wave>& wave_list = ConfigManager::instance()->wave_list;

		// 配置波次开始计时器
		timer_start_wave.set_one_shot(true);  // 设置为单次触发模式
		timer_start_wave.set_wait_time(wave_list[0].interval);  // 设置第一波的等待时间
		timer_start_wave.set_on_timeout(
			[&]()
			{
				// 波次开始计时器超时回调：开始生成敌人
				is_wave_started = true;  // 标记波次已开始
				// 设置第一个敌人的生成间隔
				timer_spawn_enemy.set_wait_time(wave_list[idx_wave].spawn_event_list[0].interval);
				timer_spawn_enemy.restart();  // 启动敌人生成计时器
			}
		);

		// 配置敌人生成计时器
		timer_spawn_enemy.set_one_shot(true);  // 设置为单次触发模式
		timer_spawn_enemy.set_on_timeout(
			[&]()
			{
				// 敌人生成计时器超时回调：生成一个敌人
				const std::vector<Wave::SpawnEvent>& spawn_event_list = wave_list[idx_wave].spawn_event_list;
				const Wave::SpawnEvent& spawn_event = spawn_event_list[idx_spawn_event];

				// 通过敌人管理器生成敌人
				EnemyManager::instance()->spawn_enemy(spawn_event.enemy_type, spawn_event.spawn_point);

				// 移动到下一个敌人生成事件
				idx_spawn_event++;

				// 检查是否是当前波次的最后一个敌人
				if (idx_spawn_event >= spawn_event_list.size())
				{
					is_spawned_last_enemy = true;  // 标记已生成最后一个敌人
					return;
				}

				// 设置下一个敌人的生成间隔并重启计时器
				timer_spawn_enemy.set_wait_time(spawn_event_list[idx_spawn_event].interval);
				timer_spawn_enemy.restart();
			}
		);
	}

	// 析构函数使用默认实现
	~WaveManager() = default;

private:
	int idx_wave = 0;               // 当前波次索引
	int idx_spawn_event = 0;        // 当前敌人生成事件的索引
	Timer timer_start_wave;         // 波次开始计时器，控制波次之间的间隔
	Timer timer_spawn_enemy;        // 敌人生成计时器，控制同一波次中敌人生成的间隔
	bool is_wave_started = false;   // 标记当前波次是否已开始
	bool is_spawned_last_enemy = false;  // 标记当前波次的最后一个敌人是否已生成
};