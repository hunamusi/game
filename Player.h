// =============================
// Gameplay/Actors/Player.h
// =============================
#pragma once
#include "Entity2D.h"

class Sword;
class Player : public Entity2D
{
public:
    Player() = default;
    ~Player() = default;

    // ライフサイクル処理
    void Init() override;
    void Reset() override;
    void Update() override;
    void Step() override;

    bool IsSwordSwinging() const noexcept { return isSwordSwinging; }
    bool CanStartSword() const noexcept { return !isSwordSwinging; }
    int facingSign() const noexcept { return facingLeft ? -1 : 1; }

private:
    DxPlus::Vec2 prevVelocity{ 0,1 };
    int shootInterval{};
    int rotate{};
    bool jumpPressed{ false };
    bool jumpHeld{ false };
    //剣で追加
    bool isSwordSwinging = false;
    int swordTimer = 0;
    bool facingLeft = false;

    Sword* sword = nullptr;   

};
