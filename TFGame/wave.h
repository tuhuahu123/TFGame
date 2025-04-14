#pragma once	

#include"manager/manager.h"        // 包含单例模式管理器，用于全局资源管理
#include"enemy/enemy_type.h"           // 包含敌人类型定义
#include"route.h"                // 包含路径定义

struct Wave    // 波次
{
	struct SpawnEvent  // 出生事件
	{
		int spawn_point = 1;  // 出生点
		double interval = 0;   // 间隔
		EnemyType enemy_type = EnemyType::Slim; // 敌人类型
	};

	double rawards = 0;  // 奖励
	double interval = 0; // 间隔
	std::vector<SpawnEvent> spawn_event_list; // 出生事件
};

