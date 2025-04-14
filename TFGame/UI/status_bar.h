#ifndef _STATUS_BAR_H_
#define _STATUS_BAR_H_

#include "../coin/coin_manager.h"     // 引入金币管理器，用于获取当前金币数量
#include "../manager/home_manager.h"     // 引入基地管理器，用于获取基地血量
#include "../manager/resources_manager.h" // 引入资源管理器，用于获取游戏资源(图片、字体等)
#include "../manager/player_manager.h"   // 引入玩家管理器，用于获取玩家魔法值(MP)

#include <SDL.h>             // 引入SDL库，提供图形绘制功能
#include <string>            // 引入字符串库，用于文本处理
#include <SDL2_gfxPrimitives.h> // 引入SDL图形原语库，用于绘制圆角矩形等

/**
 * StatusBar类 - 游戏界面上的状态栏
 *
 * 功能：
 * 1. 显示基地血量 - 通过心形图标显示当前基地剩余血量
 * 2. 显示金币数量 - 展示玩家当前拥有的金币
 * 3. 显示玩家头像 - 展示玩家角色头像
 * 4. 显示魔法值条 - 展示玩家当前魔法值
 *
 * 使用方法：
 * 1. 创建StatusBar对象
 * 2. 设置状态栏位置 set_position()
 * 3. 在游戏循环中调用 on_update() 更新状态
 * 4. 在游戏渲染循环中调用 on_render() 显示状态栏
 */
class StatusBar
{
public:
	/**
	 * 构造函数 - 使用默认实现创建状态栏对象
	 */
	StatusBar() = default;

	/**
	 * 析构函数 - 使用默认实现清理状态栏对象
	 */
	~StatusBar() = default;

	/**
	 * 设置状态栏位置
	 *
	 * @param x 状态栏左上角的X坐标（像素）
	 * @param y 状态栏左上角的Y坐标（像素）
	 */
	void set_position(int x, int y)
	{
		position.x = x, position.y = y;
	}

	/**
	 * 更新状态栏数据
	 * 在游戏循环中调用，更新状态栏上显示的文本和数值
	 *
	 * @param renderer SDL渲染器，用于创建文本纹理
	 */
	void on_update(SDL_Renderer* renderer)
	{
		static TTF_Font* font = ResourcesManager::instance()->get_font_pool().find(ResID::Font_Main)->second;

		// 清除旧的文本纹理
		SDL_DestroyTexture(tex_text_background);
		tex_text_background = nullptr;
		SDL_DestroyTexture(tex_text_foreground);
		tex_text_foreground = nullptr;

		// 获取当前金币数量并转换为文本
		std::string str_val = std::to_string((int)CoinManager::instance()->get_current_coin_num());
		// 创建背景文本和前景文本的表面
		SDL_Surface* suf_text_background = TTF_RenderText_Blended(font, str_val.c_str(), color_text_background);
		SDL_Surface* suf_text_foreground = TTF_RenderText_Blended(font, str_val.c_str(), color_text_foreground);

		// 记录文本的宽度和高度，用于后续渲染
		width_text = suf_text_background->w, height_text = suf_text_background->h;

		// 从表面创建纹理
		tex_text_background = SDL_CreateTextureFromSurface(renderer, suf_text_background);
		tex_text_foreground = SDL_CreateTextureFromSurface(renderer, suf_text_foreground);

		// 释放表面资源
		SDL_FreeSurface(suf_text_background);
		SDL_FreeSurface(suf_text_foreground);
	}

	/**
	 * 渲染状态栏
	 * 在游戏渲染循环中调用，将状态栏绘制到屏幕上
	 *
	 * @param renderer SDL渲染器，用于绘制图形和文本
	 */
	void on_render(SDL_Renderer* renderer)
	{
		static SDL_Rect rect_dst;  // 目标矩形，用于确定渲染位置和大小
		// 获取纹理池和所需的图像资源
		static const ResourcesManager::TexturePool& tex_pool = ResourcesManager::instance()->get_texture_pool();
		static SDL_Texture* tex_coin = tex_pool.find(ResID::Tex_UICoin)->second;  // 金币图标
		static SDL_Texture* tex_heart = tex_pool.find(ResID::Tex_UIHeart)->second;  // 心形图标（血量）
		static SDL_Texture* tex_home_avatar = tex_pool.find(ResID::Tex_UIHomeAvatar)->second;  // 基地头像
		static SDL_Texture* tex_player_avatar = tex_pool.find(ResID::Tex_UIPlayerAvatar)->second;  // 玩家头像

		// 渲染基地头像
		rect_dst.x = position.x, rect_dst.y = position.y;
		rect_dst.w = 78, rect_dst.h = 78;
		SDL_RenderCopy(renderer, tex_home_avatar, nullptr, &rect_dst);

		// 渲染基地血量（心形图标）
		for (int i = 0; i < (int)HomeManager::instance()->get_current_hp_num(); i++)
		{
			rect_dst.x = position.x + 78 + 15 + i * (32 + 2);
			rect_dst.y = position.y;
			rect_dst.w = 32, rect_dst.h = 32;
			SDL_RenderCopy(renderer, tex_heart, nullptr, &rect_dst);
		}

		// 渲染金币图标
		rect_dst.x = position.x + 78 + 15;
		rect_dst.y = position.y + 78 - 32;
		rect_dst.w = 32, rect_dst.h = 32;
		SDL_RenderCopy(renderer, tex_coin, nullptr, &rect_dst);

		// 渲染金币数量的文本阴影（背景）
		rect_dst.x += 32 + 10 + offset_shadow.x;
		rect_dst.y = rect_dst.y + (32 - height_text) / 2 + offset_shadow.y;
		rect_dst.w = width_text, rect_dst.h = height_text;
		SDL_RenderCopy(renderer, tex_text_background, nullptr, &rect_dst);

		// 渲染金币数量的文本（前景）
		rect_dst.x -= offset_shadow.x;
		rect_dst.y -= offset_shadow.y;
		SDL_RenderCopy(renderer, tex_text_foreground, nullptr, &rect_dst);

		// 渲染玩家头像
		rect_dst.x = position.x + (78 - 65) / 2;
		rect_dst.y = position.y + 78 + 5;
		rect_dst.w = 65, rect_dst.h = 65;
		SDL_RenderCopy(renderer, tex_player_avatar, nullptr, &rect_dst);

		// 渲染魔法值条的背景
		rect_dst.x = position.x + 78 + 15;
		rect_dst.y += 10;
		roundedBoxRGBA(renderer, rect_dst.x, rect_dst.y, rect_dst.x + width_mp_bar, rect_dst.y + height_mp_bar, 4,
			color_mp_bar_background.r, color_mp_bar_background.g, color_mp_bar_background.b, color_mp_bar_background.a);

		// 渲染魔法值条的填充部分（根据当前魔法值百分比）
		rect_dst.x += width_border_mp_bar;
		rect_dst.y += width_border_mp_bar;
		rect_dst.w = width_mp_bar - 2 * width_border_mp_bar;
		rect_dst.h = height_mp_bar - 2 * width_border_mp_bar;
		double process = PlayerManager::instance()->get_current_mp() / 100;  // 计算魔法值百分比
		roundedBoxRGBA(renderer, rect_dst.x, rect_dst.y, rect_dst.x + (int)(rect_dst.w * process), rect_dst.y + rect_dst.h, 2,
			color_mp_bar_foredground.r, color_mp_bar_foredground.g, color_mp_bar_foredground.b, color_mp_bar_foredground.a);
	}

private:
	// 常量：定义UI元素的尺寸和颜色
	const int size_heart = 32;            // 心形图标大小
	const int width_mp_bar = 200;         // 魔法值条宽度
	const int height_mp_bar = 20;         // 魔法值条高度
	const int width_border_mp_bar = 4;    // 魔法值条边框宽度
	const SDL_Point offset_shadow = { 2, 2 }; // 文本阴影偏移量
	const SDL_Color color_text_background = { 175, 175, 175, 255 }; // 文本阴影颜色
	const SDL_Color color_text_foreground = { 255, 255, 255, 255 }; // 文本前景颜色
	const SDL_Color color_mp_bar_background = { 48, 40, 51, 255 };  // 魔法值条背景颜色
	const SDL_Color color_mp_bar_foredground = { 144, 121, 173, 255 }; // 魔法值条前景颜色

private:
	// 变量：保存状态栏的位置和文本相关信息
	SDL_Point position = { 0 };             // 状态栏位置
	int width_text = 0, height_text = 0;    // 文本宽高
	SDL_Texture* tex_text_background = nullptr; // 文本阴影纹理
	SDL_Texture* tex_text_foreground = nullptr; // 文本前景纹理
};

#endif // !_STATUS_BAR_H_