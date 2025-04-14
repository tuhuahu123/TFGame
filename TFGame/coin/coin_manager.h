#pragma once  

#include "coin_prop.h"      // 包含金币道具类定义
#include "../manager/manager.h"        // 包含管理器基类模板，提供单例模式
#include "../manager/config_manager.h" // 包含配置管理器，用于获取初始金币数量

#include <vector>           // 使用标准库向量容器存储金币道具

/**
 * @class CoinManager
 * @brief 金币管理器类 - 管理游戏中所有与金币相关的功能
 *
 * CoinManager使用单例模式设计，只有一个全局实例。它负责：
 * 1. 跟踪玩家当前拥有的金币数量
 * 2. 处理金币的增加和减少（如购买防御塔时消费）
 * 3. 管理游戏场景中的金币道具（如敌人被击败后掉落的金币）
 * 4. 更新和渲染所有金币道具
 *
 * 使用示例:
 *   增加金币：CoinManager::instance()->increase_coin(50);
 *   消费金币：CoinManager::instance()->decrease_coin(100);
 *   查询金币：double coins = CoinManager::instance()->get_current_coin_num();
 */
class CoinManager : public Manager<CoinManager>
{
	// 允许Manager<CoinManager>访问CoinManager的私有成员
	friend class Manager<CoinManager>;

public:
	/**
	 * @brief 金币道具列表类型定义
	 *
	 * 使用std::vector存储指向CoinProp对象的指针，
	 * 方便管理场景中的多个金币道具
	 */
	typedef std::vector<CoinProp*> CoinPropList;

public:
	/**
	 * @brief 增加玩家金币数量
	 *
	 * @param val 要增加的金币数量
	 *
	 * 当玩家完成波次、拾取金币或击败敌人时，
	 * 会调用此方法增加玩家的金币数量
	 */
	void increase_coin(double val)
	{
		num_coin += val;  // 直接增加金币数量
	}

	/**
	 * @brief 减少玩家金币数量
	 *
	 * @param val 要减少的金币数量
	 *
	 * 当玩家购买或升级防御塔时，会调用此方法减少金币。
	 * 如果金币不足，会将金币设为0，不会变成负数。
	 */
	void decrease_coin(double val)
	{
		num_coin -= val;  // 减少金币数量

		// 确保金币不会变为负数
		if (num_coin < 0)
			num_coin = 0;
	}

	/**
	 * @brief 更新所有金币道具状态
	 *
	 * @param delta 当前帧与上一帧的时间差（秒）
	 *
	 * 此方法在每一帧被游戏主循环调用，用于：
	 * 1. 更新每个金币道具的状态
	 * 2. 移除已经被拾取或过期的金币道具
	 */
	void on_update(double delta)
	{
		// 更新每个金币道具的状态
		for (CoinProp* coin_prop : coin_prop_list)
			coin_prop->on_update(delta);

		// 移除需要删除的金币道具（使用C++11的Lambda表达式和标准库算法）
		coin_prop_list.erase(std::remove_if(coin_prop_list.begin(), coin_prop_list.end(),
			[](CoinProp* coin_prop)
			{
				bool deletable = coin_prop->can_remove();  // 检查是否可以删除
				if (deletable) delete coin_prop;           // 如果可以删除，释放内存
				return deletable;                          // 返回是否应该从列表中移除
			}), coin_prop_list.end());
	}

	/**
	 * @brief 渲染所有金币道具
	 *
	 * @param renderer SDL渲染器指针，用于绘制图形
	 *
	 * 此方法在每一帧被游戏主循环调用，用于绘制场景中所有的金币道具
	 */
	void on_render(SDL_Renderer* renderer)
	{
		// 遍历并渲染每个金币道具
		for (CoinProp* coin_prop : coin_prop_list)
			coin_prop->on_render(renderer);
	}

	/**
	 * @brief 获取当前玩家拥有的金币数量
	 *
	 * @return double 当前金币数量
	 *
	 * 用于UI显示或检查玩家是否有足够金币购买道具
	 */
	double get_current_coin_num()
	{
		return num_coin;  // 返回当前金币数量
	}

	/**
	 * @brief 获取金币道具列表的引用
	 *
	 * @return CoinPropList& 金币道具列表的引用
	 *
	 * 此方法主要供其他系统调用，用于检查玩家与金币的碰撞
	 */
	CoinPropList& get_coin_prop_list()
	{
		return coin_prop_list;  // 返回金币道具列表的引用
	}

	/**
	 * @brief 在指定位置生成一个金币道具
	 *
	 * @param position 金币道具的生成位置（游戏世界坐标）
	 *
	 * 当敌人被击败时，会调用此方法在敌人死亡位置生成金币道具，
	 * 供玩家收集
	 */
	void spawn_coin_prop(const Vector2& position)
	{
		// 创建新的金币道具对象
		CoinProp* coin_prop = new CoinProp();
		// 设置金币道具的位置
		coin_prop->set_position(position);
		// 将金币道具添加到列表中进行管理
		coin_prop_list.push_back(coin_prop);
	}

protected:
	/**
	 * @brief 构造函数 - 初始化金币管理器
	 *
	 * 由于使用单例模式，此构造函数被设为protected防止直接实例化。
	 * 初始化时从配置管理器获取初始金币数量
	 */
	CoinManager()
	{
		// 从配置管理器获取初始金币数量
		num_coin = ConfigManager::instance()->num_initial_coin;
	}

	/**
	 * @brief 析构函数 - 清理资源
	 *
	 * 释放所有金币道具占用的内存，防止内存泄漏
	 */
	~CoinManager()
	{
		// 清理所有金币道具
		for (CoinProp* coin_prop : coin_prop_list)
			delete coin_prop;
	}

private:
	double num_coin = 0;         // 玩家当前拥有的金币数量，初始值为0（构造函数中会从配置读取）
	CoinPropList coin_prop_list;  // 当前场景中存在的所有金币道具
};