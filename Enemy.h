// =============================
// Gameplay/Actors/Enemy.h
// =============================
#pragma once
#include "Entity2D.h"

enum class EnemyDirection : uint8_t
{
    Down, Left, Up, Right, 
};
class Enemy : public Entity2D
{
public:
    Enemy() = default;
    ~Enemy() = default;

    void Init() override;
    void Reset() override;
    void Update() override;
    [[nodiscard]] virtual bool IsDamageable()const noexcept override { return true ; }
    void OnHit(int atk)noexcept override;
    void CameraDraw(float camX, float camY) override;
	
	
private:

    EnemyDirection direction{ EnemyDirection::Down };
    DxPlus::Vec2 startPosition;

    int hp{ 20 };

    bool isShotHit{ false };
};
