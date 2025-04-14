#pragma once

#include "tile.h"
#include "timer.h"
#include "vector2.h"
#include "manager/resources_manager.h"

#include <SDL.h>

/**
 * @class CoinProp
 * @brief 金币道具类，表示游戏中的金币对象
 *
 * 该类实现了金币的物理行为、动画效果和生命周期管理。
 * 金币具有跳跃和悬浮两种状态，并在一定时间后自动消失。
 */
class CoinProp
{
public:
	/**
	 * @brief 构造函数，初始化金币属性和行为
	 *
	 * 创建金币时，设置两个计时器：
	 * 1. 跳跃计时器：控制金币从跳跃状态转为悬浮状态
	 * 2. 消失计时器：控制金币在一段时间后自动消失
	 *
	 * 同时随机设置金币的初始水平速度方向（左或右）
	 */
	CoinProp()
	{
		// 设置跳跃计时器，当时间到达后停止跳跃状态
		timer_jump.set_one_shot(true);
		timer_jump.set_wait_time(interval_jump);
		timer_jump.set_on_timeout(
			[&]()
			{
				is_jumping = false;
			});

		// 设置消失计时器，当时间到达后使金币无效（可被移除）
		timer_disappear.set_one_shot(true);
		timer_disappear.set_wait_time(interval_disappear);
		timer_disappear.set_on_timeout(
			[&]()
			{
				is_valid = false;
			});

		// 随机设置初始水平速度方向（左或右），并设置初始垂直速度（向上）
		velocity.x = (rand() % 2 ? 1 : -1) * 2 * SIZE_TILE;
		velocity.y = -3 * SIZE_TILE;
	}

	/**
	 * @brief 析构函数，使用默认实现
	 */
	~CoinProp() = default;

	/**
	 * @brief 设置金币的位置
	 * @param position 金币的新位置坐标
	 */
	void set_position(const Vector2& position)
	{
		this->position = position;
	}

	/**
	 * @brief 获取金币的当前位置
	 * @return 金币位置的引用
	 */
	const Vector2& get_position() const
	{
		return position;
	}

	/**
	 * @brief 获取金币的大小
	 * @return 金币大小的引用
	 */
	const Vector2& get_size() const
	{
		return size;
	}

	/**
	 * @brief 使金币无效（标记为可移除）
	 */
	void make_invalid()
	{
		is_valid = false;
	}

	/**
	 * @brief 检查金币是否可以从游戏中移除
	 * @return 如果金币无效则返回true，表示可以移除
	 */
	bool can_remove()
	{
		return !is_valid;
	}

	/**
	 * @brief 更新金币状态
	 * @param delta 两帧之间的时间差（秒）
	 *
	 * 根据当前状态更新金币的位置和行为：
	 * - 跳跃状态：受重力影响，模拟物理跳跃
	 * - 悬浮状态：进行上下浮动的正弦运动
	 */
	void on_update(double delta)
	{
		// 更新计时器状态
		timer_jump.on_update(delta);
		timer_disappear.on_update(delta);

		if (is_jumping)
		{
			// 跳跃状态：应用重力，y方向速度逐渐增加（向下）
			velocity.y += gravity * delta;
		}
		else
		{
			// 悬浮状态：停止水平移动，进行上下浮动的正弦运动
			velocity.x = 0;
			velocity.y = sin(SDL_GetTicks64() / 1000.0 * 4) * 30;
		}

		// 根据速度更新位置
		position += velocity * delta;
	}

	/**
	 * @brief 渲染金币
	 * @param renderer SDL渲染器对象
	 *
	 * 将金币纹理绘制到屏幕上，位置基于金币的当前位置
	 */
	void on_render(SDL_Renderer* renderer)
	{
		// 创建静态矩形用于渲染，并获取金币纹理
		static SDL_Rect rect = { 0, 0, (int)size.x, (int)size.y };
		static SDL_Texture* tex_coin = ResourcesManager::instance()
			->get_texture_pool().find(ResID::Tex_Coin)->second;

		// 计算渲染位置（居中显示）
		rect.x = (int)(position.x - size.x / 2);
		rect.y = (int)(position.y - size.y / 2);

		// 将金币纹理绘制到渲染器上
		SDL_RenderCopy(renderer, tex_coin, nullptr, &rect);
	}

private:
	Vector2 position;       // 金币的位置坐标
	Vector2 velocity;       // 金币的移动速度向量

	Timer timer_jump;       // 控制跳跃状态持续时间的计时器
	Timer timer_disappear;  // 控制金币消失时间的计时器

	bool is_valid = true;   // 金币是否有效（无效则可移除）
	bool is_jumping = true; // 金币是否处于跳跃状态

	double gravity = 490;             // 重力加速度
	double interval_jump = 0.75;      // 跳跃状态持续时间（秒）
	Vector2 size = { 16, 16 };        // 金币大小（宽度,高度）
	double interval_disappear = 10;   // 金币存在时间（秒）
};