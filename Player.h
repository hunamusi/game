// =============================
// Gameplay/Actors/Player.h
// =============================
#pragma once
#include "Entity2D.h"

class Player : public Entity2D
{
public:
    Player() = default;
    ~Player() = default;

    // ライフサイクル処理
    void Init() override;
    void Reset() override;
    void Update() override;
    int GetPlayerCount()const { return PlayerCount; }
    const DxPlus::Vec2& GetPrevPosition() const { return prevPos; }
private:
    DxPlus::Vec2 prevVelocity = { 0,1 };
    int PlayerCount = 20;
    DxPlus::Vec2 prevPos{};
    static constexpr float HALF_WIDTH = 30;
    static constexpr float HEIGHT = 108;


};
