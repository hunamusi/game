// =============================
// Gameplay/Actors/Player.h
// =============================
#pragma once
#include "Entity2D.h"

enum class EquipType {
    None,
    Ishab,
    Sword,
    Shield
};

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
    void Equip(EquipType newEquip);

    void CameraDraw(float camX, float camY) override;

private:
    DxPlus::Vec2 prevVelocity = { 0,1 };
    int PlayerCount = 20;
    DxPlus::Vec2 prevPos{};
    static constexpr float HALF_WIDTH = 30;
    static constexpr float HEIGHT = 108;

    bool ishab = false;
    EquipType currentEquip = EquipType::None;
    int attack = 10;
    int defense = 5;

    // 追加: 攻撃時に特定フレームを表示するための状態
    bool isAttacking{ false };
    int attackTicks{ 0 };
    static constexpr int ATTACK_DRAW_TICKS = 30; // 表示継続ティック数 

};
