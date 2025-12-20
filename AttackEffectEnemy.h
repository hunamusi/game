#pragma once
#include "Entity2D.h"
#include "AnimationClip.h"

// エネミー用の攻撃（ヒット）エフェクトを管理する小さなエンティティ
class AttackEffectEnemy : public Entity2D
{
public:
    AttackEffectEnemy() = default;
    ~AttackEffectEnemy() = default;

    void Init() override;
    void Reset() override;
    void Update() override;
    void CameraDraw(float camX, float camY) override;

    // 再生を指定のワールド座標で開始する
    void PlayAt(const DxPlus::Vec2& worldPos) noexcept;

private:
    AnimationClip anim;
    int ticksRemaining{0};
};
