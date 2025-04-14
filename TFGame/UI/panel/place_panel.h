#ifndef _PLACE_PANEL_H_
#define _PLACE_PANEL_H_

#include "panel.h"
#include "../../coin/coin_manager.h"
#include "../../tower/tower_manager.h"
#include "../../manager/resources_manager.h"

#include <SDL2_gfxPrimitives.h>

/**
 * PlacePanel类 - 防御塔放置面板
 *
 * 功能：允许玩家在选中的地图格子上放置不同类型的防御塔
 *
 * 主要组件：
 * - 顶部按钮：放置斧头兵塔(Axeman)
 * - 左侧按钮：放置弓箭手塔(Archer)
 * - 右侧按钮：放置炮手塔(Gunner)
 *
 * 特性：
 * - 显示各类防御塔的建造成本
 * - 显示各类防御塔的攻击范围（鼠标悬停时）
 * - 只有金币充足时才能放置防御塔
 * - 放置后会自动关闭面板
 */
class PlacePanel : public Panel
{
public:
	/**
	 * 构造函数 - 初始化放置面板
	 *
	 * 从资源管理器加载面板的各种状态纹理
	 */
	PlacePanel()
	{
		// 获取纹理资源池
		const ResourcesManager::TexturePool& texture_pool
			= ResourcesManager::instance()->get_texture_pool();

		// 加载面板各状态纹理
		tex_idle = texture_pool.find(ResID::Tex_UIPlaceIdle)->second;          // 默认状态纹理
		tex_hovered_top = texture_pool.find(ResID::Tex_UIPlaceHoveredTop)->second;  // 顶部悬停纹理
		tex_hovered_left = texture_pool.find(ResID::Tex_UIPlaceHoveredLeft)->second; // 左侧悬停纹理
		tex_hovered_right = texture_pool.find(ResID::Tex_UIPlaceHoveredRight)->second; // 右侧悬停纹理
	}

	// 使用默认析构函数
	~PlacePanel() = default;

	/**
	 * 更新面板状态
	 *
	 * 获取各类防御塔的成本和攻击范围，供显示和判断使用
	 *
	 * @param renderer SDL渲染器，传递给父类更新方法
	 */
	void on_update(SDL_Renderer* renderer) override
	{
		// 获取塔管理器的单例实例
		static TowerManager* instance = TowerManager::instance();

		// 更新各类防御塔的建造成本
		val_top = (int)instance->get_place_cost(TowerType::Axeman);    // 顶部按钮：斧头兵塔成本
		val_left = (int)instance->get_place_cost(TowerType::Archer);   // 左侧按钮：弓箭手塔成本
		val_right = (int)instance->get_place_cost(TowerType::Gunner);  // 右侧按钮：炮手塔成本

		// 更新各类防御塔的攻击范围（像素单位）
		reg_top = (int)instance->get_damage_range(TowerType::Axeman) * SIZE_TILE;   // 斧头兵塔攻击范围
		reg_left = (int)instance->get_damage_range(TowerType::Archer) * SIZE_TILE;  // 弓箭手塔攻击范围
		reg_right = (int)instance->get_damage_range(TowerType::Gunner) * SIZE_TILE; // 炮手塔攻击范围

		// 调用父类的更新方法以更新文本显示
		Panel::on_update(renderer);
	}

	/**
	 * 渲染面板
	 *
	 * 绘制攻击范围预览圈和面板界面
	 *
	 * @param renderer SDL渲染器
	 */
	void on_render(SDL_Renderer* renderer) override
	{
		// 如果面板不可见，不进行任何渲染
		if (!visible) return;

		// 根据鼠标悬停的按钮确定要显示的范围
		int reg = 0;
		switch (hovered_target)
		{
		case Panel::HoveredTarget::Top:
			reg = reg_top;    // 显示斧头兵塔攻击范围
			break;
		case Panel::HoveredTarget::Left:
			reg = reg_left;   // 显示弓箭手塔攻击范围
			break;
		case Panel::HoveredTarget::Right:
			reg = reg_right;  // 显示炮手塔攻击范围
			break;
		}

		// 如果有悬停目标且攻击范围大于0，绘制攻击范围预览
		if (reg > 0)
		{
			// 绘制半透明的攻击范围填充圆
			filledCircleRGBA(renderer, center_pos.x, center_pos.y, reg,
				color_region_content.r, color_region_content.g, color_region_content.b, color_region_content.a);
			// 绘制攻击范围边框圆（抗锯齿）
			aacircleRGBA(renderer, center_pos.x, center_pos.y, reg,
				color_region_frame.r, color_region_frame.g, color_region_frame.b, color_region_frame.a);
		}

		// 调用父类渲染方法绘制面板本身
		Panel::on_render(renderer);
	}

protected:
	/**
	 * 处理点击顶部按钮：放置斧头兵塔
	 *
	 * 检查金币是否足够，如果足够则扣除金币并放置防御塔
	 */
	void on_click_top_area() override
	{
		// 获取金币管理器实例
		CoinManager* instance = CoinManager::instance();

		// 检查玩家是否有足够的金币放置斧头兵塔
		if (val_top <= instance->get_current_coin_num())
		{
			// 在选中的格子位置放置斧头兵塔
			TowerManager::instance()->place_tower(TowerType::Axeman, idx_tile_selected);
			// 扣除相应的金币
			instance->decrease_coin(val_top);
		}
	}

	/**
	 * 处理点击左侧按钮：放置弓箭手塔
	 *
	 * 检查金币是否足够，如果足够则扣除金币并放置防御塔
	 */
	void on_click_left_area() override
	{
		// 获取金币管理器实例
		CoinManager* instance = CoinManager::instance();

		// 检查玩家是否有足够的金币放置弓箭手塔
		if (val_left <= instance->get_current_coin_num())
		{
			// 在选中的格子位置放置弓箭手塔
			TowerManager::instance()->place_tower(TowerType::Archer, idx_tile_selected);
			// 扣除相应的金币
			instance->decrease_coin(val_left);
		}
	}

	/**
	 * 处理点击右侧按钮：放置炮手塔
	 *
	 * 检查金币是否足够，如果足够则扣除金币并放置防御塔
	 */
	void on_click_right_area() override
	{
		// 获取金币管理器实例
		CoinManager* instance = CoinManager::instance();

		// 检查玩家是否有足够的金币放置炮手塔
		if (val_right <= instance->get_current_coin_num())
		{
			// 在选中的格子位置放置炮手塔
			TowerManager::instance()->place_tower(TowerType::Gunner, idx_tile_selected);
			// 扣除相应的金币
			instance->decrease_coin(val_right);
		}
	}

private:
	// 攻击范围显示的颜色定义
	const SDL_Color color_region_frame = { 255, 255, 255, 175 };    // 范围边框颜色（深蓝色，透明度175）
	const SDL_Color color_region_content = { 255, 255, 255, 75 };  // 范围填充颜色（浅蓝色，透明度75）

private:
	int reg_top = 0, reg_left = 0, reg_right = 0;  // 三种防御塔的攻击范围（像素单位）

};

#endif // !_PLACE_PANEL_H_