#pragma once

#include "vector2.h"        // 包含向量类定义，用于位置和速度
#include "enemy/enemy.h"    // 包含敌人类定义，用于碰撞检测
#include "animation.h"      // 包含动画类定义，用于子弹的视觉效果
#include "manager/config_manager.h" // 包含配置管理器，用于获取地图边界

/**
 * Bullet类 - 游戏中的子弹基类
 *
 * 定义了所有子弹的基本属性和行为，包括：
 * 1. 移动逻辑：根据速度向量更新位置
 * 2. 碰撞检测：与敌人的碰撞处理
 * 3. 边界检查：子弹超出地图范围时失效
 * 4. 动画渲染：支持旋转的动画效果
 */
class Bullet
{
public:
    // 默认构造函数
    Bullet() = default;
    // 默认析构函数
    ~Bullet() = default;

    /**
     * 设置子弹的速度向量
     * @param velocity 速度向量，决定子弹的移动方向和速度
     */
    void set_velocity(const Vector2& velocity)
    {
        this->velocity = velocity;

        // 如果子弹可旋转，则根据速度方向计算旋转角度
        if (can_rotated)
        {
            double randian = std::atan2(velocity.y, velocity.x); // 计算速度向量的弧度
            angle_anim_rotated = randian * 180 / 3.14159265;     // 将弧度转换为角度
        }
    }

    /**
     * 设置子弹的位置
     * @param position 子弹的位置向量
     */
    void set_position(const Vector2& position)
    {
        this->position = position;
    }

    /**
     * 设置子弹的伤害值
     * @param damage 子弹对敌人造成的伤害
     */
    void set_damage(double damage)
    {
        this->damage = damage;
    }

    /**
     * 获取子弹的尺寸
     * @return 子弹的尺寸向量(宽度和高度)
     */
    const Vector2& get_size() const
    {
        return size;
    }

    /**
     * 获取子弹的当前位置
     * @return 子弹的位置向量
     */ 
    const Vector2& get_position() const
    {
        return position;
    }

    /**
     * 获取子弹的伤害值
     * @return 子弹对敌人造成的伤害
     */
    double get_damage() const
    {
        return damage;
    }

    /**
     * 获取子弹的伤害范围
     * @return 范围伤害的半径，-1表示无范围伤害
     */
    double get_damage_range() const
    {
        return damage_range;
    }

    /**
     * 禁用子弹的碰撞功能
     * 使子弹不再与敌人发生碰撞，通常在子弹命中目标后调用
     */
    void disable_collide()
    {
        is_collisional = false;
    }

    /**
     * 检查子弹是否可以与敌人碰撞
     * @return 如果子弹可以碰撞返回true，否则返回false
     */
    bool can_collide() const
    {
        return is_collisional;
    }

    /**
     * 使子弹失效，准备移除
     * 同时禁用碰撞功能，防止产生额外的碰撞
     */
    void make_invalid()
    {
        is_valid = false;
        is_collisional = false;
    }

    /**
     * 检查子弹是否可以被移除
     * @return 如果子弹已失效返回true，表示可以从游戏中移除
     */
    bool can_remove() const
    {
        return !is_valid;
    }

    /**
     * 更新子弹状态
     * @param delta 帧时间间隔(秒)
     * 更新子弹动画、位置，并检查是否超出地图边界
     */
    virtual void on_update(double delta)
    {
        animation.on_update(delta);            // 更新子弹动画
        position += velocity * delta;          // 根据速度更新位置

        // 获取地图边界信息
        static const SDL_Rect& rect_map
            = ConfigManager::instance()->rect_tile_map;

        // 检查子弹是否超出地图边界，如果是则使其失效
        if (position.x - size.x / 2 <= rect_map.x
            || position.x + size.x / 2 >= rect_map.x + rect_map.w
            || position.y - size.y / 2 <= rect_map.y
            || position.y + size.y / 2 >= rect_map.y + rect_map.h)
        {
            is_valid = false; 
        }
    }

    /**
     * 渲染子弹
     * @param renderer SDL渲染器
     * 在屏幕上绘制子弹的动画，支持旋转
     */
    virtual void on_render(SDL_Renderer* renderer)
    {
        static SDL_Point point;                // 渲染位置点(使用静态变量避免频繁创建)

        // 计算渲染位置(左上角坐标)，考虑子弹的中心点
        point.x = (int)(position.x - size.x / 2);
        point.y = (int)(position.y - size.y / 2);

        // 渲染子弹动画，应用旋转角度
        animation.on_render(renderer, point, angle_anim_rotated);
    }

    /**
     * 处理与敌人的碰撞事件
     * @param enemy 碰撞的敌人指针
     * 基类实现中，碰撞后子弹失效并禁用碰撞功能
     */
    virtual void on_collide(Enemy* enemy)
    {
        is_valid = false;           // 子弹失效
        is_collisional = false;     // 禁用碰撞功能
    }

protected:
    Vector2 size;           // 子弹尺寸
    Vector2 velocity;       // 子弹速度向量
    Vector2 position;       // 子弹位置

    Animation animation;      // 子弹动画
    bool can_rotated = false; // 是否可以旋转(例如箭矢会根据方向旋转)

    double damage = 0;      // 子弹伤害值
    double damage_range = -1; // 伤害范围半径，-1表示无范围伤害

private:
    bool is_valid = true;       // 子弹是否有效，无效子弹将被移除
    bool is_collisional = true; // 子弹是否可以碰撞
    double angle_anim_rotated = 0; // 子弹动画的旋转角度(度)

}; // 类定义结束
