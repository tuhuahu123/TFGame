#pragma once  // 确保头文件只被包含一次，防止重复定义

// 包含相关头文件
#include "bullet.h"           // 包含基础子弹类定义
#include "manager/manager.h"  // 包含管理器基类模板
#include "arrow_bullet.h"     // 包含箭矢子弹类定义
#include "axe_bullet.h"       // 包含斧头子弹类定义
#include "shell_bullet.h"     // 包含炮弹子弹类定义
#include "bullet_type.h"      // 包含子弹类型枚举

#include <vector>             // 使用标准库的向量容器

/**
 * BulletManager类 - 子弹管理器
 *
 * 负责管理游戏中所有子弹的创建、更新、渲染和销毁。
 * 继承自Manager模板类，实现单例模式。
 */
class BulletManager : public Manager<BulletManager>
{
	// 声明Manager<BulletManager>为友元类，使其能访问BulletManager的私有成员
	friend class Manager<BulletManager>;

public:
	// 定义BulletList类型为Bullet指针的vector容器，用于存储所有活跃的子弹
	typedef std::vector<Bullet*> BulletList;

public:
	/**
	 * 更新所有子弹
	 * @param delta 帧时间间隔(秒)
	 *
	 * 1. 先更新所有子弹的状态
	 * 2. 然后移除并删除标记为可移除的子弹
	 */
	void on_update(double delta)
	{
		// 遍历所有子弹并调用其更新方法
		for (Bullet* bullet : bullet_list)
			bullet->on_update(delta);

		// 创建一个新列表来存储需要保留的子弹
		BulletList bullets_to_keep;

		// 遍历所有子弹
		for (Bullet* bullet : bullet_list)
		{
			if (bullet->can_remove())
			{
				// 如果子弹需要移除，释放其内存
				delete bullet;
			}
			else
			{
				// 否则保留在新列表中
				bullets_to_keep.push_back(bullet);
			}
		}

		// 用保留的子弹替换原列表
		bullet_list = bullets_to_keep;
	}

	/**
	 * 渲染所有子弹
	 * @param renderer SDL渲染器
	 *
	 * 遍历所有子弹并调用其渲染方法
	 */
	void on_render(SDL_Renderer* renderer)
	{
		for (Bullet* bullet : bullet_list)
			bullet->on_render(renderer);
	}

	/**
	 * 获取子弹列表
	 * @return 返回子弹列表的引用，允许外部代码访问所有活跃的子弹
	 */
	BulletList& get_bullet_list()
	{
		return bullet_list;
	}

	/**
	 * 发射一个新子弹
	 * @param type 子弹类型（Arrow箭矢, Axe斧头, Shell炮弹）
	 * @param position 子弹初始位置
	 * @param velocity 子弹速度向量（决定方向和速度）
	 * @param damage 子弹伤害值
	 *
	 * 根据指定类型创建相应的子弹，设置其属性并添加到子弹列表中
	 */
	void fire_bullet(BulletType type, const Vector2& position, const Vector2& velocity, double damage)
	{
		Bullet* bullet = nullptr;  // 初始化子弹指针为空

		// 根据子弹类型创建对应的子弹对象
		switch (type)
		{
		case Arrow:
			bullet = new ArrowBullet();		// 创建箭矢子弹
			break;
		case Axe:
			bullet = new AxeBullet();		// 创建斧头子弹
			break;
		case Shell:
			bullet = new ShellBullet();	   // 创建炮弹子弹
			break;
		default:
		bullet = new ArrowBullet();       // 默认创建箭矢子弹
			break;
		}

		// 设置子弹的基本属性
		bullet->set_position(position);  // 设置初始位置
		bullet->set_velocity(velocity);  // 设置速度向量
		bullet->set_damage(damage);      // 设置伤害值

		// 将新创建的子弹添加到子弹列表中进行管理
		bullet_list.push_back(bullet);
	}

protected:
	/**
	 * 构造函数 - 使用默认实现
	 *
	 * 被声明为protected以限制直接创建实例，遵循单例模式
	 * 只能通过Manager模板类的get_instance()方法获取实例
	 */
	BulletManager() = default;

	/**
	 * 析构函数
	 *
	 * 负责清理所有子弹对象，防止内存泄漏
	 */
	~BulletManager()
	{
		// 释放所有子弹对象的内存
		for (Bullet* bullet : bullet_list)
			delete bullet;
	}

private:
	/**
	 * 存储所有活跃子弹的容器
	 *
	 * 包含指向所有当前游戏中存在的子弹对象的指针
	 */
	BulletList bullet_list;
};