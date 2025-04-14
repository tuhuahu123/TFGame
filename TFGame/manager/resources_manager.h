#pragma once
#include "../manager/manager.h"
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <unordered_map>

enum class ResID
{
	Tex_Tileset, // 地图块纹理

	Tex_Player, // 玩家角色纹理
	Tex_Archer, // 箭手塔纹理
	Tex_Axeman, // 斧手塔纹理
	Tex_Gunner, // 枪手塔纹理

	Tex_Slime, // 史莱姆敌人纹理
	Tex_KingSlime, // 大史莱姆敌人纹理
	Tex_Skeleton, // 骷髅敌人纹理
	Tex_Goblin, // 哥布林敌人纹理
	Tex_GoblinPriest, // 哥布林牧师敌人纹理
	Tex_SlimeSketch, // 史莱姆草图纹理
	Tex_KingSlimeSketch, // 大史莱姆草图纹理
	Tex_SkeletonSketch, // 骷髅草图纹理
	Tex_GoblinSketch, // 哥布林草图纹理
	Tex_GoblinPriestSketch, // 哥布林牧师草图纹理

	Tex_BulletArrow, // 箭矢子弹纹理
	Tex_BulletAxe, // 斧头子弹纹理
	Tex_BulletShell, // 枪弹纹理

	Tex_Coin, // 金币纹理
	Tex_Home, // 家的纹理

	Tex_EffectFlash_Up, // 向上的闪光效果纹理
	Tex_EffectFlash_Down, // 向下的闪光效果纹理
	Tex_EffectFlash_Left, // 向左的闪光效果纹理
	Tex_EffectFlash_Right, // 向右的闪光效果纹理
	Tex_EffectImpact_Up, // 向上的冲击效果纹理
	Tex_EffectImpact_Down, // 向下的冲击效果纹理
	Tex_EffectImpact_Left, // 向左的冲击效果纹理
	Tex_EffectImpact_Right, // 向右的冲击效果纹理
	Tex_EffectExplode, // 爆炸效果纹理

	Tex_UISelectCursor, // UI选择光标纹理
	Tex_UIPlaceIdle, // UI放置塔时的空闲状态纹理
	Tex_UIPlaceHoveredTop, // UI放置塔时的顶部悬停状态纹理
	Tex_UIPlaceHoveredLeft, // UI放置塔时的左侧悬停状态纹理
	Tex_UIPlaceHoveredRight, // UI放置塔时的右侧悬停状态纹理
	Tex_UIUpgradeIdle, // UI升级塔时的空闲状态纹理
	Tex_UIUpgradeHoveredTop, // UI升级塔时的顶部悬停状态纹理
	Tex_UIUpgradeHoveredLeft, // UI升级塔时的左侧悬停状态纹理
	Tex_UIUpgradeHoveredRight, // UI升级塔时的右侧悬停状态纹理
	Tex_UIHomeAvatar, // UI家头像纹理
	Tex_UIPlayerAvatar, // UI玩家头像纹理
	Tex_UIHeart, // UI心形纹理
	Tex_UICoin, // UI金币纹理
	Tex_UIGameOverBar, // UI游戏结束条纹理
	Tex_UIWinText, // UI胜利文本纹理
	Tex_UILossText, // UI失败文本纹理

	Sound_ArrowFire_1, // 箭矢发射音效1
	Sound_ArrowFire_2, // 箭矢发射音效2
	Sound_AxeFire, // 斧头发射音效
	Sound_ShellFire, // 枪弹发射音效
	Sound_ArrowHit_1, // 箭矢命中音效1
	Sound_ArrowHit_2, // 箭矢命中音效2
	Sound_ArrowHit_3, // 箭矢命中音效3
	Sound_AxeHit_1, // 斧头命中音效1
	Sound_AxeHit_2, // 斧头命中音效2
	Sound_AxeHit_3, // 斧头命中音效3
	Sound_ShellHit, // 枪弹命中音效

	Sound_Flash, // 闪光音效
	Sound_Impact, // 冲击音效

	Sound_Coin, // 金币音效
	Sound_HomeHurt, // 家受伤音效
	Sound_PlaceTower, // 放置塔音效
	Sound_TowerLevelUp, // 塔升级音效

	Sound_Win, // 胜利音效
	Sound_Loss, // 失败音效

	Music_BGM, // 背景音乐

	Font_Main // 主字体
};

class ResourcesManager : public Manager<ResourcesManager>
{
	// 允许Manager<ResourcesManager>访问ResourcesManager的私有成员
	friend class Manager<ResourcesManager>;

public:
	// 定义用于存储字体资源的哈希表类型，键为资源ID，值为TTF字体指针
	typedef std::unordered_map<ResID, TTF_Font*> FontPool;
	// 定义用于存储音效资源的哈希表类型，键为资源ID，值为音效块指针
	typedef std::unordered_map<ResID, Mix_Chunk*> SoundPool;
	// 定义用于存储音乐资源的哈希表类型，键为资源ID，值为音乐指针
	typedef std::unordered_map<ResID, Mix_Music*> MusicPool;
	// 定义用于存储纹理资源的哈希表类型，键为资源ID，值为SDL纹理指针
	typedef std::unordered_map<ResID, SDL_Texture*> TexturePool;


	
public:
	// 从文件加载所有游戏资源的方法
	bool load_from_file(SDL_Renderer* renderer)
	{
		// 加载地图块纹理
		texture_pool[ResID::Tex_Tileset] = IMG_LoadTexture(renderer, "resources/tileset.png");

		// 加载玩家和防御塔纹理
		texture_pool[ResID::Tex_Player] = IMG_LoadTexture(renderer, "resources/player.png");
		texture_pool[ResID::Tex_Archer] = IMG_LoadTexture(renderer, "resources/tower_archer.png");
		texture_pool[ResID::Tex_Axeman] = IMG_LoadTexture(renderer, "resources/tower_axeman.png");
		texture_pool[ResID::Tex_Gunner] = IMG_LoadTexture(renderer, "resources/tower_gunner.png");

		// 加载敌人单位纹理
		texture_pool[ResID::Tex_Slime] = IMG_LoadTexture(renderer, "resources/enemy_slime.png");
		texture_pool[ResID::Tex_KingSlime] = IMG_LoadTexture(renderer, "resources/enemy_king_slime2.png");
		texture_pool[ResID::Tex_Skeleton] = IMG_LoadTexture(renderer, "resources/enemy_skeleton2.png");
		texture_pool[ResID::Tex_Goblin] = IMG_LoadTexture(renderer, "resources/enemy_goblin2.png");  
		texture_pool[ResID::Tex_GoblinPriest] = IMG_LoadTexture(renderer, "resources/enemy_goblin_priest2.png");
		// 敌人受击纹理
		texture_pool[ResID::Tex_SlimeSketch] = IMG_LoadTexture(renderer, "resources/New_resources/enemy_slime_White.png");
		texture_pool[ResID::Tex_KingSlimeSketch] = IMG_LoadTexture(renderer, "resources/New_resources/enemy_king_slime2_White.png");
		texture_pool[ResID::Tex_SkeletonSketch] = IMG_LoadTexture(renderer, "resources/New_resources/enemy_skeleton2_White.png");
		texture_pool[ResID::Tex_GoblinSketch] = IMG_LoadTexture(renderer, "resources/New_resources/enemy_goblin2_White.png");
		texture_pool[ResID::Tex_GoblinPriestSketch] = IMG_LoadTexture(renderer, "resources/New_resources/enemy_goblin_priest2_White.png");

		// 加载子弹/投射物纹理
		texture_pool[ResID::Tex_BulletArrow] = IMG_LoadTexture(renderer, "resources/bullet_arrow.png");
		texture_pool[ResID::Tex_BulletAxe] = IMG_LoadTexture(renderer, "resources/bullet_axe.png");
		texture_pool[ResID::Tex_BulletShell] = IMG_LoadTexture(renderer, "resources/bullet_shell.png");

		// 加载游戏道具纹理
		texture_pool[ResID::Tex_Coin] = IMG_LoadTexture(renderer, "resources/coin.png");
		texture_pool[ResID::Tex_Home] = IMG_LoadTexture(renderer, "resources/home.png");

		// 加载特效纹理 - 闪光效果（不同方向）
		texture_pool[ResID::Tex_EffectFlash_Up] = IMG_LoadTexture(renderer, "resources/effect_flash_up.png");
		texture_pool[ResID::Tex_EffectFlash_Down] = IMG_LoadTexture(renderer, "resources/effect_flash_down.png");
		texture_pool[ResID::Tex_EffectFlash_Left] = IMG_LoadTexture(renderer, "resources/effect_flash_left.png");
		texture_pool[ResID::Tex_EffectFlash_Right] = IMG_LoadTexture(renderer, "resources/effect_flash_right.png");
		// 加载特效纹理 - 冲击效果（不同方向）
		texture_pool[ResID::Tex_EffectImpact_Up] = IMG_LoadTexture(renderer, "resources/effect_impact_up.png");
		texture_pool[ResID::Tex_EffectImpact_Down] = IMG_LoadTexture(renderer, "resources/effect_impact_down.png");
		texture_pool[ResID::Tex_EffectImpact_Left] = IMG_LoadTexture(renderer, "resources/effect_impact_left.png");
		texture_pool[ResID::Tex_EffectImpact_Right] = IMG_LoadTexture(renderer, "resources/effect_impact_right.png");
		// 加载爆炸特效纹理
		texture_pool[ResID::Tex_EffectExplode] = IMG_LoadTexture(renderer, "resources/effect_explode.png");

		// 加载UI元素纹理 - 光标和塔放置相关
		texture_pool[ResID::Tex_UISelectCursor] = IMG_LoadTexture(renderer, "resources/ui_select_cursor.png");
		texture_pool[ResID::Tex_UIPlaceIdle] = IMG_LoadTexture(renderer, "resources/ui_place_idle.png");
		texture_pool[ResID::Tex_UIPlaceHoveredTop] = IMG_LoadTexture(renderer, "resources/ui_place_hovered_top.png");
		texture_pool[ResID::Tex_UIPlaceHoveredLeft] = IMG_LoadTexture(renderer, "resources/ui_place_hovered_left.png");
		texture_pool[ResID::Tex_UIPlaceHoveredRight] = IMG_LoadTexture(renderer, "resources/ui_place_hovered_right.png");
		// 加载UI元素纹理 - 塔升级相关
		texture_pool[ResID::Tex_UIUpgradeIdle] = IMG_LoadTexture(renderer, "resources/ui_upgrade_idle.png");
		texture_pool[ResID::Tex_UIUpgradeHoveredTop] = IMG_LoadTexture(renderer, "resources/ui_upgrade_hovered_top.png");
		texture_pool[ResID::Tex_UIUpgradeHoveredLeft] = IMG_LoadTexture(renderer, "resources/ui_upgrade_hovered_left.png");
		texture_pool[ResID::Tex_UIUpgradeHoveredRight] = IMG_LoadTexture(renderer, "resources/ui_upgrade_hovered_right.png");
		// 加载UI元素纹理 - 游戏状态和信息显示
		texture_pool[ResID::Tex_UIHomeAvatar] = IMG_LoadTexture(renderer, "resources/ui_home_avatar.png");
		texture_pool[ResID::Tex_UIPlayerAvatar] = IMG_LoadTexture(renderer, "resources/ui_player_avatar.png");
		texture_pool[ResID::Tex_UIHeart] = IMG_LoadTexture(renderer, "resources/ui_heart.png");
		texture_pool[ResID::Tex_UICoin] = IMG_LoadTexture(renderer, "resources/ui_coin.png");
		texture_pool[ResID::Tex_UIGameOverBar] = IMG_LoadTexture(renderer, "resources/ui_game_over_bar.png");
		texture_pool[ResID::Tex_UIWinText] = IMG_LoadTexture(renderer, "resources/ui_win_text.png");
		texture_pool[ResID::Tex_UILossText] = IMG_LoadTexture(renderer, "resources/ui_loss_text.png");

		// 检查所有纹理是否成功加载，若有任何一个加载失败则返回false
		for (const auto& pair : texture_pool)
			if (!pair.second) return false;

		// 加载箭矢发射和命中音效
		sound_pool[ResID::Sound_ArrowFire_1] = Mix_LoadWAV("resources/sound_arrow_fire_1.mp3");
		sound_pool[ResID::Sound_ArrowFire_2] = Mix_LoadWAV("resources/sound_arrow_fire_2.mp3");
		sound_pool[ResID::Sound_AxeFire] = Mix_LoadWAV("resources/sound_axe_fire.wav");
		sound_pool[ResID::Sound_ShellFire] = Mix_LoadWAV("resources/sound_shell_fire.wav");
		sound_pool[ResID::Sound_ArrowHit_1] = Mix_LoadWAV("resources/sound_arrow_hit_1.mp3");
		sound_pool[ResID::Sound_ArrowHit_2] = Mix_LoadWAV("resources/sound_arrow_hit_2.mp3");
		sound_pool[ResID::Sound_ArrowHit_3] = Mix_LoadWAV("resources/sound_arrow_hit_3.mp3");
		sound_pool[ResID::Sound_AxeHit_1] = Mix_LoadWAV("resources/sound_axe_hit_1.mp3");
		sound_pool[ResID::Sound_AxeHit_2] = Mix_LoadWAV("resources/sound_axe_hit_2.mp3");
		sound_pool[ResID::Sound_AxeHit_3] = Mix_LoadWAV("resources/sound_axe_hit_3.mp3");
		sound_pool[ResID::Sound_ShellHit] = Mix_LoadWAV("resources/sound_shell_hit.mp3");

		// 加载特效音效
		sound_pool[ResID::Sound_Flash] = Mix_LoadWAV("resources/sound_flash.wav");
		sound_pool[ResID::Sound_Impact] = Mix_LoadWAV("resources/sound_impact.wav");

		// 加载游戏事件音效
		sound_pool[ResID::Sound_Coin] = Mix_LoadWAV("resources/sound_coin.mp3");
		sound_pool[ResID::Sound_HomeHurt] = Mix_LoadWAV("resources/sound_home_hurt.wav");
		sound_pool[ResID::Sound_PlaceTower] = Mix_LoadWAV("resources/sound_place_tower.mp3");
		sound_pool[ResID::Sound_TowerLevelUp] = Mix_LoadWAV("resources/sound_tower_level_up.mp3");

		// 加载游戏结果音效
		sound_pool[ResID::Sound_Win] = Mix_LoadWAV("resources/sound_win.wav");
		sound_pool[ResID::Sound_Loss] = Mix_LoadWAV("resources/sound_loss.mp3");

		// 检查所有音效是否成功加载，若有任何一个加载失败则返回false
		for (const auto& pair : sound_pool)
			if (!pair.second) return false;

		// 加载背景音乐
		music_pool[ResID::Music_BGM] = Mix_LoadMUS("resources/music_bgm.mp3");

		// 检查所有音乐是否成功加载，若有任何一个加载失败则返回false
		for (const auto& pair : music_pool)
			if (!pair.second) return false;

		// 加载主字体，大小为25像素
		font_pool[ResID::Font_Main] = TTF_OpenFont("resources/ipix.ttf", 25);

		// 检查所有字体是否成功加载，若有任何一个加载失败则返回false
		for (const auto& pair : font_pool)
			if (!pair.second) return false;

		// 所有资源加载成功，返回true
		return true;
	}




	// 获取字体资源池的常量引用，用于外部访问但不可修改
	const FontPool& get_font_pool()
	{
		return font_pool;
	}

	// 获取音效资源池的常量引用，用于外部访问但不可修改
	const SoundPool& get_sound_pool()
	{
		return sound_pool;
	}

	// 获取音乐资源池的常量引用，用于外部访问但不可修改
	const MusicPool& get_music_pool()
	{
		return music_pool;
	}

	// 获取纹理资源池的常量引用，用于外部访问但不可修改
	const TexturePool& get_texture_pool()
	{
		return texture_pool;
	}

protected:
	// 构造函数使用默认实现，仅允许通过Manager<ResourcesManager>创建实例
	ResourcesManager() = default;
	// 析构函数使用默认实现，资源释放可能在其他地方处理
	~ResourcesManager() = default;

private:
	// 存储所有字体资源的哈希表
	FontPool font_pool;
	// 存储所有音效资源的哈希表
	SoundPool sound_pool;
	// 存储所有音乐资源的哈希表
	MusicPool music_pool;
	// 存储所有纹理资源的哈希表
	TexturePool texture_pool;
};


