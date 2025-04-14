#pragma once                // 确保此头文件在编译过程中只被包含一次
#include "bullet.h"         // 包含基类Bullet的定义
#include "manager/resources_manager.h" // 包含资源管理器，用于获取纹理和音效

/**
 * ArrowBullet类 - 箭矢投射物
 *
 * 继承自Bullet基类，实现了特定类型的投射物(箭矢)
 * 特点：能够根据飞行方向旋转，给敌人造成伤害
 * 命中敌人时播放随机的箭矢命中音效
 */
class ArrowBullet : public Bullet
{
public:
	ArrowBullet()
	{
		static SDL_Texture* tex_arrow = ResourcesManager::instance()
			->get_texture_pool().find(ResID::Tex_BulletArrow)->second;

		static const std::vector<int> idx_list = { 0, 1 };

		animation.set_loop(true);
		animation.set_interval(0.1);
		animation.set_frame_data(tex_arrow, 2, 1, idx_list);

		can_rotated = true;
		size.x = 48, size.y = 48;
	}

	~ArrowBullet() = default;

	void on_collide(Enemy* enemy) override
	{
		static const ResourcesManager::SoundPool& sound_pool
			= ResourcesManager::instance()->get_sound_pool();

		switch (rand() % 3)
		{
		case 0:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowHit_1)->second, 0);
			break;
		case 1:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowHit_2)->second, 0);
			break;
		case 2:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowHit_3)->second, 0);
			break;
		}

		Bullet::on_collide(enemy);
	}

};