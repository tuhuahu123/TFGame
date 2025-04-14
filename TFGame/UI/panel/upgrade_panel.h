#ifndef _UPGRADE_PANEL_H_  // 防止头文件重复包含
#define _UPGRADE_PANEL_H_

#include "panel.h"                      // 包含基类Panel
#include "../../tower/tower_manager.h"  // 包含防御塔管理器，用于获取升级信息
#include "../../manager/resources_manager.h" // 包含资源管理器，用于加载纹理资源

/**
 * UpgradePanel类 - 防御塔升级面板
 *
 * 功能：允许玩家升级三种不同类型的防御塔
 *
 * 主要组件：
 * - 顶部按钮：升级斧头兵塔(Axeman)
 * - 左侧按钮：升级弓箭手塔(Archer)
 * - 右侧按钮：升级炮手塔(Gunner)
 *
 * 特性：
 * - 显示各类防御塔的升级成本
 * - 如果防御塔达到最高级，显示"MAX"而非数值
 * - 只有金币充足时才能升级防御塔
 * - 升级后会自动关闭面板
 */
class UpgradePanel : public Panel
{
public:
    /**
     * 构造函数 - 初始化升级面板
     *
     * 从资源管理器加载面板的各种状态纹理
     */
    UpgradePanel()
    {
        // 获取纹理资源池
        const ResourcesManager::TexturePool& texture_pool
            = ResourcesManager::instance()->get_texture_pool();

        // 加载面板各状态纹理
        tex_idle = texture_pool.find(ResID::Tex_UIUpgradeIdle)->second;          // 默认状态纹理
        tex_hovered_top = texture_pool.find(ResID::Tex_UIUpgradeHoveredTop)->second;  // 顶部悬停纹理
        tex_hovered_left = texture_pool.find(ResID::Tex_UIUpgradeHoveredLeft)->second;  // 左侧悬停纹理
        tex_hovered_right = texture_pool.find(ResID::Tex_UIUpgradeHoveredRight)->second;  // 右侧悬停纹理
    }

    // 使用默认析构函数
    ~UpgradePanel() = default;

    /**
     * 更新面板状态
     *
     * 获取各类防御塔的升级成本，供显示和判断使用
     * 如果防御塔达到最高级，升级成本将返回-1
     *
     * @param renderer SDL渲染器，传递给父类更新方法
     */
    void on_update(SDL_Renderer* renderer)
    {
        // 获取塔管理器的单例实例（使用静态变量避免重复获取）
        static TowerManager* instance = TowerManager::instance();

        // 更新各类防御塔的升级成本
        val_top = (int)instance->get_upgrade_cost(TowerType::Axeman);    // 顶部按钮：斧头兵塔升级成本
        val_left = (int)instance->get_upgrade_cost(TowerType::Archer);   // 左侧按钮：弓箭手塔升级成本
        val_right = (int)instance->get_upgrade_cost(TowerType::Gunner);  // 右侧按钮：炮手塔升级成本

        // 调用父类的更新方法以更新文本显示（负值会显示为"MAX"）
        Panel::on_update(renderer);
    }

protected:
    /**
     * 处理点击顶部按钮：升级斧头兵塔
     *
     * 检查金币是否足够且该类型防御塔未达到最高级
     * 如果条件满足，则扣除金币并升级防御塔
     */
    void on_click_top_area() override
    {
        // 获取金币管理器实例
        CoinManager* instance = CoinManager::instance();

        // 检查是否能够升级：val_top > 0表示未达到最高级，后面的条件检查金币是否足够
        if (val_top > 0 && val_top <= instance->get_current_coin_num())
        {
            // 调用防御塔管理器升级斧头兵塔
            TowerManager::instance()->upgrade_tower(TowerType::Axeman);
            // 扣除相应的金币
            instance->decrease_coin(val_top);
        }
    }

    /**
     * 处理点击左侧按钮：升级弓箭手塔
     *
     * 检查金币是否足够且该类型防御塔未达到最高级
     * 如果条件满足，则扣除金币并升级防御塔
     */
    void on_click_left_area() override
    {
        // 获取金币管理器实例
        CoinManager* instance = CoinManager::instance();

        // 检查是否能够升级：val_left > 0表示未达到最高级，后面的条件检查金币是否足够
        if (val_left > 0 && val_left <= instance->get_current_coin_num())
        {
            // 调用防御塔管理器升级弓箭手塔
            TowerManager::instance()->upgrade_tower(TowerType::Archer);
            // 扣除相应的金币
            instance->decrease_coin(val_left);
        }
    }

    /**
     * 处理点击右侧按钮：升级炮手塔
     *
     * 检查金币是否足够且该类型防御塔未达到最高级
     * 如果条件满足，则扣除金币并升级防御塔
     */
    void on_click_right_area() override
    {
        // 获取金币管理器实例
        CoinManager* instance = CoinManager::instance();

        // 检查是否能够升级：val_right > 0表示未达到最高级，后面的条件检查金币是否足够
        if (val_right > 0 && val_right <= instance->get_current_coin_num())
        {
            // 调用防御塔管理器升级炮手塔
            TowerManager::instance()->upgrade_tower(TowerType::Gunner);
            // 扣除相应的金币
            instance->decrease_coin(val_right);
        }
    }
};

#endif // !_UPGRADE_PANEL_H_