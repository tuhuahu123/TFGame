#ifndef _PANEL_H_
#define _PANEL_H_

#include "../../tile.h"
#include "../../manager/resources_manager.h"

#include <SDL.h>
#include <string>

/**
 * Panel类 - 游戏交互面板基类
 *
 * 功能：提供一个基础UI面板，允许玩家与游戏世界交互
 *
 * 主要组件：
 * - 三个交互按钮：顶部、左侧、右侧
 * - 选择光标：指示当前选中的格子
 * - 数值显示：悬停在按钮上时显示对应数值
 *
 * 主要方法：
 * - show()：显示面板
 * - set_idx_tile()：设置关联的格子索引
 * - set_center_pos()：设置面板位置
 * - on_input()：处理输入事件(鼠标移动、点击)
 * - on_update()：更新面板状态和文本
 * - on_render()：绘制面板到屏幕
 *
 * 子类需实现的虚函数：
 * - on_click_top_area()：顶部按钮点击处理
 * - on_click_left_area()：左侧按钮点击处理
 * - on_click_right_area()：右侧按钮点击处理
 */
class Panel
{
public:
	// 构造函数：初始化面板并加载选择光标资源
	Panel()
	{
		tex_select_cursor = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_UISelectCursor)->second;
	}

	// 析构函数：释放文本纹理资源
	~Panel()
	{
		SDL_DestroyTexture(tex_text_background);
		SDL_DestroyTexture(tex_text_foreground);
	}

	// 显示面板
	void show()
	{
		visible = true;
	}

	// 设置选中的格子索引
	void set_idx_tile(const SDL_Point& idx)
	{
		idx_tile_selected = idx;
	}

	// 设置面板的中心位置
	void set_center_pos(const SDL_Point& pos)
	{
		center_pos = pos;
	}

	/**
	 * 处理输入事件
	 * 检测鼠标悬停和点击，更新悬停状态并触发点击回调
	 */
	void on_input(const SDL_Event& event)
	{
		if (!visible) return;

		switch (event.type)  // 根据事件类型进行处理
		{
			// 处理鼠标移动事件
			case SDL_MOUSEMOTION:  
			{
			SDL_Point pos_cursor = { event.motion.x, event.motion.y };  // 获取鼠标当前位置
			SDL_Rect rect_target = { 0, 0, size_button, size_button };  // 初始化按钮的碰撞检测矩形

			// 检测鼠标是否悬停在顶部按钮上
			rect_target.x = center_pos.x - width / 2 + offset_top.x;  // 计算顶部按钮的X坐标
			rect_target.y = center_pos.y - width / 2 + offset_top.y;  // 计算顶部按钮的Y坐标
			if (SDL_PointInRect(&pos_cursor, &rect_target))  // 如果鼠标在顶部按钮内
			{
				hovered_target = HoveredTarget::Top;  // 设置当前悬停目标为顶部按钮
				return;  // 找到悬停区域后立即返回，不再检测其他区域
			}

			// 检测鼠标是否悬停在左侧按钮上
			rect_target.x = center_pos.x - width / 2 + offset_left.x;  // 计算左侧按钮的X坐标
			rect_target.y = center_pos.y - width / 2 + offset_left.y;  // 计算左侧按钮的Y坐标
			if (SDL_PointInRect(&pos_cursor, &rect_target))  // 如果鼠标在左侧按钮内
			{
				hovered_target = HoveredTarget::Left;  // 设置当前悬停目标为左侧按钮
				return;  // 找到悬停区域后立即返回，不再检测其他区域
			}

			// 检测鼠标是否悬停在右侧按钮上
			rect_target.x = center_pos.x - width / 2 + offset_right.x;  // 计算右侧按钮的X坐标
			rect_target.y = center_pos.y - width / 2 + offset_right.y;  // 计算右侧按钮的Y坐标
			if (SDL_PointInRect(&pos_cursor, &rect_target))  // 如果鼠标在右侧按钮内
			{
				hovered_target = HoveredTarget::Right;  // 设置当前悬停目标为右侧按钮
				return;  // 找到悬停区域后立即返回，不再检测其他区域
			}

			hovered_target = HoveredTarget::None;  // 如果鼠标不在任何按钮上，设置为无悬停
		}
		break;

		// 处理鼠标按键释放事件
		case SDL_MOUSEBUTTONUP:  
		{
			switch (hovered_target)  // 根据当前悬停的目标按钮触发相应回调
			{
			case Panel::HoveredTarget::Top:
				on_click_top_area();  // 调用顶部按钮点击回调函数
				break;
			case Panel::HoveredTarget::Left:
				on_click_left_area();  // 调用左侧按钮点击回调函数
				break;
			case Panel::HoveredTarget::Right:
				on_click_right_area();  // 调用右侧按钮点击回调函数
				break;
			}

			visible = false;  // 处理点击后隐藏面板，完成交互
		}
		break;
		default:  // 其他事件类型不处理
			break;
		}
	}

	/**
	 * 更新面板状态
	 * 根据当前悬停区域生成对应数值的文本纹理
	 */
	virtual void on_update(SDL_Renderer* renderer)
	{
		// 获取游戏主字体，使用静态变量避免重复查找
		static TTF_Font* font = ResourcesManager::instance()->get_font_pool().find(ResID::Font_Main)->second;

		// 如果当前没有悬停在任何区域上，则不需要更新文本显示，直接返回
		if (hovered_target == HoveredTarget::None)
			return;

		// 初始化显示值，将根据当前悬停区域选择对应的数值
		int val = 0;
		switch (hovered_target)
		{
		case Panel::HoveredTarget::Top:    // 如果悬停在顶部按钮上
			val = val_top;                 // 获取顶部按钮对应的数值
			break;
		case Panel::HoveredTarget::Left:   // 如果悬停在左侧按钮上
			val = val_left;                // 获取左侧按钮对应的数值
			break;
		case Panel::HoveredTarget::Right:  // 如果悬停在右侧按钮上
			val = val_right;               // 获取右侧按钮对应的数值
			break;
		}

		// 释放之前创建的文本纹理，避免内存泄漏
		SDL_DestroyTexture(tex_text_background);
		tex_text_background = nullptr;     // 设置为nullptr防止悬空指针
		SDL_DestroyTexture(tex_text_foreground);
		tex_text_foreground = nullptr;     // 设置为nullptr防止悬空指针

		// 将数值转换为字符串，如果值小于0则显示"MAX"
		std::string str_val = val < 0 ? "MAX" : std::to_string(val);
		// 创建背景文本表面（灰色阴影效果）
		SDL_Surface* suf_text_background = TTF_RenderText_Blended(font, str_val.c_str(), color_text_background);
		// 创建前景文本表面（白色主文本）
		SDL_Surface* suf_text_foreground = TTF_RenderText_Blended(font, str_val.c_str(), color_text_foreground);

		// 保存文本表面的宽高，用于后续渲染定位
		width_text = suf_text_background->w, height_text = suf_text_background->h;
		// 从表面创建背景文本纹理
		tex_text_background = SDL_CreateTextureFromSurface(renderer, suf_text_background);
		// 从表面创建前景文本纹理
		tex_text_foreground = SDL_CreateTextureFromSurface(renderer, suf_text_foreground);

		// 释放不再需要的表面资源，避免内存泄漏
		SDL_FreeSurface(suf_text_background);
		SDL_FreeSurface(suf_text_foreground);
	}

	/**
	 * 渲染面板
	 * 绘制选择光标、面板背景和数值文本
	 */
	virtual void on_render(SDL_Renderer* renderer)
	{
		// 如果面板不可见，直接返回不进行渲染
		if (!visible) return;

		// 创建并设置选择光标的目标矩形（显示在选中的格子上）
		SDL_Rect rect_dst_cursor =
		{
			center_pos.x - SIZE_TILE / 2,      // 光标左上角X坐标
			center_pos.y - SIZE_TILE / 2,      // 光标左上角Y坐标
			SIZE_TILE, SIZE_TILE               // 光标宽高
		};
		// 渲染选择光标（显示在玩家选中的格子上）
		SDL_RenderCopy(renderer, tex_select_cursor, nullptr, &rect_dst_cursor);

		// 创建并设置面板的目标矩形（以面板中心点为基准）
		SDL_Rect rect_dst_panel =
		{
			center_pos.x - width / 2,          // 面板左上角X坐标
			center_pos.y - height / 2,         // 面板左上角Y坐标
			width, height                      // 面板宽高
		};

		// 根据当前鼠标悬停区域选择对应的面板纹理
		SDL_Texture* tex_panel = nullptr;
		switch (hovered_target)
		{
		case Panel::HoveredTarget::None:
			tex_panel = tex_idle;              // 无悬停时的纹理
			break;
		case Panel::HoveredTarget::Top:
			tex_panel = tex_hovered_top;       // 顶部按钮悬停时的纹理
			break;
		case Panel::HoveredTarget::Left:
			tex_panel = tex_hovered_left;      // 左侧按钮悬停时的纹理
			break;
		case Panel::HoveredTarget::Right:
			tex_panel = tex_hovered_right;     // 右侧按钮悬停时的纹理
			break;
		}

		// 渲染面板背景
		SDL_RenderCopy(renderer, tex_panel, nullptr, &rect_dst_panel);

		// 如果没有悬停在任何按钮上，不显示数值文本
		if (hovered_target == HoveredTarget::None)
			return;

		// 设置数值文本的显示位置和大小
		SDL_Rect rect_dst_text;

		// 先绘制文本阴影（偏移一定距离）
		rect_dst_text.x = center_pos.x - width_text / 2 + offset_shadow.x;
		rect_dst_text.y = center_pos.y + height / 2 + offset_shadow.y;
		rect_dst_text.w = width_text, rect_dst_text.h = height_text;
		SDL_RenderCopy(renderer, tex_text_background, nullptr, &rect_dst_text);

		// 再绘制文本前景（正常位置）
		rect_dst_text.x -= offset_shadow.x;
		rect_dst_text.y -= offset_shadow.y;
		SDL_RenderCopy(renderer, tex_text_foreground, nullptr, &rect_dst_text); 
	}

protected:
	// 悬停区域枚举：无、顶部、左侧、右侧
	enum class HoveredTarget
	{
		None,
		Top,
		Left,
		Right
	};

protected:
	bool visible = false;               // 面板是否可见
	SDL_Point idx_tile_selected;        // 选中的格子索引
	SDL_Point center_pos = { 0 };       // 面板中心位置
	SDL_Texture* tex_idle = nullptr;    // 默认状态纹理
	SDL_Texture* tex_hovered_top = nullptr;    // 上方区域悬停状态纹理
	SDL_Texture* tex_hovered_left = nullptr;   // 左侧区域悬停状态纹理
	SDL_Texture* tex_hovered_right = nullptr;  // 右侧区域悬停状态纹理
	SDL_Texture* tex_select_cursor = nullptr;  // 选择光标纹理
	int val_top = 0, val_left = 0, val_right = 0;  // 三个区域对应的值
	HoveredTarget hovered_target = HoveredTarget::None;  // 当前悬停的区域

protected:
	// 点击上方区域的回调函数（子类必须实现）
	virtual void on_click_top_area() = 0;

	// 点击左侧区域的回调函数（子类必须实现）
	virtual void on_click_left_area() = 0;

	// 点击右侧区域的回调函数（子类必须实现）
	virtual void on_click_right_area() = 0;

private:
	const int size_button = 48;         // 按钮大小（像素）
	const int width = 144, height = 144; // 面板宽高（像素）
	const SDL_Point offset_top = { 48, 6 };    // 顶部按钮偏移量
	const SDL_Point offset_left = { 8, 80 };   // 左侧按钮偏移量
	const SDL_Point offset_right = { 90, 80 }; // 右侧按钮偏移量
	const SDL_Point offset_shadow = { 3, 3 };  // 文本阴影偏移量
	const SDL_Color color_text_background = { 175, 175, 175, 255 }; // 文本背景颜色
	const SDL_Color color_text_foreground = { 255, 255, 255, 255 }; // 文本前景颜色

	int width_text = 0, height_text = 0;       // 文本宽高
	SDL_Texture* tex_text_background = nullptr; // 文本背景纹理
	SDL_Texture* tex_text_foreground = nullptr; // 文本前景纹理
};

#endif // !_PANEL_H_