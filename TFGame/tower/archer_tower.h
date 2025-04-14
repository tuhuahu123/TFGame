#pragma once  

// 包含基类Tower和资源管理器的头文件
#include "tower.h"  // 引入防御塔基类
#include "../manager/resources_manager.h"  // 引入资源管理器，用于获取纹理资源

/**
 * ArcherTower类 - 弓箭手防御塔
 *
 * 继承自Tower基类，实现弓箭手防御塔的特定属性和行为
 * 主要特点：攻击范围较大，攻击速度较快，伤害适中
 */
class ArcherTower : public Tower
{
public:
    /**
     * 构造函数 - 初始化弓箭手防御塔的特定属性
     *
     * 设置动画帧、尺寸、塔类型和攻击参数
     */
    ArcherTower()
    {
        // 获取弓箭手纹理资源（使用static保证只获取一次，提高性能）
        static SDL_Texture* tex_archer = ResourcesManager::instance()
            ->get_texture_pool().find(ResID::Tex_Archer)->second;

        // 定义各个动画状态对应的精灵表中的帧索引
        // 闲置状态的动画帧索引（上、下、左、右四个方向）
        static const std::vector<int> idx_list_idle_up = { 3, 4 };        // 向上闲置动画使用第3和第4帧
        static const std::vector<int> idx_list_idle_down = { 0, 1 };      // 向下闲置动画使用第0和第1帧
        static const std::vector<int> idx_list_idle_left = { 6, 7 };      // 向左闲置动画使用第6和第7帧
        static const std::vector<int> idx_list_idle_right = { 9, 10 };    // 向右闲置动画使用第9和第10帧

        // 攻击状态的动画帧索引（上、下、左、右四个方向）
        static const std::vector<int> idx_list_fire_up = { 15, 16, 17 };     // 向上攻击动画使用第15、16和17帧
        static const std::vector<int> idx_list_fire_down = { 12, 13, 14 };   // 向下攻击动画使用第12、13和14帧
        static const std::vector<int> idx_list_fire_left = { 18, 19, 20 };   // 向左攻击动画使用第18、19和20帧
        static const std::vector<int> idx_list_fire_right = { 21, 22, 23 };  // 向右攻击动画使用第21、22和23帧

        // 为各个方向的动画设置帧数据
        // 参数说明：纹理、精灵表水平分割数(3)、精灵表垂直分割数(8)、帧索引列表
        anim_idle_up.set_frame_data(tex_archer, 3, 8, idx_list_idle_up);       // 设置向上闲置动画
        anim_idle_down.set_frame_data(tex_archer, 3, 8, idx_list_idle_down);   // 设置向下闲置动画
        anim_idle_left.set_frame_data(tex_archer, 3, 8, idx_list_idle_left);   // 设置向左闲置动画
        anim_idle_right.set_frame_data(tex_archer, 3, 8, idx_list_idle_right); // 设置向右闲置动画

        anim_fire_up.set_frame_data(tex_archer, 3, 8, idx_list_fire_up);       // 设置向上攻击动画
        anim_fire_down.set_frame_data(tex_archer, 3, 8, idx_list_fire_down);   // 设置向下攻击动画
        anim_fire_left.set_frame_data(tex_archer, 3, 8, idx_list_fire_left);   // 设置向左攻击动画
        anim_fire_right.set_frame_data(tex_archer, 3, 8, idx_list_fire_right); // 设置向右攻击动画

        // 设置弓箭手防御塔的尺寸（像素）
        size.x = 48, size.y = 48;  // 宽度和高度都是48像素

        // 设置塔的类型为弓箭手
        tower_type = TowerType::Archer;  // 使用枚举类型标识为弓箭手塔

        // 设置攻击属性
        fire_speed = 6;  // 子弹飞行速度
        bullet_type = BulletType::Arrow;  // 使用箭矢类型的子弹
    }

    /**
     * 析构函数 - 使用默认实现
     *
     * 由于没有动态分配的资源，使用编译器生成的默认析构函数
     */
    ~ArcherTower() = default;
};
