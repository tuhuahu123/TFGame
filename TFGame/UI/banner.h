#ifndef _BANNER_H_    // 头文件保护宏，防止重复包含
#define _BANNER_H_

// 包含必要的头文件
#include "../timer.h"             // 计时器类，用于控制横幅显示时间
#include "../vector2.h"           // 二维向量类，用于位置和尺寸
#include "../manager/config_manager.h"    // 配置管理器，获取游戏状态
#include "../manager/resources_manager.h" // 资源管理器，获取纹理资源

#include <SDL.h>                  // SDL库，用于图形渲染

// Banner类：用于显示游戏结束时的胜利/失败横幅
class Banner
{
public:
    // 构造函数：初始化横幅尺寸和显示计时器
    Banner()
    {
        // 设置前景（文字）尺寸
        size_foreground = { 646, 215 };
        // 设置背景（条幅）尺寸
        size_background = { 1282, 209 };

        // 配置显示计时器
        timer_display.set_one_shot(true);        // 设置为单次触发
        timer_display.set_wait_time(5);          // 设置显示时间为5秒
        timer_display.set_on_timeout(            // 设置计时器超时回调
            [&]()
            {
                is_end_display = true;           // 标记显示结束
            });
    }

    // 析构函数（使用默认实现）
    ~Banner() = default;

    // 设置横幅在屏幕上的中心位置
    void set_center_position(const Vector2& pos)
    {
        pos_center = pos;
    }

    // 更新方法：更新计时器和纹理
    void on_update(double delta)
    {
        // 更新显示计时器
        timer_display.on_update(delta);

        // 获取纹理资源池
        const ResourcesManager::TexturePool& tex_pool
            = ResourcesManager::instance()->get_texture_pool();
        // 获取配置管理器实例
        const ConfigManager* instance = ConfigManager::instance();

        // 根据游戏结果选择对应的文字纹理（胜利/失败）
        tex_foreground = tex_pool.find(instance->is_game_win ? ResID::Tex_UIWinText : ResID::Tex_UILossText)->second;
        // 获取背景条幅纹理
        tex_background = tex_pool.find(ResID::Tex_UIGameOverBar)->second;
    }

    // 渲染方法：绘制横幅到屏幕
    void on_render(SDL_Renderer* renderer)
    {
        // 定义静态目标矩形，用于指定渲染位置和大小
        static SDL_Rect rect_dst;

        // 计算并设置背景条幅的渲染区域
        rect_dst.x = (int)(pos_center.x - size_background.x / 2);  // 水平居中
        rect_dst.y = (int)(pos_center.y - size_background.y / 2);  // 垂直居中
        rect_dst.w = (int)size_background.x, rect_dst.h = (int)size_background.y;
        // 渲染背景条幅
        SDL_RenderCopy(renderer, tex_background, nullptr, &rect_dst);

        // 计算并设置前景文字的渲染区域
        rect_dst.x = (int)(pos_center.x - size_foreground.x / 2);  // 水平居中
        rect_dst.y = (int)(pos_center.y - size_foreground.y / 2);  // 垂直居中
        rect_dst.w = (int)size_foreground.x, rect_dst.h = (int)size_foreground.y;
        // 渲染前景文字
        SDL_RenderCopy(renderer, tex_foreground, nullptr, &rect_dst);
    }

    // 检查横幅是否已完成显示
    bool check_end_dispaly()
    {
        return is_end_display;
    }

private:
    Vector2 pos_center;           // 横幅中心位置

    Vector2 size_foreground;      // 前景（文字）尺寸
    Vector2 size_background;      // 背景（条幅）尺寸

    SDL_Texture* tex_foreground = nullptr;  // 前景纹理（胜利/失败文字）
    SDL_Texture* tex_background = nullptr;  // 背景纹理（条幅）

    Timer timer_display;          // 显示计时器
    bool is_end_display = false;  // 显示完成标志

};

#endif // !_BANNER_H_