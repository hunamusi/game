#pragma once
#include "DxPlus/DxPlus.h"

class Wall
{
public:
    Wall(const DxPlus::Vec2& pos, float w, float h)
        : position(pos), width(w), height(h) {
    }

    // 座標（中心座標）
    const DxPlus::Vec2& GetPos() const { return position; }

    // 幅・高さ
    float GetWidth() const { return width; }
    float GetHeight() const { return height; }


    DxPlus::Vec2 position{};
    float width{ 32.0f };
    float height{ 32.0f };
};

