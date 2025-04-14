#include "bullet.h"                   // 引入基类Bullet的定义
#include "manager/resources_manager.h" // 引入资源管理器，用于获取纹理和音效

/**
 * ShellBullet类 - 炮弹/炮塔类型的子弹
 * 
 * 特点：
 * 1. 在命中敌人后产生爆炸动画效果
 * 2. 具有范围伤害能力
 * 3. 播放特定的命中音效
 * 
 * 继承自Bullet基类，实现了特定的碰撞和视觉效果
 */
class ShellBullet : public Bullet
{
public:
	ShellBullet()
	{
		// 使用static关键字，确保这些资源只加载一次，所有ShellBullet实例共享
		// 获取子弹飞行时的纹理
		static SDL_Texture* tex_shell = ResourcesManager::instance()
			->get_texture_pool().find(ResID::Tex_BulletShell)->second;
		// 获取爆炸效果的纹理
		static SDL_Texture* tex_explode = ResourcesManager::instance()
			->get_texture_pool().find(ResID::Tex_EffectExplode)->second;

		// 定义子弹动画使用的帧索引（0和1两帧）
		static const std::vector<int> idx_list = { 0, 1 };
		// 定义爆炸动画使用的帧索引（0到4共5帧）
		static const std::vector<int> idx_explode_list = { 0, 1, 2, 3, 4 };

		// 设置子弹飞行动画 - 继承自基类的animation属性
		animation.set_loop(true);              // 设置为循环播放
		animation.set_interval(0.1);           // 每帧显示0.1秒
		animation.set_frame_data(tex_shell, 2, 1, idx_list); // 设置帧数据：纹理、横向2帧、纵向1帧、索引列表

		// 设置爆炸动画 - 本类特有的animation_explode属性
		animation_explode.set_loop(false);     // 设置为不循环播放（播放一次后停止）
		animation_explode.set_interval(0.1);   // 每帧显示0.1秒
		animation_explode.set_frame_data(tex_explode, 5, 1, idx_explode_list); // 设置帧数据：纹理、横向5帧、纵向1帧、索引列表
		// 设置爆炸动画结束时的回调函数，使用Lambda表达式
		animation_explode.set_on_finished(
			[&]()  // [&]表示捕获外部所有变量的引用
			{
				make_invalid(); // 爆炸动画播放完毕后，使子弹无效（从游戏中移除）
			});

		// 设置伤害范围为96像素（半径）
		damage_range = 96;
		// 设置子弹的物理尺寸为48x48像素
		size.x = 48, size.y = 48;
	}

	// 析构函数使用默认实现，因为没有需要手动释放的资源
	~ShellBullet() = default;

	/**
	 * 更新子弹状态
	 * @param delta 上一帧到当前帧的时间差（秒）
	 * 
	 * 根据子弹是否仍可碰撞来决定更新逻辑：
	 * - 可碰撞：更新位置并检查边界（调用基类方法）
	 * - 不可碰撞：更新爆炸动画
	 */
	void on_update(double delta) override
	{
		// 检查子弹是否还可以与敌人碰撞
		if (can_collide())
		{
			// 如果可以碰撞，调用基类的更新方法（更新位置和普通动画）
			Bullet::on_update(delta);
			return;
		}

		// 如果不可碰撞（已命中敌人），则更新爆炸动画
		animation_explode.on_update(delta);
	}

	/**
	 * 渲染子弹
	 * @param renderer SDL渲染器，用于绘制图像
	 * 
	 * 根据子弹是否仍可碰撞来决定渲染逻辑：
	 * - 可碰撞：渲染子弹飞行图像（调用基类方法）
	 * - 不可碰撞：渲染爆炸动画
	 */
	void on_render(SDL_Renderer* renderer) override
	{
		// 检查子弹是否还可以与敌人碰撞
		if (can_collide())
		{
			// 如果可以碰撞，调用基类的渲染方法（渲染普通子弹动画）
			Bullet::on_render(renderer);
			return;
		}

		// 创建一个静态的SDL_Point用于指定渲染位置（静态变量可以减少重复创建对象的开销）
		static SDL_Point point;

		// 计算爆炸效果的渲染位置（使爆炸居中于子弹的位置）
		// 由于爆炸图像大小为96x96，需要从中心点减去一半宽高来定位左上角
		point.x = (int)(position.x - 96 / 2);
		point.y = (int)(position.y - 96 / 2);

		// 在计算好的位置渲染爆炸动画
		animation_explode.on_render(renderer, point);
	}

	/**
	 * 处理与敌人的碰撞
	 * @param enemy 碰撞到的敌人对象指针
	 * 
	 * 当子弹碰到敌人时：
	 * 1. 播放命中音效
	 * 2. 禁用碰撞功能，使子弹不再与其他敌人碰撞
	 * 3. 开始播放爆炸动画（在update和render中处理）
	 */
	void on_collide(Enemy* enemy) override
	{
		// 使用static关键字，确保只获取一次资源，所有实例共享
		static const ResourcesManager::SoundPool& sound_pool
			= ResourcesManager::instance()->get_sound_pool();

		// 播放炮弹命中音效（-1表示使用任意可用的音频通道，0表示不循环播放）
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ShellHit)->second, 0);

		// 禁用碰撞检测，使子弹不再与其他敌人产生碰撞
		// 注意：此时子弹仍然有效，只是转变为爆炸效果阶段
		disable_collide();
	}

private:
	// 爆炸效果的动画控制器
	// 注意：普通子弹动画使用基类的animation属性
	Animation animation_explode;
};


