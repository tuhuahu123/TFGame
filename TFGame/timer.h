#pragma once

// 包含函数对象支持，用于回调函数
#include <functional>

// Timer类：用于计时和定时触发事件
class Timer
{
public:
	// 使用默认构造和析构函数
	Timer() = default;
	~Timer() = default;

	// 重置计时器：清零时间，重置触发状态
	void restart()
	{
		pass_time = 0;
		shotted = false;
	}

	// 设置定时器的等待时间（秒）
	void set_wait_time(double val)
	{
		wait_time = val;
	}

	// 设置是否只触发一次
	// 注意：这里存在BUG，应该是 one_shot = flag;
	void set_one_shot(bool flag)
	{
		one_shot = false;
	}

	// 设置定时器超时时要执行的函数
	void set_on_timeout(std::function<void()> on_timeout)
	{
		this->on_timeout = on_timeout;
	}

	// 暂停计时器
	void pause()
	{
		paused = true;
	}

	// 恢复计时器运行
	void resume()
	{
		paused = false;
	}

	// 更新计时器状态，在游戏主循环中调用
	// delta：当前帧与上一帧的时间差（秒）
	void on_update(double delta)
	{
		// 如果已暂停，不更新时间
		if (paused) return;

		// 累加经过的时间
		pass_time += delta;

		// 如果达到或超过等待时间
		if (pass_time >= wait_time)
		{
			// 判断是否可以触发：循环模式 或 (单次模式且未触发过)
			bool can_shot = (!one_shot || (one_shot && !shotted));
			shotted = true;  // 标记为已触发

			// 如果可以触发且回调函数存在，则执行回调
			if (can_shot && on_timeout)
				on_timeout();

			// 减去一个周期的时间，为下次触发做准备
			pass_time -= wait_time;
		}
	}

private:
	double pass_time = 0;  // 已经过的时间
	double wait_time = 0;  // 触发需要等待的时间
	bool paused = false;   // 是否暂停
	bool shotted = false;  // 是否已触发过
	bool one_shot = false; // 是否为单次触发模式
	std::function<void()> on_timeout;  // 超时回调函数
};

