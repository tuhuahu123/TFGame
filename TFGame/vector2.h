#pragma once
#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#include <cmath>

// Vector2类：表示2D空间中的向量，支持常见向量运算
// 用于游戏中的位置、方向、速度和力的表示
class Vector2
{
public:
    double x = 0;  // 向量的x分量
    double y = 0;  // 向量的y分量

public:
    // 默认构造函数：创建零向量(0,0)
    Vector2() = default;
    ~Vector2() = default;

    // 构造具有指定坐标的向量
    // x: 向量的x分量
    // y: 向量的y分量
    Vector2(double x, double y)
        : x(x), y(y) {
    }

    // 向量加法：返回两个向量的和
    // vec: 要加的向量
    // 返回：新的和向量
    Vector2 operator+(const Vector2& vec) const
    {
        return Vector2(x + vec.x, y + vec.y);
    }

    // 向量加等：将另一个向量加到当前向量
    // vec: 要加的向量
    void operator+=(const Vector2& vec)
    {
        x += vec.x, y += vec.y;
    }

    // 向量减法：返回两个向量的差
    // vec: 要减的向量
    // 返回：新的差向量
    Vector2 operator-(const Vector2& vec) const
    {
        return Vector2(x - vec.x, y - vec.y);
    }

    // 向量减等：从当前向量中减去另一个向量
    // vec: 要减的向量
    void operator-=(const Vector2& vec)
    {
        x -= vec.x, y -= vec.y;
    }

    // 向量点积：返回两个向量的点积(标量)
    // vec: 要做点积的向量
    // 返回：点积结果(x1*x2 + y1*y2)
    double operator*(const Vector2& vec) const
    {
        return x * vec.x + y * vec.y;
    }

    // 向量缩放：返回向量与标量的乘积
    // val: 缩放因子
    // 返回：新的缩放后的向量
    Vector2 operator*(double val) const
    {
        return Vector2(x * val, y * val);
    }

    // 向量缩放等：将当前向量缩放指定倍数
    // val: 缩放因子
    void operator*=(double val)
    {
        x *= val, y *= val;
    }

    // 向量相等比较：检查两个向量是否完全相等
    // vec: 要比较的向量
    // 返回：如果两个向量的x和y分量都相等则为true
    bool operator==(const Vector2& vec) const
    {
        return x == vec.x && y == vec.y;
    }

    // 向量大小比较：基于向量长度比较
    // vec: 要比较的向量
    // 返回：如果当前向量长度大于参数向量则为true
    bool operator>(const Vector2& vec) const
    {
        return length() > vec.length();
    }

    // 向量小于比较：基于向量长度比较
    // vec: 要比较的向量
    // 返回：如果当前向量长度小于参数向量则为true
    bool operator<(const Vector2& vec) const
    {
        return length() < vec.length();
    }

    // 计算向量的长度(模)
    // 返回：向量的欧几里得长度(√(x²+y²))
    double length() const
    {
        return sqrt(x * x + y * y);
    }

    // 返回向量的标准化形式(单位向量)
    // 返回：与原向量方向相同但长度为1的向量，零向量返回(0,0)
    Vector2 normalize() const
    {
        double len = length();

        if (len == 0)
            return Vector2(0, 0);

        return Vector2(x / len, y / len);
    }

    // 检查向量是否近似为零向量
    // 返回：如果向量长度小于0.0001则为true
    bool approx_zero() const
    {
        return length() < 0.0001;
    }
};

#endif // !_VECTOR2_H_