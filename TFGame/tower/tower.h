#pragma once  
#include "facing.h"              // 包含表示方向(上下左右)的枚举定义
#include "../vector2.h"             // 包含二维向量类，用于位置和方向
#include "../animation.h"           // 包含动画系统
#include "tower_type.h"             // 包含防御塔类型枚举(弓箭手、斧头兵、炮手)
#include "../manager/enemy_manager.h" // 包含敌人管理器
#include "../bullet/bullet_manager.h" // 包含子弹管理器
#include "../enemy/enemy.h"

/**
 * Tower类 - 塔防游戏中的防御塔基类
 *
 * 负责实现防御塔的基本功能，包括:
 * - 检测攻击范围内的敌人
 * - 发射子弹攻击敌人
 * - 管理攻击冷却时间
 * - 展示攻击和闲置动画
 */
class Tower
{
public:
    /**
     * 构造函数 - 初始化防御塔的基础属性和动画
     *
     * 设置开火计时器和各个方向的动画状态
     */
    Tower()
    {
        // 设置开火计时器为单次触发模式
        timer_fire.set_one_shot(true);
        // 设置计时器超时后的回调函数：重新允许开火
        timer_fire.set_on_timeout(
            [&]()  // Lambda表达式(匿名函数)，捕获this指针
            {
                can_fire = true;  // 计时器超时后，允许塔再次开火
            }
        );

        // 设置各个方向的闲置动画属性
        // 向上闲置动画：循环播放，每帧间隔0.2秒
        anim_idle_up.set_loop(true);
        anim_idle_up.set_interval(0.2);
        // 向下闲置动画：循环播放，每帧间隔0.2秒
        anim_idle_down.set_loop(true);
        anim_idle_down.set_interval(0.2);
        // 向左闲置动画：循环播放，每帧间隔0.2秒
        anim_idle_left.set_loop(true);
        anim_idle_left.set_interval(0.2);
        // 向右闲置动画：循环播放，每帧间隔0.2秒
        anim_idle_right.set_loop(true);
        anim_idle_right.set_interval(0.2);

        // 设置各个方向的开火动画属性
        // 向上开火动画：非循环(播放一次)，每帧间隔0.2秒
        anim_fire_up.set_loop(false);
        anim_fire_up.set_interval(0.2);
        // 开火动画结束后的回调：切换回闲置动画
        anim_fire_up.set_on_finished(
            [&]()
            {
                update_idle_animation();  // 动画完成后切换回闲置动画
            });

        // 向下开火动画：非循环，每帧间隔0.2秒
        anim_fire_down.set_loop(false);
        anim_fire_down.set_interval(0.2);
        // 开火动画结束后的回调：切换回闲置动画
        anim_fire_down.set_on_finished(
            [&]()
            {
                update_idle_animation();
            });

        // 向左开火动画：非循环，每帧间隔0.2秒
        anim_fire_left.set_loop(false);
        anim_fire_left.set_interval(0.2);
        // 开火动画结束后的回调：切换回闲置动画
        anim_fire_left.set_on_finished(
            [&]()
            {
                update_idle_animation();
            });

        // 向右开火动画：非循环，每帧间隔0.2秒
        anim_fire_right.set_loop(false);
        anim_fire_right.set_interval(0.2);
        // 开火动画结束后的回调：切换回闲置动画
        anim_fire_right.set_on_finished(
            [&]()
            {
                update_idle_animation();
            });
    }

    /**
     * 析构函数 - 使用默认实现
     *
     * 由于没有动态分配的资源需要释放，使用编译器生成的默认析构函数
     */
    ~Tower() = default;

    /**
     * 设置防御塔的位置
     * @param position 要设置的新位置（二维向量）
     */
    void set_position(const Vector2& position)
    {
        this->position = position;  // 将传入的位置赋值给成员变量
    }

    /**
     * 获取防御塔的尺寸
     * @return 返回防御塔尺寸的二维向量
     */
    const Vector2& get_size() const
    {
        return size;  // 返回防御塔尺寸
    }

    /**
     * 获取防御塔的位置
     * @return 返回防御塔位置的二维向量
     * 注意：这里有个bug，应该返回position而不是size
     */
    const Vector2& get_position() const
    {
        return size;  // 这里应该是position，而不是size
    }

    /**
     * 更新防御塔状态
     * @param delta 上一帧到当前帧的时间间隔(秒)
     *
     * 更新计时器、动画状态，并检查是否可以开火
     */
    void on_update(double delta)
    {
        timer_fire.on_update(delta);     // 更新开火计时器
        anim_current->on_update(delta);  // 更新当前动画

        if (can_fire) on_fire();  // 如果可以开火，则尝试寻找目标并开火
    }

    /**
     * 渲染防御塔
     * @param renderer SDL渲染器
     *
     * 将当前动画帧绘制到屏幕上
     */
    void on_render(SDL_Renderer* renderer)
    {
        // 声明静态点变量，避免重复创建临时对象
        static SDL_Point point;

        // 计算渲染位置（塔的中心点为基准，向左上偏移半个尺寸）
        point.x = (int)(position.x - size.x / 2);
        point.y = (int)(position.y - size.y / 2);

        // 渲染当前动画帧到计算出的位置
        anim_current->on_render(renderer, point);
    }

protected:
    Vector2 size;  // 防御塔的尺寸

    // 各个方向的闲置动画
    Animation anim_idle_up;     // 向上闲置的动画
    Animation anim_idle_down;   // 向下闲置的动画
    Animation anim_idle_left;   // 向左闲置的动画
    Animation anim_idle_right;  // 向右闲置的动画

    // 各个方向的开火动画
    Animation anim_fire_up;     // 向上开火的动画
    Animation anim_fire_down;   // 向下开火的动画
    Animation anim_fire_left;   // 向左开火的动画
    Animation anim_fire_right;  // 向右开火的动画

    TowerType tower_type = TowerType::Archer;  // 防御塔类型，默认为弓箭手

    double fire_speed = 0;  // 子弹发射速度
    BulletType bullet_type = BulletType::Arrow;  // 子弹类型，默认为箭矢

private:
    Timer timer_fire;  // 控制开火冷却的计时器
    Vector2 position;  // 防御塔的位置
    bool can_fire = true;  // 是否可以开火的标志
    Facing facing = Facing::Right;  // 防御塔当前朝向，默认向右
    Animation* anim_current = &anim_idle_right;  // 当前播放的动画，默认为向右闲置

private:
    /**
     * 更新闲置动画
     *
     * 根据当前朝向，将当前动画设置为对应方向的闲置动画
     */
    void update_idle_animation()
    {
        // 根据当前朝向选择对应的闲置动画
        switch (facing)
        {
        case Left:
            anim_current = &anim_idle_left;  // 朝左时使用向左闲置动画
            break;
        case Right:
            anim_current = &anim_idle_right; // 朝右时使用向右闲置动画
            break;
        case Up:
            anim_current = &anim_idle_up;    // 朝上时使用向上闲置动画
            break;
        case Down:
            anim_current = &anim_idle_down;  // 朝下时使用向下闲置动画
            break;
        }
    }

    /**
     * 更新开火动画
     *
     * 根据当前朝向，将当前动画设置为对应方向的开火动画
     */
    void update_fire_animation()
    {
        // 根据当前朝向选择对应的开火动画
        switch (facing)
        {
        case Left:
            anim_current = &anim_fire_left;  // 朝左时使用向左开火动画
            break;
        case Right:
            anim_current = &anim_fire_right; // 朝右时使用向右开火动画
            break;
        case Up:
            anim_current = &anim_fire_up;    // 朝上时使用向上开火动画
            break;
        case Down:
            anim_current = &anim_fire_down;  // 朝下时使用向下开火动画
            break;
        }
    }

    /**
     * 寻找目标敌人
     * @return 返回找到的目标敌人指针，如果没找到则返回nullptr
     *
     * 在攻击范围内寻找最靠近终点的敌人作为目标
     */
    Enemy* find_target_enemy()
    {
        double process = -1;  // 记录当前找到的最大路径进度
        double view_range = 0;  // 视野范围
        Enemy* enemy_target = nullptr;  // 目标敌人指针，初始为空

        // 获取配置管理器实例（使用静态变量避免重复获取）
        static ConfigManager* instance = ConfigManager::instance();

        // 根据塔类型获取对应的视野范围
        switch (tower_type)
        {
        case Archer:
            view_range = instance->archer_template.view_range[instance->level_archere];
            break;
        case Axeman:
            view_range = instance->axeman_template.view_range[instance->level_axeman];
            break;
        case Gunner:
            view_range = instance->gunner_template.view_range[instance->level_gunner];
            break;
        }

        // 获取敌人列表
        EnemyManager::EnemyList& enemy_list = EnemyManager::instance()->get_enemy_list();

        // 遍历所有敌人，寻找在视野范围内且路径进度最大的敌人
        for (Enemy* enemy : enemy_list)
        {
            // 计算敌人到防御塔的距离，并检查是否在视野范围内
            if ((enemy->get_position() - position).length() <= view_range * SIZE_TILE)
            {
                // 获取敌人的路径进度
                double new_process = enemy->get_route_process();
                // 如果这个敌人的进度更靠近终点（值更大），则更新目标
                if (new_process > process)
                {
                    enemy_target = enemy;
                    process = new_process;
                }
            }
        }

        return enemy_target;  // 返回找到的目标敌人（或nullptr）
    }

    /**
     * 发射攻击
     *
     * 寻找目标敌人，计算攻击方向，播放攻击动画和音效，
     * 发射子弹并设置冷却时间
     */
    void on_fire()
    {
        // 寻找目标敌人
        Enemy* target_enemy = find_target_enemy();

        // 如果没有找到目标敌人，则直接返回
        if (!target_enemy) return;

        // 设置不能开火状态（进入冷却）
        can_fire = false;

        // 获取配置和资源管理器实例（使用静态变量避免重复获取）
        static ConfigManager* instance = ConfigManager::instance();
        static const ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();

        // 根据塔类型获取攻击间隔和伤害值，并播放对应音效
        double interval = 0, damage = 0;
        switch (tower_type)
        {
        case Archer:  // 弓箭手塔
            // 获取弓箭手攻击间隔和伤害值
            interval = instance->archer_template.interval[instance->level_archere];
            damage = instance->archer_template.damage[instance->level_archere];
            // 随机播放两种弓箭发射音效之一
            switch (rand() % 2)
            {
            case 0:
                Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowFire_1)->second, 0);
                break;
            case 1:
                Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowFire_2)->second, 0);
                break;
            }
            break;
        case Axeman:  // 斧头塔
            // 获取斧头塔攻击间隔和伤害值
            interval = instance->axeman_template.interval[instance->level_axeman];
            damage = instance->axeman_template.damage[instance->level_axeman];
            // 播放斧头投掷音效
            Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_AxeFire)->second, 0);
            break;
        case Gunner:  // 炮塔
            // 获取炮塔攻击间隔和伤害值
            interval = instance->gunner_template.interval[instance->level_gunner];
            damage = instance->gunner_template.damage[instance->level_gunner];
            // 播放炮弹发射音效
            Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ShellFire)->second, 0);
            break;
        }

        // 设置开火计时器的等待时间和重启计时器
        timer_fire.set_wait_time(interval);
        timer_fire.restart();

        // 计算从防御塔到目标敌人的方向向量
        Vector2 direction = target_enemy->get_position() - position;

        // 通过子弹管理器发射子弹
        // 参数：子弹类型、起始位置、速度向量(归一化后乘以速度和格子大小)、伤害值
        BulletManager::instance()->fire_bullet(bullet_type, position, direction.normalize() * fire_speed * SIZE_TILE, damage);

        // 判断应该显示水平还是垂直方向的动画（根据方向向量的分量大小）
        bool is_show_x_anim = abs(direction.x) >= abs(direction.y);
        if (is_show_x_anim)
            // 如果水平分量更大，根据x正负判断朝向左还是右
            facing = direction.x > 0 ? Facing::Right : Facing::Left;
        else
            // 如果垂直分量更大，根据y正负判断朝向上还是下
            facing = direction.y > 0 ? Facing::Down : Facing::Up;

        // 更新开火动画和重置动画状态
        update_fire_animation();
        anim_current->reset();
    }
};