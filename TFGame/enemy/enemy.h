
#pragma once
#include "timer.h"          // 定时器类，用于控制技能和动画
#include "route.h"          // 路径类，定义敌人移动路线
#include "vector2.h"        // 二维向量，用于位置和方向计算
#include "animation.h"      // 动画类，处理敌人的视觉效果
#include "manager/config_manager.h" // 配置管理器，获取全局设置

#include <functional>       // 使用std::function实现回调

/**
 * Enemy类 - 塔防游戏中的敌人单位
 *
 * 功能：
 * 1. 路径跟随：沿预定义路线从起点移动到终点(玩家基地)
 * 2. 状态管理：处理生命值、移动速度和有效性状态
 * 3. 动画系统：根据移动方向和状态(普通/受击)显示不同动画
 * 4. 特殊技能：某些敌人可以释放技能(如治疗周围单位)
 * 5. 效果反馈：显示受击效果、血量条等视觉反馈
 * 6. 减速机制：可被减速，并在一段时间后恢复原速度
 */

class Enemy
{
public:
    // 敌人技能触发时的回调函数类型
    typedef std::function<void(Enemy* enemy)> SkillCallback;

public:
    // 构造函数：初始化敌人的计时器和状态
    Enemy()
    {
        timer_skill.set_one_shot(false);  // 技能计时器设为循环触发
        timer_skill.set_on_timeout([&]() { on_skill_released(this); });  // 设置技能触发回调

        timer_sketch.set_one_shot(true);  // 受击效果计时器设为单次触发
        timer_sketch.set_wait_time(0.075);  // 受击效果显示时间
        timer_sketch.set_on_timeout([&]() { is_show_sketch = false; });  // 关闭受击效果

        timer_restore_speed.set_one_shot(true);  // 速度恢复计时器设为单次触发
        timer_restore_speed.set_on_timeout([&]() { speed = max_speed; });  // 恢复正常速度
    }

    // 析构函数
    ~Enemy() = default;

    // 更新敌人状态
    // delta: 帧时间间隔(秒)
    void on_update(double delta)
    {
        // 更新所有计时器
        timer_skill.on_update(delta);   
        timer_sketch.on_update(delta);  
        timer_restore_speed.on_update(delta);

        // 计算移动距离和目标距离
        Vector2 move_distance = velocity * delta;
        Vector2 target_distance = position_target - position;
        // 移动敌人，确保不会超过目标点
        position += move_distance < target_distance ? move_distance : target_distance;

        // 如果到达当前目标点
        if (target_distance.approx_zero())
        {
            idx_target++;  // 移向路径的下一个点
            refresh_position_target();  // 刷新目标位置

            // 计算新的移动方向
            direction = (position_target - position).normalize();
        }

        // 根据方向和速度更新速度向量
        velocity.x = direction.x * speed * SIZE_TILE;
        velocity.y = direction.y * speed * SIZE_TILE;

        // 确定动画方向(水平或垂直)
        bool is_show_x_amin = abs(velocity.x) >= abs(velocity.y);
        
      
        if (is_show_sketch)  // 显示受击剪影
        {
            if (is_show_x_amin)  // 水平方向移动
                anim_current = velocity.x > 0 ? &anim_right_sketch : &anim_left_sketch;
            else  // 垂直方向移动
                anim_current = velocity.y > 0 ? &anim_down_sketch : &anim_up_sketch;
        }
        else  // 正常显示时
        {
            if (is_show_x_amin)  // 水平方向移动
                anim_current = velocity.x > 0 ? &anim_right : &anim_left;
            else  // 垂直方向移动
                anim_current = velocity.y > 0 ? &anim_down : &anim_up;
        }

        // 更新当前动画
        anim_current->on_update(delta);
    }

    // 渲染敌人HP
    // renderer: SDL渲染器
    void on_render(SDL_Renderer* renderer)
    {
        // 静态变量减少渲染时的内存分配
		static SDL_Rect rect;           // 用于绘制血条的矩形
		static SDL_Point point;         // 用于绘制敌人的位置
        static const int offset_y = 2;  // 血条与角色的间隔像素
        static const Vector2 size_hp_bar = { 50, 9 };  // 血条大小
        static const SDL_Color color_border = { 116, 185, 124, 255 };  // 血条边框颜色
        static const SDL_Color color_content = { 226, 255, 194, 255 };  // 血条填充颜色

        // 计算敌人的渲染位置(中心点转左上角)
        point.x = (int)(position.x - size.x / 2);
        point.y = (int)(position.y - size.y / 2);

        // 渲染敌人动画
        anim_current->on_render(renderer, point);

        // 只在血量低于最大值时显示血条
        if (hp < max_hp)
        {
            // 设置血条位置和大小
            rect.x = (int)(position.x - size_hp_bar.x / 2);  // 血条水平居中
            rect.y = (int)(position.y - size.y / 2 - size_hp_bar.y - offset_y);  // 血条在敌人上方
            rect.w = (int)(size_hp_bar.x * (hp / max_hp));  // 血条宽度根据血量百分比调整
            rect.h = (int)size_hp_bar.y;  // 血条高度固定

            // 绘制血条填充部分
            SDL_SetRenderDrawColor(renderer, color_content.r, color_content.g, color_content.b, color_content.a);
            SDL_RenderFillRect(renderer, &rect);

            // 绘制血条边框
            rect.w = (int)size_hp_bar.x;  // 边框宽度固定
            SDL_SetRenderDrawColor(renderer, color_border.r, color_border.g, color_border.b, color_border.a);
            SDL_RenderDrawRect(renderer, &rect);
        }
    }

    // 设置技能释放回调
    // on_skill_released: 敌人释放技能时调用的函数
    void set_on_skill_released(SkillCallback on_skill_released)
    {
        this->on_skill_released = on_skill_released;
    }

    // 增加血量
    // val: 要增加的血量值
    void increase_hp(double val)
    {
        hp += val;  // 增加血量

        // 限制最大血量
        if (hp > max_hp)
            hp = max_hp;
    }

    // 减少血量
    // val: 要减少的血量值
    void decrease_hp(double val)
    {
        hp -= val;  // 减少血量

        // 检查是否死亡
        if (hp <= 0)
        {
            hp = 0;
            is_valid = false;  // 标记敌人为无效(已死亡)
        }

        // 显示受击特效
        //is_show_sketch = true;
        //timer_sketch.restart();  // 启动受击特效计时器
    }

    // 减速效果
    void slow_down()
    {
        speed = max_speed - 0.5;  // 降低速度
        timer_restore_speed.set_wait_time(1);  // 设置恢复时间
        timer_restore_speed.restart();  // 启动速度恢复计时器
    }

    // 设置敌人位置
    // position: 新位置
    void set_position(const Vector2& position)
    {
        this->position = position;
    }

    // 设置敌人移动路径
    // route: 路径对象
    void set_route(const Route* route)
    {
        this->route = route;
        refresh_position_target();  // 刷新目标位置
    }

    // 使敌人失效(死亡)
    void make_invalid()
    {
        is_valid = false;
    }

    // 获取当前血量
    double get_hp() const
    {
        return hp;
    }

    // 获取敌人尺寸
    const Vector2& get_size() const
    {
        return size;
    }

    // 获取当前位置
    const Vector2& get_position() const
    {
        return position;
    }

    // 获取当前速度向量
    const Vector2& get_velocity() const
    {
        return velocity;
    }

    // 获取敌人造成的伤害
    double get_damage() const
    {
        return damage;
    }

    // 获取击杀奖励比率
    double get_reward_ratio() const
    {
        return reward_ratio;
    }

    // 获取治疗范围
    double get_recover_radius() const
    {
        return SIZE_TILE * recover_range;  // 转换为像素单位
    }

    // 获取治疗强度
    double get_recover_intensity() const
    {
        return recover_intensity;
    }

    // 检查敌人是否可以移除
    bool can_remove() const
    {
        return !is_valid;  // 无效敌人可以移除
    }

    // 获取路径完成度
    double get_route_process() const
    {
        if (route->get_idx_list().size() == 1)
            return 1;  // 只有一个路径点时，直接返回1

        // 计算完成百分比
        return (double)idx_target / (route->get_idx_list().size() - 1);
    }

protected:
    // 派生类可访问的成员
    Vector2 size;  // 敌人尺寸

    Timer timer_skill;  // 技能计时器

    // 各方向的正常和受击动画
    Animation anim_up;
    Animation anim_down;
    Animation anim_left;
    Animation anim_right;
    Animation anim_up_sketch;
    Animation anim_down_sketch;
    Animation anim_left_sketch;
    Animation anim_right_sketch;

    // 敌人属性
    double hp = 0;                  // 当前血量
    double max_hp = 0;              // 最大血量
    double speed = 0;               // 当前移动速度
    double max_speed = 0;           // 最大移动速度
    double damage = 0;              // 对玩家造成的伤害
    double reward_ratio = 0;        // 击杀奖励比率
    double recover_interval = 0;    // 治疗间隔
    double recover_range = 0;       // 治疗范围
    double recover_intensity = 0;   // 治疗强度

private:
    // 位置和运动相关
    Vector2 position;        // 当前位置
    Vector2 velocity;        // 当前速度
    Vector2 direction;       // 当前方向

    bool is_valid = true;    // 敌人是否有效

    // 受击特效相关
    Timer timer_sketch;            // 受击特效计时器
    bool is_show_sketch = false;  // 是否显示受击特效

    Animation* anim_current = nullptr;  // 当前使用的动画

    SkillCallback on_skill_released;  // 技能回调函数

    Timer timer_restore_speed;  // 速度恢复计时器

    // 路径和目标位置
    const Route* route = nullptr;  // 敌人的移动路径
    int idx_target = 0;           // 当前目标点索引
    Vector2 position_target;      // 当前目标位置

private:
     // 刷新目标位置，在到达路径点或设置新路径时调用
     //本函数将路径中的网格坐标转换为对应的屏幕像素坐标，
     //并考虑了地图在屏幕上的偏移以及格子的中心点位置。
    void refresh_position_target()
    {
        const Route::IdxList& idx_list = route->get_idx_list();  // 获取路径点列表

        if (idx_target < idx_list.size())  // 确保目标点有效
        {
            const SDL_Point& point = idx_list[idx_target];  // 获取目标网格坐标
            static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;  // 获取地图位置

            // 计算目标点的屏幕像素坐标(网格坐标转世界坐标)
            position_target.x = rect_tile_map.x + point.x * SIZE_TILE + SIZE_TILE / 2;
            position_target.y = rect_tile_map.y + point.y * SIZE_TILE + SIZE_TILE / 2;
        }
    }
};

