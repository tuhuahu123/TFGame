#pragma once

// 包含必要的头文件：地图块定义和路径生成功能
#include "tile.h"           // 包含地图瓦片(Tile)定义和TileMap类型
#include "route.h"          // 包含怪物路径生成工具

// 包含标准库和SDL库
#include <SDL.h>            // SDL库用于定义点(SDL_Point)结构
#include <string>           // 字符串处理
#include <fstream>          // 文件流操作，用于读取地图文件
#include <sstream>          // 字符串流处理，用于解析CSV格式
#include <unordered_map>    // 哈希表，用于存储怪物生成点和对应路径




// 地图类：负责加载、存储和提供地图数据
class Map
{
public:
    // 类型定义：SpawnerRoutePool是怪物生成点ID到其路径的映射表
    typedef std::unordered_map<int, Route> SpawnerRoutePool;

public:
    // 默认构造函数和析构函数
    Map() = default;        // 使用默认构造函数
    ~Map() = default;       // 使用默认析构函数

    // 加载地图文件函数
    bool load(const std::string& path)
    {
        // 步骤1：打开地图文件
        std::ifstream file(path);                // 创建文件输入流
        if (!file.good()) return false;          // 文件不存在或无法打开，返回失败

        // 步骤2：准备临时地图数据结构
        TileMap tile_map_temp;                   // 临时存储读取的地图数据
        int idx_x = -1, idx_y = -1;              // 地图坐标索引初始化

        // 步骤3：逐行读取地图文件
        std::string str_line;                    // 存储当前读取的行
        while (std::getline(file, str_line))     // 循环读取文件的每一行
        {
            str_line = trim_str(str_line);       // 清理行中的多余空白
            if (str_line.empty())                // 跳过空行
                continue;

            // 步骤4：为每行创建新的地图行
            idx_x = -1, idx_y++;                 // 重置X索引，递增Y索引
            tile_map_temp.emplace_back();        // 在地图中添加新行

            // 步骤5：解析行中的每个格子(按逗号分隔)
            std::string str_tile;                // 存储当前地图格子的字符串
            std::stringstream str_stream(str_line); // 创建字符串流便于按逗号分割
            while (std::getline(str_stream, str_tile, ',')) // 按逗号分割读取每个格子
            {
                idx_x++;                         // 递增X索引
                tile_map_temp[idx_y].emplace_back(); // 在当前行末尾添加新格子
                Tile& tile = tile_map_temp[idx_y].back(); // 获取新添加格子的引用
                load_tile_from_string(tile, str_tile); // 解析格子字符串，填充格子属性
            }
        }

        // 步骤6：关闭文件和验证地图
        file.close();                            // 关闭文件流

        // 检查地图是否为空
        if (tile_map_temp.empty() || tile_map_temp[0].empty())
            return false;                        // 地图为空，返回失败

        // 步骤7：保存解析后的地图
        tile_map = tile_map_temp;                // 将临时地图数据赋值给成员变量

        // 步骤8：生成地图缓存(处理特殊点位如家、怪物生成点等)
        generate_map_cache();                    // 解析特殊标记点并生成路径

        return true;                             // 地图加载成功
    }

    // 获取地图宽度(格子数)
    size_t get_width() const
    {
        if (tile_map.empty())                    // 检查地图是否为空
            return 0;                            // 地图为空，宽度为0

        return tile_map[0].size();               // 返回第一行的长度作为地图宽度
    }

    // 获取地图高度(格子数)
    size_t get_height() const
    {
        return tile_map.size();                  // 返回行数作为地图高度
    }

    // 获取完整地图数据的常量引用
    const TileMap& get_tile_map() const
    {
        return tile_map;                         // 返回地图数据的常量引用
    }

    // 获取家(防守目标)的位置索引
    const SDL_Point& get_idx_home() const
    {
        return idx_home;                         // 返回家位置的常量引用
    }

    // 获取所有怪物生成点及其路径的映射表
    const SpawnerRoutePool& get_idx_spawner_pool() const
    {
        return spwaner_route_pool;               // 返回生成点路径池的常量引用
    }

    // 在指定位置放置防御塔
    void place_tower(const SDL_Point& idx_tile)
    {
        tile_map[idx_tile.y][idx_tile.x].has_tower = true; // 将指定位置标记为有塔
    }

private:
    // 成员变量
    TileMap tile_map;                            // 存储地图的二维数组
    SDL_Point idx_home = { 0 };                  // 家(防守目标)的位置坐标
    SpawnerRoutePool spwaner_route_pool;         // 怪物生成点到路径的映射表

private:
    // 辅助函数：去除字符串首尾的空白字符
    std::string trim_str(const std::string& str)
    {
        // 查找第一个非空白字符的位置
        size_t begin_idx = str.find_first_not_of(" \t");
        if (begin_idx == std::string::npos)      // 如果字符串全是空白
            return "";                           // 返回空字符串

        // 查找最后一个非空白字符的位置
        size_t end_idx = str.find_last_not_of(" \t");
        // 计算非空白部分的长度
        size_t idx_range = end_idx - begin_idx + 1;

        // 返回去除首尾空白的子字符串
        return str.substr(begin_idx, idx_range);
    }

    // 从字符串解析地图块属性
    void load_tile_from_string(Tile& tile, const std::string& str)
    {
        // 步骤1：清理输入字符串
        std::string str_tidy = trim_str(str);    // 去除字符串首尾空白

        // 步骤2：准备变量用于解析
        std::string str_value;                   // 存储当前解析的值
        std::vector<int> values;                 // 存储所有解析出的数值
        std::stringstream str_stream(str_tidy);  // 创建字符串流用于分割

        // 步骤3：按反斜杠分割并解析每个值
        while (std::getline(str_stream, str_value, '\\')) // 使用反斜杠作为分隔符
        {
            int value;                           // 临时存储转换后的整数值
            try
            {
                value = std::stoi(str_value);    // 尝试将字符串转换为整数
            }
            catch (const std::invalid_argument&) // 捕获转换异常
            {
                value = -1;                      // 转换失败时使用默认值-1
            }
            values.push_back(value);             // 将值添加到数组中
        }

        // 步骤4：根据解析出的值设置地图块属性
        // 设置地形，如果值不存在或无效则使用默认值0
        tile.terrian = (values.size() < 1 || values[0] < 0) ? 0 : values[0];
        // 设置装饰，如果值不存在则使用默认值-1(表示无装饰)
        tile.decoration = (values.size() < 2) ? -1 : values[1];
        // 设置方向，如果值不存在或无效则使用默认值None(0)
        tile.direction = (Tile::Direction)((values.size() < 3 || values[2] < 0) ? 0 : values[2]);
        // 设置特殊标记，如果值不存在则使用默认值-1(表示无特殊标记)
        tile.special_flag = (values.size() <= 3) ? -1 : values[3];
    }

    // 生成地图缓存：处理特殊标记点及路径
    void generate_map_cache()
    {
        // 双重循环遍历整个地图的每个格子
        for (int y = 0; y < get_height(); y++)
        {
            for (int x = 0; x < get_width(); x++)
            {
                // 获取当前格子的引用
                const Tile& tile = tile_map[y][x];
                // 忽略没有特殊标记的普通地图块
                if (tile.special_flag < 0)
                    continue;

                // 如果是家(防守目标)
                if (tile.special_flag == 0)
                {
                    // 记录家的位置坐标
                    idx_home.x = x;
                    idx_home.y = y;
                }
                else // 如果是怪物生成点
                {
                    // 从该生成点创建到家的路径，并存入映射表
                    spwaner_route_pool[tile.special_flag] = Route(tile_map, { x, y });
                }
            }
        }
    }
};
