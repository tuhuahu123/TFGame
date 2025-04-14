#pragma once
// 包含所需的头文件
#include "manager/manager.h"        // 包含管理器模板类，用于实现单例模式
#include "config_manager.h"  // 包含配置管理器，用于获取初始生命值
#include "resources_manager.h" // 包含资源管理器，用于加载音效






// HomeManager 类定义，继承自 Manager<HomeManager>，使用单例模式
class HomeManager : public Manager<HomeManager>
{
    // 声明 Manager<HomeManager> 为友元类，使其能访问本类的保护成员
    friend class Manager<HomeManager>;

public:
    // 获取当前基地生命值的方法
    double get_current_hp_num()
    {
        return num_hp;  // 返回当前生命值
    }

    // 减少基地生命值的方法
    void decrease_hp(double val)
    {
        num_hp -= val;  

        if (num_hp < 0)  // 防止生命值为负数
            num_hp = 0;  // 如果生命值小于0，则设为0

        // 创建静态常量引用，获取游戏中所有音效资源
        static const ResourcesManager::SoundPool& sound_pool
            = ResourcesManager::instance()->get_sound_pool();

        // 播放基地受伤音效，-1表示使用第一个可用的声道，0表示不循环播放
        Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_HomeHurt)->second, 0);
    }

protected:
    // 构造函数，被设为保护类型防止直接实例化（单例模式的要求）
    HomeManager()
    {
        // 初始化基地生命值，从配置管理器中获取初始值
        num_hp = ConfigManager::instance()->num_initial_hp;
    }

    // 析构函数，使用默认实现
    ~HomeManager() = default;

private:
    double num_hp = 0;  // 存储基地当前生命值的私有成员变量，初始值为0（构造函数中会重新赋值）
};


