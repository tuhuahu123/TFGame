#pragma once

#include "../UI/banner.h"
#include "manager.h"
#include "config_manager.h"
#include "enemy_manager.h"
#include "wave_manager.h"

#include "resources_manager.h"
#include "tower/tower_manager.h"
#include "bullet/bullet_manager.h"

#include "UI/status_bar.h"
#include "UI/panel/panel.h"
#include "UI/panel/place_panel.h"
#include "UI/panel/upgrade_panel.h"
#include "player_manager.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>




class GameManager : public Manager<GameManager>
{
	friend class Manager<GameManager>; // 友元类
public:
	
	int run(int argc, char** argv)
	{
		//TowerManager::instance()->place_tower(TowerType::Archer, { 5,0 }); // 初始化塔管理器
		

		Mix_FadeInMusic(ResourcesManager::instance()->get_music_pool().find(ResID::Music_BGM)->second, -1, 1500); // 播放背景音乐 -1表示循环播放 1500毫秒淡入

		Uint64 last_counter = SDL_GetPerformanceCounter();			 // 上一帧计时器
		const Uint64 counter_freq = SDL_GetPerformanceFrequency();  // 计时器频率


		while (!is_quit) // 游戏主循环
		{
			while (SDL_PollEvent(&event)) 
				on_input(); 

			Uint64 current_counter = SDL_GetPerformanceCounter();
			double delta = (double)(current_counter - last_counter) / counter_freq;
			last_counter = current_counter;
			if (delta * 1000 < 1000.0 / 60)
				SDL_Delay((Uint32)(1000.0 / 60 - delta * 1000));

			on_update(delta);

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			on_render();

			SDL_RenderPresent(renderer);
		}

		return 0;
	}

protected:
	GameManager() 
	{
		init_assert(SDL_Init(SDL_INIT_EVERYTHING) == 0, "SDL初始化失败"); // 初始化SDL
		init_assert(IMG_Init(IMG_INIT_PNG) == IMG_INIT_PNG, "SDL_image 初始化失败"); // 初始化SDL_image
		init_assert(Mix_Init(MIX_INIT_MP3), u8"SDL_mixer 初始化失败!");
		init_assert(!TTF_Init(), u8"SDL_ttf 初始化失败!");

		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048); // 初始化音频 44100Hz, 2声道, 2048字节缓冲区 
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");   // 设置输入法显示 

		ConfigManager* cofig = ConfigManager::instance(); // 获取配置管理器



		init_assert(cofig->map.load("config/map.csv"), u8"加载游戏地图失败！");
		init_assert(cofig->load_level_config("config/level.json"), u8"加载关卡配置失败！");
		init_assert(cofig->load_game_config("config/config.json"), u8"加载游戏配置失败！");


		window = SDL_CreateWindow(cofig->basic_template.window_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			cofig->basic_template.window_width, cofig->basic_template.window_height, SDL_WINDOW_SHOWN);
		init_assert(window, u8"创建游戏窗口失败！");

		//init_assert(window, "窗口创建失败"); // 窗口创建失败

		// 创建自定义按钮
		SDL_MessageBoxButtonData buttons[] = {
			{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, u8"确定" },  // 默认按钮
		};

		// 创建自定义消息框
		SDL_MessageBoxData messageboxdata = {
			SDL_MESSAGEBOX_INFORMATION,  // 类型
			window,                      // 父窗口
			u8"开发者信息",              // 标题
			u8"开发作者QQ：1030131545\n\n点击确定进入游戏", // 内容
			SDL_arraysize(buttons),      // 按钮数量
			buttons,                     // 按钮数组
			NULL                         // 颜色方案（使用默认）
		};

		int buttonid;  // 按钮ID
		SDL_ShowMessageBox(&messageboxdata, &buttonid);  // 显示消息框

		//创建渲染器启用硬件加速 且 启用垂直同步 且 启用渲染到纹理功能
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

		init_assert(renderer, "渲染器创建失败"); 
		init_assert(ResourcesManager::instance()->load_from_file(renderer), u8" 游戏资源加载失败");
		init_assert(generate_tile_map_texture(), u8"地图瓦片纹理生成失败");

		status_bar.set_position( 5, 5 ); // 设置状态栏位置

		banner = new Banner(); // 创建游戏结束横幅

		place_panel = new PlacePanel(); // 创建放置防御塔面板

		upgrade_panel = new UpgradePanel(); // 创建升级防御塔面板

	}

	~GameManager()
	{
		SDL_DestroyRenderer(renderer); // 销毁渲染器
		SDL_DestroyWindow(window);     // 销毁窗口
		TTF_Quit();                    // 退出TTF
		Mix_Quit();                    // 退出MIX
		IMG_Quit();                    // 退出IMG
		SDL_Quit();                    // 退出SDL

	}

private:
	SDL_Event event;         // 事件
	bool is_quit = false;    // 游戏退出标志

	StatusBar status_bar; // 状态栏

	SDL_Window* window = nullptr;      // 窗口
	SDL_Renderer* renderer = nullptr; //渲染器

	SDL_Texture* tex_tile_map = nullptr; // 瓦片地图纹理

	Panel* place_panel = nullptr;   // 放置防御塔面板
	Panel* upgrade_panel = nullptr; // 升级防御塔面板
	Banner* banner = nullptr; // 游戏结束横幅



	// 初始化断言
	void init_assert(bool flag, const char* err_msg)
	{
		if (flag) return;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, u8"游戏启动失败", err_msg,window);  // 初始化失败
	}

	
	/**
	 * 处理用户输入事件
	 *
	 * 该方法负责处理所有游戏输入事件，包括退出游戏、点击地图格子等
	 * 并根据点击位置显示相应的UI面板（升级面板或放置面板）
	 */
	void on_input()
	{
		// 静态变量保持在函数调用之间的值，避免重复创建
		static SDL_Point pos_center;               // 存储选中格子的中心点屏幕坐标
		static SDL_Point idx_tile_selected;        // 存储选中的格子索引
		static ConfigManager* instance = ConfigManager::instance(); // 获取配置管理器实例

		// 根据不同的事件类型进行处理
		switch (event.type)
		{
		case SDL_QUIT:                     // 如果是退出事件（如点击窗口关闭按钮）
			is_quit = true;                // 设置退出标志，将导致游戏主循环结束
			break;
		case SDL_MOUSEBUTTONDOWN:          // 如果是鼠标按键按下事件
			if (instance->is_game_over)    // 如果游戏已结束，不处理点击事件
				break;
			// 尝试获取鼠标点击位置对应的地图格子索引
			if (get_cursor_idx_tile(idx_tile_selected, event.motion.x, event.motion.y))
			{
				// 将格子索引转换为屏幕坐标（格子中心点）
				get_selected_tile_center_pos(pos_center, idx_tile_selected);

				// 如果点击了家（基地）所在位置
				if (check_home(idx_tile_selected))
				{
					// 设置升级面板的格子索引和位置
					upgrade_panel->set_idx_tile(idx_tile_selected);
					upgrade_panel->set_center_pos(pos_center);
					upgrade_panel->show();     // 显示升级面板
				}
				// 如果点击的是空地（可以放置防御塔的位置）
				else if (can_place_tower(idx_tile_selected))
				{
					// 设置放置面板的格子索引和位置
					place_panel->set_idx_tile(idx_tile_selected);
					place_panel->set_center_pos(pos_center);
					place_panel->show();       // 显示放置面板
				}
			}
			break;
		default:                           // 其他未处理的事件类型
			break;                         // 不做特殊处理
		}

		// 如果游戏未结束，将当前事件传递给其他交互组件
		if (!instance->is_game_over)
		{
			place_panel->on_input(event);       // 放置面板处理输入事件（如按钮悬停和点击）
			upgrade_panel->on_input(event);     // 升级面板处理输入事件（如按钮悬停和点击）
			PlayerManager::instance()->on_input(event);  // 玩家管理器处理输入事件（如技能释放）
		}
	}

	void on_update(double delta)
	{
		// 记录上一帧游戏是否已结束，用于检测游戏状态变化
		static bool is_game_over_last_tick = false;
		// 获取配置管理器实例，避免重复获取提高性能
		static ConfigManager* instance = ConfigManager::instance();

		// 检查游戏是否正在进行（未结束）
		if (!instance->is_game_over)
		{
			
			status_bar.on_update(renderer);          // 更新状态栏（生命值、金币等信息）
			place_panel->on_update(renderer);        // 更新防御塔放置面板
			upgrade_panel->on_update(renderer);      // 更新防御塔升级面板

			// 更新各个游戏系统管理器
			WaveManager::instance()->on_update(delta);    // 更新波次管理器，控制敌人的生成
			EnemyManager::instance()->on_update(delta);   // 更新敌人管理器，处理敌人移动和行为
			CoinManager::instance()->on_update(delta);    // 更新金币管理器，处理金币收集和经济系统
			BulletManager::instance()->on_update(delta);  // 更新子弹管理器，处理子弹移动和碰撞

			// 以下注释掉的代码是其他游戏系统的更新
			TowerManager::instance()->on_update(delta);   // 更新防御塔管理器，处理防御塔攻击逻辑
			PlayerManager::instance()->on_update(delta);  // 更新玩家管理器，处理玩家输入和行为

			return;
		}

		// 检测游戏是否刚刚结束（上一帧未结束，当前帧结束）
		if (!is_game_over_last_tick && instance->is_game_over)
		{
			// 获取音效资源池的静态引用，避免重复获取
			static const ResourcesManager::SoundPool& sound_pool
				= ResourcesManager::instance()->get_sound_pool();

			// 淡出背景音乐，持续1500毫秒
			Mix_FadeOutMusic(1500);
			// 播放胜利或失败音效（根据游戏结果选择）
			// -1表示使用第一个可用的声道，0表示不循环播放
			Mix_PlayChannel(-1, sound_pool.find(instance->is_game_win ? ResID::Sound_Win : ResID::Sound_Loss)->second, 0);
		}

		// 更新游戏结束状态记录，用于下一帧检测状态变化
		is_game_over_last_tick = instance->is_game_over;

		// 更新结算界面横幅
		banner->on_update(delta);
		// 如果横幅显示完成（默认5秒），则设置游戏退出标志
		if (banner->check_end_dispaly())
			is_quit = true;
	}

	void on_render()  //渲染
	{
		static ConfigManager* instance = ConfigManager::instance();  // 获取配置管理器实例
		static SDL_Rect& rect_dst = instance->rect_tile_map;
		SDL_RenderCopy(renderer, tex_tile_map, nullptr, &rect_dst); // 渲染地图纹理

		EnemyManager::instance()->on_render(renderer);  // 渲染敌人
		CoinManager::instance()->on_render(renderer);
		BulletManager::instance()->on_render(renderer);
		TowerManager::instance()->on_render(renderer);
		PlayerManager::instance()->on_render(renderer);

		if (!instance->is_game_over)
		{
			place_panel->on_render(renderer);
			upgrade_panel->on_render(renderer);
			status_bar.on_render(renderer);   // 渲染状态栏

			return;
		}

		int width_screen, height_screen;  // 获取屏幕宽高
		SDL_GetWindowSizeInPixels(window, &width_screen, &height_screen);  // 获取窗口像素大小
		banner->set_center_position({ (double)width_screen / 2, (double)height_screen / 2 });  // 设置横幅中心位置
		banner->on_render(renderer);  // 渲染横幅
	}


	// 生成地图瓦片纹理，将地图数据转换为可渲染的纹理
	bool generate_tile_map_texture()
	{
		// 获取地图数据，通过ConfigManager单例访问地图对象
		const Map& map = ConfigManager::instance()->map;

		// 获取瓦片地图数据，包含每个地图格子的地形和装饰信息
		const TileMap& tile_map = map.get_tile_map();

		// 获取地图矩形引用，用于确定地图在屏幕上的位置和大小
		SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		// 获取瓦片集纹理，包含所有地形和装饰的图像资源   获取资源管理器实例->获取纹理池->查找瓦片集纹理
		SDL_Texture* tex_tile_set = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Tileset)->second;


		// 瓦片集纹理的宽度和高度
		int width_tex_tile_set;
		int height_tex_tile_set;
		// 获取纹理的宽度和高度 纹理，格式，宽度，高度
		SDL_QueryTexture(tex_tile_set, nullptr, nullptr, &width_tex_tile_set, &height_tex_tile_set);

		// 计算瓦片集一行能放多少个瓦片，向上取整确保完整覆盖 瓦片集宽度/瓦片尺寸
		int num_tile_single_line = (int)std::ceil((double)width_tex_tile_set / SIZE_TILE);

		// 计算地图纹理的总宽度和高度
		int width_tex_tile_map, height_tex_tile_map;
		width_tex_tile_map = (int)map.get_width() * SIZE_TILE;  // 地图宽度 * 瓦片尺寸
		height_tex_tile_map = (int)map.get_height() * SIZE_TILE; // 地图高度 * 瓦片尺寸

		// 创建用于渲染地图的目标纹理，使用ARGB格式以支持透明度  
		tex_tile_map = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_TARGET, width_tex_tile_map, height_tex_tile_map);

		// 如果纹理创建失败，则返回false
		if (!tex_tile_map) return false;

		// 获取配置管理器以访问窗口尺寸信息
		ConfigManager* config = ConfigManager::instance();

		// 设置地图在屏幕上的位置，居中显示
		rect_tile_map.x = (config->basic_template.window_width - width_tex_tile_map) / 2;
		rect_tile_map.y = (config->basic_template.window_height - height_tex_tile_map) / 2;

		// 设置地图矩形的宽度和高度
		rect_tile_map.w = width_tex_tile_map;
		rect_tile_map.h = height_tex_tile_map;

		// 设置纹理混合模式为BLEND，使透明度生效
		SDL_SetTextureBlendMode(tex_tile_map, SDL_BLENDMODE_BLEND);

		// 设置渲染目标为地图纹理，之后的渲染操作将绘制到该纹理上
		SDL_SetRenderTarget(renderer, tex_tile_map);

		// 双重循环遍历地图上的每个瓦片
		for (int y = 0; y < map.get_height(); y++)
		{
			for (int x = 0; x < map.get_width(); x++)
			{
				// 声明用于存储源矩形的变量（从瓦片集中截取的区域）
				SDL_Rect rect_src;

				// 获取当前位置的瓦片对象
				const Tile& tile = tile_map[y][x];

				// 计算当前瓦片的目标位置矩形
				const SDL_Rect& rect_dst =
				{
					x * SIZE_TILE, y * SIZE_TILE,  // 左上角坐标
					SIZE_TILE, SIZE_TILE           // 宽度和高度
				};

				// 计算地形瓦片在瓦片集中的源矩形位置
				rect_src =
				{
					(tile.terrian % num_tile_single_line) * SIZE_TILE,    // 根据地形ID计算X坐标
					(tile.terrian / num_tile_single_line) * SIZE_TILE,    // 根据地形ID计算Y坐标
					SIZE_TILE, SIZE_TILE                                  // 宽度和高度
				};

				// 渲染地形瓦片到地图纹理
				SDL_RenderCopy(renderer, tex_tile_set, &rect_src, &rect_dst);

				// 如果有装饰物(decoration值大于等于0)，再渲染一层装饰物
				if (tile.decoration >= 0)
				{
					
					rect_src =
					{
						(tile.decoration % num_tile_single_line) * SIZE_TILE,    // 根据装饰ID计算X坐标
						(tile.decoration / num_tile_single_line) * SIZE_TILE,    // 根据装饰ID计算Y坐标
						SIZE_TILE, SIZE_TILE                                     // 宽度和高度
					};

					// 渲染装饰物瓦片到地图纹理，叠加在地形上
					SDL_RenderCopy(renderer, tex_tile_set, &rect_src, &rect_dst);
				}
			}
		}

		// 获取家的位置索引
		const SDL_Point& idx_home = map.get_idx_home();

		// 创建家的目标位置矩形
		const SDL_Rect rect_dst =
		{
			idx_home.x * SIZE_TILE, idx_home.y * SIZE_TILE,  // 左上角坐标，根据家的索引计算
			SIZE_TILE, SIZE_TILE                             // 宽度和高度
		};

		// 在家的位置渲染家的图像
		SDL_RenderCopy(renderer, ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Home)->second, nullptr, &rect_dst);

		// 将渲染目标重置为默认窗口，完成地图纹理的绘制
		SDL_SetRenderTarget(renderer, nullptr);

		// 返回true表示地图纹理生成成功
		return true;
	}


	// 获取鼠标点击的瓦片索引
	bool get_cursor_idx_tile(SDL_Point& idx_tile_selected, int screen_x, int screen_y) const
	{
		// 获取地图数据的静态引用，避免重复获取以提高性能
		static const Map& map = ConfigManager::instance()->map;

		// 获取地图在屏幕上的位置和尺寸信息
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		// 边界检查：判断鼠标点击位置是否在地图矩形范围内
		// 注意：这里存在一个bug，第四个条件应该是screen_y > rect_tile_map.y + rect_tile_map.h
		if (screen_x < rect_tile_map.x || screen_x > rect_tile_map.x + rect_tile_map.w
			|| screen_y < rect_tile_map.y || screen_y > rect_tile_map.x + rect_tile_map.h)
			return false;  // 点击在地图范围外，返回false

		// 计算点击位置对应的地图格子X坐标
		// 步骤：1. 计算点击位置相对于地图左上角的偏移量(screen_x - rect_tile_map.x)
		//       2. 除以格子大小得到格子索引
		//       3. 使用min确保索引不超过地图宽度范围
		idx_tile_selected.x = std::min((screen_x - rect_tile_map.x) / SIZE_TILE, (int)map.get_width() - 1);

		// 计算点击位置对应的地图格子Y坐标
		// 与X坐标计算方法相同
		idx_tile_selected.y = std::min((screen_y - rect_tile_map.y) / SIZE_TILE, (int)map.get_height() - 1);

		// 点击位置在地图范围内，计算成功，返回true
		return true;
	}

	/**
	 * 检查指定位置是否可以放置防御塔
	 *
	 * 该函数通过检查地图格子的属性来判断是否可以在此位置放置防御塔。
	 * 放置防御塔的三个条件：
	 * 1. 没有装饰物(decoration < 0)
	 * 2. 不是怪物路径(direction == None)
	 * 3. 当前格子没有其他防御塔(!has_tower)
	 */
	bool can_place_tower(const SDL_Point& idx_tile_selected) const
	{
		static const Map& map = ConfigManager::instance()->map;
		const Tile& tile = map.get_tile_map()[idx_tile_selected.y][idx_tile_selected.x];

		return (tile.decoration < 0 && tile.direction == Tile::Direction::None && !tile.has_tower);
	}

	/**
	 * 获取选中格子的中心点坐标（屏幕像素坐标）
	 *
	 * 该函数将格子索引转换为屏幕像素坐标，计算指定格子的中心点位置。
	 * 计算过程：
	 * 1. 地图原点(rect_tile_map.x/y) +
	 * 2. 格子偏移量(idx_tile_selected.x/y * SIZE_TILE) +
	 * 3. 格子中心点偏移(SIZE_TILE / 2)
	 */
	void get_selected_tile_center_pos(SDL_Point& pos, const SDL_Point& idx_tile_selected) const
	{
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		pos.x = rect_tile_map.x + idx_tile_selected.x * SIZE_TILE + SIZE_TILE / 2;
		pos.y = rect_tile_map.y + idx_tile_selected.y * SIZE_TILE + SIZE_TILE / 2;
	}


	//检查所选格子是否为家(基地)的位置
	bool check_home(const SDL_Point& idx_tile_selected)
	{
		// 获取地图对象的静态引用，避免重复获取提高性能
		static const Map& map = ConfigManager::instance()->map;

		// 获取家(基地)位置的静态引用，避免重复调用get_idx_home()
		static const SDL_Point& idx_home = map.get_idx_home();

		// 比较选中格子坐标与家(基地)坐标是否完全相同
		// 只有当x和y坐标都相等时才表示点击的是家(基地)位置
		return (idx_home.x == idx_tile_selected.x && idx_home.y == idx_tile_selected.y);
	}
};
