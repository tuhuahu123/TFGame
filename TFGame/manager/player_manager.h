#ifndef _PLAYER_MANAGER_H_    // 头文件保护宏，防止重复包含
#define _PLAYER_MANAGER_H_

// 包含必要的头文件
#include "../tile.h"          // 引入瓦片相关定义
//#include "../facing.h"      // 面向方向头文件（已注释掉）
#include "../vector2.h"       // 引入二维向量类
#include "manager.h"          // 引入管理器基类
#include "../animation.h"     // 引入动画系统
#include "../coin/coin_manager.h" // 引入金币管理器
#include "enemy_manager.h"    // 引入敌人管理器
#include "resources_manager.h" // 引入资源管理器

#include <SDL.h>              // 引入SDL库，用于图形渲染和输入处理

// 玩家管理器类，继承自Manager单例模板类
class PlayerManager : public Manager<PlayerManager>
{
	friend class Manager<PlayerManager>; // 允许Manager基类访问私有成员

public:
	// 处理输入事件的方法
	void on_input(const SDL_Event& event)
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:     // 键盘按下事件
			switch (event.key.keysym.sym)
			{
			case SDLK_a:      // A键控制向左移动
				is_move_left = true;
				break;
			case SDLK_d:      // D键控制向右移动
				is_move_right = true;
				break;
			case SDLK_w:      // W键控制向上移动
				is_move_up = true;
				break;
			case SDLK_s:      // S键控制向下移动
				is_move_down = true;
				break;
			case SDLK_SPACE:      // 发闪光攻击技能
				on_release_flash();
				break;
			case SDLK_LSHIFT:      // 冲击攻击技能
				on_release_impact();
				break;
			default:
				break;
			}
			break;
		case SDL_KEYUP:       // 键盘释放事件
			switch (event.key.keysym.sym)
			{
			case SDLK_a:      // 释放A键，停止向左移动
				is_move_left = false;
				break;
			case SDLK_d:      // 释放D键，停止向右移动
				is_move_right = false;
				break;
			case SDLK_w:      // 释放W键，停止向上移动
				is_move_up = false;
				break;
			case SDLK_s:      // 释放S键，停止向下移动
				is_move_down = false;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}

	// 游戏逻辑更新方法，delta为帧间时间差
	void on_update(double delta)
	{
		// 更新魔法值自动增加和闪光技能冷却的计时器
		timer_auto_increase_mp.on_update(delta);
		timer_release_flash_cd.on_update(delta);

		// 根据按键状态计算移动方向向量并归一化
		Vector2 direction =
			Vector2(is_move_right - is_move_left,
				is_move_down - is_move_up).normalize();
		// 计算当前速度向量
		velocity = direction * speed * SIZE_TILE;

		// 如果没有释放技能，处理移动逻辑
		if (!is_releasing_flash && !is_releasing_impact)
		{
			// 更新位置
			position += velocity * delta;

			// 获取地图边界并限制玩家位置
			const SDL_Rect& rect_map = ConfigManager::instance()->rect_tile_map;
			if (position.x < rect_map.x) position.x = rect_map.x;
			if (position.x > rect_map.x + rect_map.w) position.x = rect_map.x + rect_map.w;
			if (position.y < rect_map.y) position.y = rect_map.y;
			if (position.y > rect_map.y + rect_map.h) position.y = rect_map.y + rect_map.h;

			// 根据移动方向更新面向
			if (velocity.y > 0) facing = Facing::Down;
			if (velocity.y < 0) facing = Facing::Up;
			if (velocity.x > 0) facing = Facing::Right;
			if (velocity.x < 0) facing = Facing::Left;

			// 根据当前面向选择相应的待机动画
			switch (facing)
			{
			case Facing::Left:
				anim_current = &anim_idle_left;
				break;
			case Facing::Right:
				anim_current = &anim_idle_right;
				break;
			case Facing::Up:
				anim_current = &anim_idle_up;
				break;
			case Facing::Down:
				anim_current = &anim_idle_down;
				break;
			}
		}
		else // 正在释放技能时，显示攻击动画
		{
			// 根据当前面向选择相应的攻击动画
			switch (facing)
			{
			case Facing::Left:
				anim_current = &anim_attack_left;
				break;
			case Facing::Right:
				anim_current = &anim_attack_right;
				break;
			case Facing::Up:
				anim_current = &anim_attack_up;
				break;
			case Facing::Down:
				anim_current = &anim_attack_down;
				break;
			}
		}

		// 更新当前动画
		anim_current->on_update(delta);

		// 如果正在释放闪光技能
		if (is_releasing_flash)
		{
			// 更新闪光特效动画
			anim_effect_flash_current->on_update(delta);

			// 获取敌人列表
			EnemyManager::EnemyList& enemy_list
				= EnemyManager::instance()->get_enemy_list();
			// 对每个敌人进行碰撞检测
			for (Enemy* enemy : enemy_list)
			{
				// 跳过已标记为可移除的敌人
				if (enemy->can_remove())
					continue;

				// 获取敌人位置
				const Vector2& position = enemy->get_position();
				// 检查敌人是否在闪光攻击范围内
				if (position.x >= rect_hitbox_flash.x
					&& position.x <= rect_hitbox_flash.x + rect_hitbox_flash.w
					&& position.y >= rect_hitbox_flash.y
					&& position.y <= rect_hitbox_flash.y + rect_hitbox_flash.h)
				{
					// 对范围内敌人造成伤害
					enemy->decrease_hp(ConfigManager::instance()->player_template.normal_attack_damage * delta);
				}
			}
		}

		// 如果正在释放冲击技能
		if (is_releasing_impact)
		{
			// 更新冲击特效动画
			anim_effect_impact_current->on_update(delta);

			// 获取敌人列表
			EnemyManager::EnemyList& enemy_list
				= EnemyManager::instance()->get_enemy_list();
			// 对每个敌人进行碰撞检测
			for (Enemy* enemy : enemy_list)
			{
				// 跳过已标记为可移除的敌人
				if (enemy->can_remove())
					continue;

				// 获取敌人尺寸和位置
				const Vector2& size = enemy->get_size();
				const Vector2& position = enemy->get_position();
				// 检查敌人是否在冲击攻击范围内
				if (position.x >= rect_hitbox_impact.x
					&& position.x <= rect_hitbox_impact.x + rect_hitbox_impact.w
					&& position.y >= rect_hitbox_impact.y
					&& position.y <= rect_hitbox_impact.y + rect_hitbox_impact.h)
				{
					// 对范围内敌人造成伤害并减速
					enemy->decrease_hp(ConfigManager::instance()->player_template.skill_damage * delta);
					enemy->slow_down();
				}
			}
		}

		// 获取金币道具列表
		CoinManager::CoinPropList& coin_prop_list = CoinManager::instance()->get_coin_prop_list();
		// 获取声音资源池
		static const ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();

		// 检查玩家与金币的碰撞
		for (CoinProp* coin_prop : coin_prop_list)
		{
			// 跳过已标记为可移除的金币
			if (coin_prop->can_remove())
				continue;

			// 获取金币位置
			const Vector2& pos_coin_prop = coin_prop->get_position();
			// 检查金币是否在玩家碰撞区域内
			if (pos_coin_prop.x >= position.x - size.x / 2
				&& pos_coin_prop.x <= position.x + size.x / 2
				&& pos_coin_prop.y >= position.y - size.y / 2
				&& pos_coin_prop.y <= position.y + size.y / 2)
			{
				// 使金币失效
				coin_prop->make_invalid();
				// 增加玩家金币数量
				CoinManager::instance()->increase_coin(10);
				// 播放金币收集音效
				Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_Coin)->second, 0);
			}
		}
	}

	// 渲染方法，将玩家和特效绘制到屏幕
	void on_render(SDL_Renderer* renderer)
	{
		// 定义静态点，用于渲染位置
		static SDL_Point point;

		// 计算角色渲染位置（居中绘制）
		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);
		// 渲染当前动画
		anim_current->on_render(renderer, point);

		// 如果正在释放闪光技能，渲染闪光特效
		if (is_releasing_flash)
		{
			point.x = rect_hitbox_flash.x;
			point.y = rect_hitbox_flash.y;
			anim_effect_flash_current->on_render(renderer, point);
		}

		// 如果正在释放冲击技能，渲染冲击特效
		if (is_releasing_impact)
		{
			point.x = rect_hitbox_impact.x;
			point.y = rect_hitbox_impact.y;
			anim_effect_impact_current->on_render(renderer, point);
		}
	}

	// 获取当前魔法值
	double get_current_mp() const
	{
		return mp;
	}

protected:
	// 构造函数，初始化玩家属性、动画和计时器
	PlayerManager()
	{
		// 设置魔法值自动增加的计时器（每0.1秒触发一次）
		timer_auto_increase_mp.set_one_shot(false);
		timer_auto_increase_mp.set_wait_time(0.1);
		timer_auto_increase_mp.set_on_timeout(
			[&]()
			{
				// 根据技能间隔计算魔法值回复速度
				double interval = ConfigManager::instance()->player_template.skill_interval;
				mp = std::min(mp + 100 / (interval / 0.1), 100.0);
			});

		// 设置闪光技能冷却计时器
		timer_release_flash_cd.set_one_shot(true);
		timer_release_flash_cd.set_wait_time(
			ConfigManager::instance()->player_template.skill_interval);
		timer_release_flash_cd.set_on_timeout(
			[&]()
			{
				// 冷却结束，可以再次使用闪光技能
				can_release_flash = true;
			});

		// 获取纹理资源池
		const ResourcesManager::TexturePool& tex_pool
			= ResourcesManager::instance()->get_texture_pool();

		// 获取玩家精灵表纹理
		SDL_Texture* tex_player = tex_pool.find(ResID::Tex_Player)->second;

		// 初始化各个方向的待机动画
		anim_idle_up.set_loop(true); anim_idle_up.set_interval(0.1);
		anim_idle_up.set_frame_data(tex_player, 4, 8, { 4, 5, 6, 7 });
		anim_idle_down.set_loop(true); anim_idle_down.set_interval(0.1);
		anim_idle_down.set_frame_data(tex_player, 4, 8, { 0, 1, 2, 3 });
		anim_idle_left.set_loop(true); anim_idle_left.set_interval(0.1);
		anim_idle_left.set_frame_data(tex_player, 4, 8, { 8, 9, 10, 11 });
		anim_idle_right.set_loop(true); anim_idle_right.set_interval(0.1);
		anim_idle_right.set_frame_data(tex_player, 4, 8, { 12, 13, 14, 15 });

		// 初始化各个方向的攻击动画
		anim_attack_up.set_loop(true); anim_attack_up.set_interval(0.1);
		anim_attack_up.set_frame_data(tex_player, 4, 8, { 20, 21 });
		anim_attack_down.set_loop(true); anim_attack_down.set_interval(0.1);
		anim_attack_down.set_frame_data(tex_player, 4, 8, { 16, 17 });
		anim_attack_left.set_loop(true); anim_attack_left.set_interval(0.1);
		anim_attack_left.set_frame_data(tex_player, 4, 8, { 24, 25 });
		anim_attack_right.set_loop(true); anim_attack_right.set_interval(0.1);
		anim_attack_right.set_frame_data(tex_player, 4, 8, { 28, 29 });

		// 初始化闪光特效动画（向上）
		anim_effect_flash_up.set_loop(false); anim_effect_flash_up.set_interval(0.1);
		anim_effect_flash_up.set_frame_data(tex_pool.find(ResID::Tex_EffectFlash_Up)->second, 5, 1, { 0, 1, 2, 3, 4 });
		anim_effect_flash_up.set_on_finished([&]() { is_releasing_flash = false; });

		// 初始化闪光特效动画（向下）
		anim_effect_flash_down.set_loop(false); anim_effect_flash_down.set_interval(0.1);
		anim_effect_flash_down.set_frame_data(tex_pool.find(ResID::Tex_EffectFlash_Down)->second, 5, 1, { 4, 3, 2, 1, 0 });
		anim_effect_flash_down.set_on_finished([&]() { is_releasing_flash = false; });

		// 初始化闪光特效动画（向左）
		anim_effect_flash_left.set_loop(false); anim_effect_flash_left.set_interval(0.1);
		anim_effect_flash_left.set_frame_data(tex_pool.find(ResID::Tex_EffectFlash_Left)->second, 1, 5, { 4, 3, 2, 1, 0 });
		anim_effect_flash_left.set_on_finished([&]() { is_releasing_flash = false; });

		// 初始化闪光特效动画（向右）
		anim_effect_flash_right.set_loop(false); anim_effect_flash_right.set_interval(0.1);
		anim_effect_flash_right.set_frame_data(tex_pool.find(ResID::Tex_EffectFlash_Right)->second, 1, 5, { 0, 1, 2, 3, 4 });
		anim_effect_flash_right.set_on_finished([&]() { is_releasing_flash = false; });

		// 初始化冲击特效动画（向上）
		anim_effect_impact_up.set_loop(false); anim_effect_impact_up.set_interval(0.1);
		anim_effect_impact_up.set_frame_data(tex_pool.find(ResID::Tex_EffectImpact_Up)->second, 5, 1, { 0, 1, 2, 3, 4 });
		anim_effect_impact_up.set_on_finished([&]() { is_releasing_impact = false; });

		// 初始化冲击特效动画（向下）
		anim_effect_impact_down.set_loop(false); anim_effect_impact_down.set_interval(0.1);
		anim_effect_impact_down.set_frame_data(tex_pool.find(ResID::Tex_EffectImpact_Down)->second, 5, 1, { 4, 3, 2, 1, 0 });
		anim_effect_impact_down.set_on_finished([&]() { is_releasing_impact = false; });

		// 初始化冲击特效动画（向左）
		anim_effect_impact_left.set_loop(false); anim_effect_impact_left.set_interval(0.1);
		anim_effect_impact_left.set_frame_data(tex_pool.find(ResID::Tex_EffectImpact_Left)->second, 1, 5, { 4, 3, 2, 1, 0 });
		anim_effect_impact_left.set_on_finished([&]() { is_releasing_impact = false; });

		// 初始化冲击特效动画（向右）
		anim_effect_impact_right.set_loop(false); anim_effect_impact_right.set_interval(0.1);
		anim_effect_impact_right.set_frame_data(tex_pool.find(ResID::Tex_EffectImpact_Right)->second, 1, 5, { 0, 1, 2, 3, 4 });
		anim_effect_impact_right.set_on_finished([&]() { is_releasing_impact = false; });

		// 设置玩家初始位置（地图中央）
		const SDL_Rect& rect_map = ConfigManager::instance()->rect_tile_map;
		position.x = rect_map.x + rect_map.w / 2;
		position.y = rect_map.y + rect_map.h / 2;

		// 从配置中获取玩家速度
		speed = ConfigManager::instance()->player_template.speed;

		// 设置玩家尺寸
		size.x = 96, size.y = 96;
	}

	// 虚构函数（使用默认）
	~PlayerManager() = default;

private:
	// 玩家属性
	Vector2 size;        // 玩家尺寸
	Vector2 position;    // 玩家位置
	Vector2 velocity;    // 玩家速度向量

	// 技能攻击范围碰撞盒
	SDL_Rect rect_hitbox_flash = { 0 };   // 闪光技能碰撞盒
	SDL_Rect rect_hitbox_impact = { 0 };  // 冲击技能碰撞盒

	double mp = 100;     // 魔法值，初始为100

	double speed = 0;    // 移动速度

	// 技能状态标志
	bool can_release_flash = true;     // 是否可以释放闪光技能
	bool is_releasing_flash = false;   // 是否正在释放闪光技能
	bool is_releasing_impact = false;  // 是否正在释放冲击技能

	// 移动方向控制状态
	bool is_move_up = false;      // 是否正在向上移动
	bool is_move_down = false;    // 是否正在向下移动
	bool is_move_left = false;    // 是否正在向左移动
	bool is_move_right = false;   // 是否正在向右移动

	// 待机动画集
	Animation anim_idle_up;       // 向上待机动画
	Animation anim_idle_down;     // 向下待机动画
	Animation anim_idle_left;     // 向左待机动画
	Animation anim_idle_right;    // 向右待机动画

	// 攻击动画集
	Animation anim_attack_up;     // 向上攻击动画
	Animation anim_attack_down;   // 向下攻击动画
	Animation anim_attack_left;   // 向左攻击动画
	Animation anim_attack_right;  // 向右攻击动画

	// 当前播放动画指针
	Animation* anim_current = &anim_idle_right;

	// 闪光技能特效动画
	Animation anim_effect_flash_up;    // 向上闪光特效动画
	Animation anim_effect_flash_down;  // 向下闪光特效动画
	Animation anim_effect_flash_left;  // 向左闪光特效动画
	Animation anim_effect_flash_right; // 向右闪光特效动画
	Animation* anim_effect_flash_current = nullptr; // 当前闪光特效动画指针

	// 冲击技能特效动画
	Animation anim_effect_impact_up;    // 向上冲击特效动画
	Animation anim_effect_impact_down;  // 向下冲击特效动画
	Animation anim_effect_impact_left;  // 向左冲击特效动画
	Animation anim_effect_impact_right; // 向右冲击特效动画
	Animation* anim_effect_impact_current = nullptr; // 当前冲击特效动画指针

	// 计时器
	Timer timer_release_flash_cd;  // 闪光技能冷却计时器
	Timer timer_auto_increase_mp;  // 魔法值自动恢复计时器

	// 当前朝向
	Facing facing = Facing::Left;

private:
	// 释放闪光技能方法
	void on_release_flash()
	{
		// 检查是否可以释放技能
		if (!can_release_flash || is_releasing_flash)
			return;

		// 根据当前朝向设置闪光特效和碰撞盒
		switch (facing)
		{
		case Facing::Left:
			// 设置特效动画
			anim_effect_flash_current = &anim_effect_flash_left;
			// 设置攻击碰撞盒位置和大小
			rect_hitbox_flash.x = (int)(position.x - size.x / 2 - 300);
			rect_hitbox_flash.y = (int)(position.y - 68 / 2);
			rect_hitbox_flash.w = 300, rect_hitbox_flash.h = 68;
			break;
		case Facing::Right:
			anim_effect_flash_current = &anim_effect_flash_right;
			rect_hitbox_flash.x = (int)(position.x + size.x / 2);
			rect_hitbox_flash.y = (int)(position.y - 68 / 2);
			rect_hitbox_flash.w = 300, rect_hitbox_flash.h = 68;
			break;
		case Facing::Up:
			anim_effect_flash_current = &anim_effect_flash_up;
			rect_hitbox_flash.x = (int)(position.x - 68 / 2);
			rect_hitbox_flash.y = (int)(position.y - size.x / 2 - 300);
			rect_hitbox_flash.w = 68, rect_hitbox_flash.h = 300;
			break;
		case Facing::Down:
			anim_effect_flash_current = &anim_effect_flash_down;
			rect_hitbox_flash.x = (int)(position.x - 68 / 2);
			rect_hitbox_flash.y = (int)(position.y + size.x / 2);
			rect_hitbox_flash.w = 68, rect_hitbox_flash.h = 300;
			break;
		}

		// 设置技能状态
		is_releasing_flash = true;
		// 重置特效动画
		anim_effect_flash_current->reset();
		// 启动技能冷却计时器
		timer_release_flash_cd.restart();

		// 获取声音资源池
		static const ResourcesManager::SoundPool& sound_pool
			= ResourcesManager::instance()->get_sound_pool();

		// 播放闪光技能音效
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_Flash)->second, 0);
	}

	// 释放冲击技能方法
	void on_release_impact()
	{
		// 检查魔法值是否足够且未处于释放状态
		if (mp < 100 || is_releasing_impact)
			return;

		// 根据当前朝向设置冲击特效和碰撞盒
		switch (facing)
		{
		case Facing::Left:
			// 设置特效动画
			anim_effect_impact_current = &anim_effect_impact_left;
			// 设置攻击碰撞盒位置和大小
			rect_hitbox_impact.x = (int)(position.x - size.x / 2 - 60);
			rect_hitbox_impact.y = (int)(position.y - 140 / 2);
			rect_hitbox_impact.w = 60, rect_hitbox_impact.h = 140;
			break;
		case Facing::Right:
			anim_effect_impact_current = &anim_effect_impact_right;
			rect_hitbox_impact.x = (int)(position.x + size.x / 2);
			rect_hitbox_impact.y = (int)(position.y - 140 / 2);
			rect_hitbox_impact.w = 60, rect_hitbox_impact.h = 140;
			break;
		case Facing::Up:
			anim_effect_impact_current = &anim_effect_impact_up;
			rect_hitbox_impact.x = (int)(position.x - 140 / 2);
			rect_hitbox_impact.y = (int)(position.y - size.x / 2 - 60);
			rect_hitbox_impact.w = 140, rect_hitbox_impact.h = 60;
			break;
		case Facing::Down:
			anim_effect_impact_current = &anim_effect_impact_down;
			rect_hitbox_impact.x = (int)(position.x - 140 / 2);
			rect_hitbox_impact.y = (int)(position.y + size.x / 2);
			rect_hitbox_impact.w = 140, rect_hitbox_impact.h = 60;
			break;
		}

		// 消耗所有魔法值
		mp = 0;
		// 设置技能状态
		is_releasing_impact = true;
		// 重置特效动画
		anim_effect_impact_current->reset();

		// 获取声音资源池
		static const ResourcesManager::SoundPool& sound_pool
			= ResourcesManager::instance()->get_sound_pool();

		// 播放冲击技能音效
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_Impact)->second, 0);
	}
};

#endif // !_PLAYER_MANAGER_H_