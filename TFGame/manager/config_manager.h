#pragma once  

#include"manager/manager.h"        // 包含单例模式管理器，用于全局资源管理
#include"../tile.h"           // 包含地图块(Tile)定义及TileMap类型
#include <SDL_video.h>     // 包含SDL视频相关功能，用于图形渲染
#include <string>          // 包含标准字符串类，用于路径和文本处理
#include <fstream>         // 包含文件流操作，用于读取地图文件
#include<sstream>          // 包含字符串流，用于解析地图数据
#include <SDL.h>           // 包含SDL库，用于基础功能
#include<unordered_map>
#include "../route.h"
#include "../map.h"
#include"enemy/enemy_type.h"
#include "../wave.h"
#include <cJSON.h>

/**
 * ConfigManager 类用于管理游戏的配置数据，包括窗口设置、玩家属性、塔属性和敌人属性等。
 * 它从JSON文件中加载这些配置，并将其解析为相应的模板结构体。
 */


class ConfigManager : public Manager<ConfigManager>
{
    friend class Manager<ConfigManager>; // 友元类

public:
    struct BasicTemplate	// 基础模板
    {
        std::string window_title = u8"代号：风语"; // 窗口标题
        int window_width = 1280; // 窗口宽度
        int window_height = 720; // 窗口高度

    };

    struct PlayerTemplate    // 玩家模板
    {
        double speed = 3;   // 移动速度
        double normal_attack_interval = 0.5;  // 普通攻击间隔
        double normal_attack_damage = 0;     // 普通攻击伤害
        double skill_interval = 10;         // 技能间隔
        double skill_damage = 1;           // 技能伤害
    };

    // 塔模板
    struct TowerTemplate
    {
        double interval[10] = { 1 };     // 攻击间隔
        double damage[10] = { 25 };      // 攻击伤害
        double view_range[10] = { 5 };   // 视野范围
        double cost[10] = { 50 };        // 造价
        double upgrade_cost[9] = { 75 }; // 升级造价
    };

    // 敌人模板
    struct EnemyTemplate
    {
        double speed = 1;					// 移动速度
        double health = 100;				// 生命值
        double damage = 10;					// 攻击伤害
        double reward_ratio = 0.5;			// 奖励
        double recover_interval = 10;		// 恢复间隔
        double recover_range = 0;           // 恢复范围
        double recover_intensity = 25;      // 恢复强度
    };

public:
    Map map; // 地图
    std::vector<Wave> wave_list; // 波

    int level_archere = 0;  // 等级
    int level_axeman = 0;
    int level_gunner = 0;

    bool is_game_win = true;   
    bool is_game_over = false;
    SDL_Rect rect_tile_map = { 0 };

    BasicTemplate basic_template;  // 基础模板

    PlayerTemplate player_template;  // 玩家模板


    TowerTemplate archer_template; // 箭塔模板
    TowerTemplate axeman_template; // 斧手模板
    TowerTemplate gunner_template; // 枪手模板

    EnemyTemplate slim_template;		      // 史莱姆模板
    EnemyTemplate king_slim_template;       // 大史莱姆模板
    EnemyTemplate skeleton_template;       // 骷髅模板
    EnemyTemplate goblin_template;         // 哥布林模板
    EnemyTemplate goblin_priest_template; // 哥布林牧师模板

    const double num_initial_hp = 10;       //初始生命值
    const double num_initial_coin = 100;   //初始金币
    const double num_coin_per_prop = 10;  //每个道具的金币

public:

    /**
     * 加载关卡配置 - 从JSON文件中读取波次和敌人生成信息
     *
     * 该函数读取关卡配置JSON文件，解析波次数据和敌人生成事件，
     * 建立游戏中敌人出现的时间、位置和类型安排。
     *
     * @param path 配置文件路径（JSON格式文件）
     * @return 成功返回true，失败返回false
     */
    bool load_level_config(const std::string& path)
    {
        // 第一步：打开并读取JSON文件
        std::ifstream file(path);  // 创建文件输入流对象

        if (!file.good()) return false;  // 文件不存在或无法打开，返回失败

        // 将文件内容读入字符串流
        std::stringstream str_stream;
        str_stream << file.rdbuf(); file.close();  // 读取全部内容并关闭文件

        // 第二步：解析JSON数据
        cJSON* json_root = cJSON_Parse(str_stream.str().c_str());  // 解析JSON字符串
        if (!json_root) return false;  // JSON解析失败，返回失败

        // 检查根节点类型是否为数组（每个元素代表一个波次）
        if (json_root->type != cJSON_Array)
        {
            cJSON_Delete(json_root);  // 释放JSON资源
            return false;  // 格式错误，返回失败
        }

        // 第三步：遍历波次数组
        cJSON* json_wave = nullptr;  // 当前处理的波次JSON对象
        cJSON_ArrayForEach(json_wave, json_root)  // 遍历根数组中的每个元素
        {
            // 跳过非对象类型的元素
            if (json_wave->type != cJSON_Object)
                continue;

            // 创建新的波次对象
            wave_list.emplace_back();  // 在波次列表末尾添加新元素
            Wave& wave = wave_list.back();  // 获取刚添加的波次引用

            // 读取波次的奖励值
            cJSON* json_wave_rewards = cJSON_GetObjectItem(json_wave, "rewards");
            if (json_wave_rewards && json_wave_rewards->type == cJSON_Number)
            {
                wave.rawards = json_wave_rewards->valuedouble;  // 设置波次完成后的奖励值
            }

            // 读取波次的时间间隔
            cJSON* json_wave_interval = cJSON_GetObjectItem(json_wave, "interval");
            if (json_wave_interval && json_wave_interval->type == cJSON_Number)
            {
                wave.interval = json_wave_interval->valuedouble;  // 设置波次间的等待时间
            }

            // 第四步：处理敌人生成事件列表
            cJSON* json_wave_spawn_list = cJSON_GetObjectItem(json_wave, "spawn_list");
            if (json_wave_spawn_list && json_wave_spawn_list->type == cJSON_Array)
            {
                // 遍历生成事件数组
                cJSON* json_spawn_event = nullptr;
                cJSON_ArrayForEach(json_spawn_event, json_wave_spawn_list)
                {
                    // 跳过非对象类型的元素
                    if (json_spawn_event->type != cJSON_Object)
                        continue;

                    // 创建新的敌人生成事件
                    wave.spawn_event_list.emplace_back();  // 添加新的生成事件
                    Wave::SpawnEvent& spawn_event = wave.spawn_event_list.back();  // 获取引用

                    // 读取生成间隔时间
                    cJSON* json_spawn_event_interval = cJSON_GetObjectItem(json_spawn_event, "interval");
                    if (json_spawn_event_interval && json_spawn_event_interval->type == cJSON_Number)
                    {
                        spawn_event.interval = json_spawn_event_interval->valuedouble;  // 设置敌人生成的间隔时间
                    }

                    // 读取生成点编号
                    cJSON* json_spawn_event_spawn_point = cJSON_GetObjectItem(json_spawn_event, "point");
                    if (json_spawn_event_spawn_point && json_spawn_event_spawn_point->type == cJSON_Number)
                    {
                        spawn_event.spawn_point = json_spawn_event_spawn_point->valueint;  // 设置敌人出生点ID
                    }

                    // 读取敌人类型
                    cJSON* json_spawn_event_enemy_type = cJSON_GetObjectItem(json_spawn_event, "enemy");
                    if (json_spawn_event_enemy_type && json_spawn_event_enemy_type->type == cJSON_String)
                    {
                        // 根据字符串确定敌人类型枚举值
                        const std::string str_enemy_type = json_spawn_event_enemy_type->valuestring;

                        // 将字符串映射到枚举类型
                        if (str_enemy_type == "Slim")  // 史莱姆（基础敌人）
                            spawn_event.enemy_type = EnemyType::Slim;
                        else if (str_enemy_type == "KingSlim")  // 史莱姆王（强化敌人）
                            spawn_event.enemy_type = EnemyType::KingSlim;
                        else if (str_enemy_type == "Skeleton")  // 骷髅（远程敌人）
                            spawn_event.enemy_type = EnemyType::Skeleton;
                        else if (str_enemy_type == "Goblin")  // 哥布林（快速敌人）
                            spawn_event.enemy_type = EnemyType::Goblin;
                        else if (str_enemy_type == "GoblinPriest")  // 哥布林牧师（治疗型敌人）
                            spawn_event.enemy_type = EnemyType::GoblinPriest;
                    }
                }

                // 检查生成事件列表是否为空，为空则移除当前波次
                if (wave.spawn_event_list.empty())
                {
                    wave_list.pop_back();  // 移除没有敌人生成事件的波次
                }
            }
        }

        // 第五步：清理和验证
        cJSON_Delete(json_root);  // 释放JSON解析资源

        // 确保至少有一个有效波次
        if (wave_list.empty())
        {
            return false;  // 没有有效波次配置，返回失败
        }

        return true;  // 成功加载关卡配置
    }


    bool load_game_config(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.good()) return false;

        std::stringstream str_stream;
        str_stream << file.rdbuf(); file.close();

        cJSON* json_root = cJSON_Parse(str_stream.str().c_str());
        if (!json_root || json_root->type != cJSON_Object) return false;

        cJSON* json_basic = cJSON_GetObjectItem(json_root, "basic");
        cJSON* json_player = cJSON_GetObjectItem(json_root, "player");
        cJSON* json_tower = cJSON_GetObjectItem(json_root, "tower");
        cJSON* json_enemy = cJSON_GetObjectItem(json_root, "enemy");

        if (!json_basic || !json_player || !json_tower || !json_enemy
            || json_basic->type != cJSON_Object
            || json_player->type != cJSON_Object
            || json_tower->type != cJSON_Object
            || json_enemy->type != cJSON_Object)
        {
            cJSON_Delete(json_root);
            return false;
        }

        parse_basic_template(basic_template, json_basic);

        parse_player_template(player_template, json_player);

        parse_tower_template(archer_template, cJSON_GetObjectItem(json_tower, "archer"));
        parse_tower_template(axeman_template, cJSON_GetObjectItem(json_tower, "axeman"));
        parse_tower_template(gunner_template, cJSON_GetObjectItem(json_tower, "gunner"));

        parse_enemy_template(slim_template, cJSON_GetObjectItem(json_enemy, "slim"));
        parse_enemy_template(king_slim_template, cJSON_GetObjectItem(json_enemy, "king_slim"));
        //parse_enemy_template(goblin_template, cJSON_GetObjectItem(json_enemy, "skeleton"));
        parse_enemy_template(skeleton_template, cJSON_GetObjectItem(json_enemy, "skeleton"));
        parse_enemy_template(goblin_template, cJSON_GetObjectItem(json_enemy, "goblin"));
        parse_enemy_template(goblin_priest_template, cJSON_GetObjectItem(json_enemy, "goblin_priest"));

        cJSON_Delete(json_root);
        return true;
    }

protected:
    ConfigManager() = default;
    ~ConfigManager() = default;

private:

    /**
    * 解析基本模板配置 - 从JSON配置中提取基本游戏窗口设置
    *
    * 该函数负责从JSON配置对象中读取基本游戏设置，包括窗口标题、宽度和高度，
    * 并将这些值更新到BasicTemplate结构体中。
    *
    * @param tpl 基本模板结构体引用，用于存储提取的配置值
    * @param json_root JSON对象指针，包含基本配置信息
    */
    void parse_basic_template(BasicTemplate& tpl, cJSON* json_root)
    {
        // 检查JSON根节点是否有效且为对象类型
        if (!json_root || json_root->type != cJSON_Object) return;

        // 从JSON对象中获取窗口标题、宽度和高度字段
        cJSON* json_window_title = cJSON_GetObjectItem(json_root, "window_title");
        cJSON* json_window_width = cJSON_GetObjectItem(json_root, "window_width");
        cJSON* json_window_height = cJSON_GetObjectItem(json_root, "window_height");

        // 如果窗口标题存在且为字符串类型，则更新模板中的窗口标题
        if (json_window_title && json_window_title->type == cJSON_String)
            tpl.window_title = json_window_title->valuestring;
        // 如果窗口宽度存在且为数字类型，则更新模板中的窗口宽度
        if (json_window_width && json_window_width->type == cJSON_Number)
            tpl.window_width = json_window_width->valueint;
        // 如果窗口高度存在且为数字类型，则更新模板中的窗口高度
        if (json_window_height && json_window_height->type == cJSON_Number)
            tpl.window_height = json_window_height->valueint;
    }


    // JSON解析玩家模板
    void parse_player_template(PlayerTemplate& tpl, cJSON* json_root)
    {
        // 检查JSON根节点是否有效且为对象类型
        if (!json_root || json_root->type != cJSON_Object) return;

        // 从JSON对象中获取玩家各项属性字段
        cJSON* json_speed = cJSON_GetObjectItem(json_root, "speed");  // 移动速度
        cJSON* json_normal_attack_interval = cJSON_GetObjectItem(json_root, "normal_attack_interval");  // 普通攻击间隔
        cJSON* json_normal_attack_damage = cJSON_GetObjectItem(json_root, "normal_attack_damage");  // 普通攻击伤害
        cJSON* json_skill_interval = cJSON_GetObjectItem(json_root, "skill_interval");  // 技能冷却时间
        cJSON* json_skill_damage = cJSON_GetObjectItem(json_root, "skill_damage");  // 技能伤害值

        // 逐一检查并更新玩家属性值
        // 如果速度值存在且为数字类型，则更新模板中的速度值
        if (json_speed && json_speed->type == cJSON_Number)
            tpl.speed = json_speed->valuedouble;
        // 如果普通攻击间隔存在且为数字类型，则更新模板中的普通攻击间隔
        if (json_normal_attack_interval && json_normal_attack_interval->type == cJSON_Number)
            tpl.normal_attack_interval = json_normal_attack_interval->valuedouble;
        // 如果普通攻击伤害存在且为数字类型，则更新模板中的普通攻击伤害
        if (json_normal_attack_damage && json_normal_attack_damage->type == cJSON_Number)
            tpl.normal_attack_damage = json_normal_attack_damage->valuedouble;
        // 如果技能间隔存在且为数字类型，则更新模板中的技能间隔
        if (json_skill_interval && json_skill_interval->type == cJSON_Number)
            tpl.skill_interval = json_skill_interval->valuedouble;
        // 如果技能伤害存在且为数字类型，则更新模板中的技能伤害
        if (json_skill_damage && json_skill_damage->type == cJSON_Number)
            tpl.skill_damage = json_skill_damage->valuedouble;
    }


    // 解析数字数组 参数：ary 数组指针，max_len 数组最大长度，json_root JSON根节点
    void parse_number_array(double* ary, int max_len, cJSON* json_root)
    {
        // 检查JSON节点是否有效且为数组类型
        if (!json_root || json_root->type != cJSON_Array)
            return;

        // 初始化索引为-1，用于跟踪当前处理的数组元素
        int idx = -1;
        // 定义用于遍历JSON数组的元素指针
        cJSON* json_element = nullptr;
        // 使用cJSON提供的宏遍历数组中的每个元素
        cJSON_ArrayForEach(json_element, json_root)
        {
            // 增加索引，指向下一个数组位置
            idx++;
            // 如果元素不是数字类型或索引超出数组最大长度，则跳过
            if (json_element->type != cJSON_Number || idx >= max_len)
                continue;

            // 将JSON数字值存储到数组的对应位置
            ary[idx] = json_element->valuedouble;
        }
    }


    // JSON解析 塔模板
    void parse_tower_template(TowerTemplate& tpl, cJSON* json_root)
    {
        // 检查JSON根节点是否有效且为对象类型
        if (!json_root || json_root->type != cJSON_Object) return;

        // 从JSON对象中获取塔的各项属性数组
        cJSON* json_interval = cJSON_GetObjectItem(json_root, "interval");         // 攻击间隔数组
        cJSON* json_damage = cJSON_GetObjectItem(json_root, "damage");             // 伤害值数组
        cJSON* json_view_range = cJSON_GetObjectItem(json_root, "view_range");     // 视野范围数组
        cJSON* json_cost = cJSON_GetObjectItem(json_root, "cost");                 // 建造成本数组
        cJSON* json_upgrade_cost = cJSON_GetObjectItem(json_root, "upgrade_cost"); // 升级成本数组

        // 解析各个数组并填充到塔模板的相应字段中
        parse_number_array(tpl.interval, 10, json_interval);       // 最多10个等级的攻击间隔
        parse_number_array(tpl.damage, 10, json_damage);           // 最多10个等级的攻击伤害
        parse_number_array(tpl.view_range, 10, json_view_range);   // 最多10个等级的视野范围
        parse_number_array(tpl.cost, 10, json_cost);               // 最多10个等级的建造成本
        parse_number_array(tpl.upgrade_cost, 9, json_upgrade_cost); // 最多9个升级成本(1-9级)
    }

    // JSON解析 敌人模板
    void parse_enemy_template(EnemyTemplate& tpl, cJSON* json_root)
    {
        // 检查JSON根节点是否有效且为对象类型
        if (!json_root || json_root->type != cJSON_Object) return;

        // 从JSON对象中获取敌人的各项属性字段
        cJSON* json_hp = cJSON_GetObjectItem(json_root, "hp");                       // 生命值
        cJSON* json_speed = cJSON_GetObjectItem(json_root, "speed");                 // 移动速度
        cJSON* json_damage = cJSON_GetObjectItem(json_root, "damage");               // 攻击伤害
        cJSON* json_reward_ratio = cJSON_GetObjectItem(json_root, "reward_ratio");   // 击杀奖励比率
        cJSON* json_recover_interval = cJSON_GetObjectItem(json_root, "recover_interval"); // 恢复间隔
        cJSON* json_recover_range = cJSON_GetObjectItem(json_root, "recover_range");       // 恢复范围
        cJSON* json_recover_intensity = cJSON_GetObjectItem(json_root, "recover_intensity"); // 恢复强度

        // 逐一检查并更新敌人属性值
        // 如果生命值存在且为数字类型，则更新模板中的生命值
        if (json_hp && json_hp->type == cJSON_Number)
            tpl.health = json_hp->valuedouble;
        // 如果速度值存在且为数字类型，则更新模板中的速度值
        if (json_speed && json_speed->type == cJSON_Number)
            tpl.speed = json_speed->valuedouble;
        // 如果伤害值存在且为数字类型，则更新模板中的伤害值
        if (json_damage && json_damage->type == cJSON_Number)
            tpl.damage = json_damage->valuedouble;
        // 如果奖励比率存在且为数字类型，则更新模板中的奖励比率
        if (json_reward_ratio && json_reward_ratio->type == cJSON_Number)
            tpl.reward_ratio = json_reward_ratio->valuedouble;
        // 如果恢复间隔存在且为数字类型，则更新模板中的恢复间隔
        if (json_recover_interval && json_recover_interval->type == cJSON_Number)
            tpl.recover_interval = json_recover_interval->valuedouble;
        // 如果恢复范围存在且为数字类型，则更新模板中的恢复范围
        if (json_recover_range && json_recover_range->type == cJSON_Number)
            tpl.recover_range = json_recover_range->valuedouble;
        // 如果恢复强度存在且为数字类型，则更新模板中的恢复强度
        if (json_recover_intensity && json_recover_intensity->type == cJSON_Number)
            tpl.recover_intensity = json_recover_intensity->valuedouble;
    }


};



