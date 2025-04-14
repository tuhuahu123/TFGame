#pragma once                // 确保此头文件在编译过程中只被包含一次

#include "enemy.h"          // 包含基类Enemy的定义
#include "manager/config_manager.h" // 包含配置管理器，用于获取敌人属性
#include "manager/resources_manager.h" // 包含资源管理器，用于获取纹理资源

/**
 * SlimEnemy类 - 史莱姆敌人
 *
 * 继承自Enemy基类，实现了特定类型敌人(史莱姆)的初始化
 * 设置了史莱姆的动画、属性和行为特性
 */
class SlimEnemy : public Enemy
{
public:
    // 构造函数：初始化史莱姆敌人的所有属性和动画
    SlimEnemy()
    {
        // 获取纹理资源池(使用静态变量提高效率，避免重复获取)
        static const ResourcesManager::TexturePool& texture_pool
            = ResourcesManager::instance()->get_texture_pool();
        // 获取史莱姆普通状态纹理
        static SDL_Texture* tex_slime = texture_pool.find(ResID::Tex_Slime)->second;
        // 获取史莱姆受击状态纹理(草图效果)
        static SDL_Texture* tex_slime_sketch = texture_pool.find(ResID::Tex_SlimeSketch)->second;
        // 获取史莱姆配置模板，包含各项属性值
        static ConfigManager::EnemyTemplate& slim_template = ConfigManager::instance()->slim_template;

        // 定义各方向动画的帧索引序列 3×4序列帧
        // 这些索引对应精灵表中的具体位置(按行优先顺序)
        static const std::vector<int> idx_list_down = { 0, 1, 2 };     // 向下移动的3帧动画
        static const std::vector<int> idx_list_left = { 3, 4, 5 };     // 向左移动的3帧动画
        static const std::vector<int> idx_list_right = { 6, 7, 8 };    // 向右移动的3帧动画
        static const std::vector<int> idx_list_up = { 9, 10, 11 };     // 向上移动的3帧动画

        // 设置向下移动的普通动画
        anim_down.set_loop(true);                           // 设置为循环播放
        anim_down.set_interval(0.15);                       // 设置帧间隔为0.15秒
        anim_down.set_frame_data(tex_slime, 3, 4, idx_list_down); // 设置动画帧数据(纹理、3列4行、帧索引列表)

        // 设置向左移动的普通动画
        anim_left.set_loop(true);
        anim_left.set_interval(0.15);
        anim_left.set_frame_data(tex_slime, 3, 4, idx_list_left);

        // 设置向右移动的普通动画
        anim_right.set_loop(true);
        anim_right.set_interval(0.15);
        anim_right.set_frame_data(tex_slime, 3, 4, idx_list_right);

        // 设置向上移动的普通动画
        anim_up.set_loop(true);
        anim_up.set_interval(0.15);
        anim_up.set_frame_data(tex_slime, 3, 4, idx_list_up);

        // 设置向下移动的受击动画(使用草图纹理)
        anim_down_sketch.set_loop(true);
        anim_down_sketch.set_interval(0.15);
        anim_down_sketch.set_frame_data(tex_slime_sketch, 3, 4, idx_list_down);

        // 设置向左移动的受击动画(使用草图纹理)
        anim_left_sketch.set_loop(true);
        anim_left_sketch.set_interval(0.15);
        anim_left_sketch.set_frame_data(tex_slime_sketch, 3, 4, idx_list_left);

        // 设置向右移动的受击动画(使用草图纹理)
        anim_right_sketch.set_loop(true);
        anim_right_sketch.set_interval(0.15);
        anim_right_sketch.set_frame_data(tex_slime_sketch, 3, 4, idx_list_right);

        // 设置向上移动的受击动画(使用草图纹理)
        anim_up_sketch.set_loop(true);
        anim_up_sketch.set_interval(0.15);
        anim_up_sketch.set_frame_data(tex_slime_sketch, 3, 4, idx_list_up);

        // 从配置模板中初始化敌人属性
        max_hp = slim_template.health;                 // 最大生命值
        max_speed = slim_template.speed;               // 最大移动速度
        damage = slim_template.damage;                 // 攻击伤害
        reward_ratio = slim_template.reward_ratio;     // 击杀奖励比例
        recover_interval = slim_template.recover_interval; // 治疗技能冷却时间
        recover_range = slim_template.recover_range;       // 治疗技能范围
        recover_intensity = slim_template.recover_intensity; // 治疗技能强度

        // 设置敌人尺寸为64x64像素
        size.x = 64, size.y = 64;
        // 初始化当前生命值和速度为最大值
        hp = max_hp, speed = max_speed;
    }

    // 析构函数：使用默认实现，因为没有需要手动释放的资源
    ~SlimEnemy() = default;

}; // 类定义结束