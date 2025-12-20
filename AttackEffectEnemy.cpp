// =============================================
// 概要: エネミー被弾時に再生する小規模な一時エフェクト実体。
// 責務:
//  - アニメーションフレームの管理（`AnimationClip`）
//  - ワールド座標での位置管理と、カメラ相対の描画
//  - ライフタイム（ticksRemaining）での自己消滅
// 設計メモ:
//  - エフェクトはゲームプレイロジックに影響しないため、当たり判定や速度更新は行いません。
//  - プール運用前提（GameContext 側で非アクティブな個体を再利用）。
//  - 依存関係はリソース（`ResourceManager`）と描画のみ。スレッドセーフは想定しません（メインスレッド更新想定）。
// =============================================
#include "AttackEffectEnemy.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"

void AttackEffectEnemy::Init()
{
    // スプライトアトラスからフレームを構築
    anim.Clear();
    for (int x = 0; x < 10; ++x)
    {
        const DxPlus::Sprite::SpriteBase* sp = RM().GridAt(ResourceKeys::AttackEffect1, x, 0);
        if (sp) anim.AddFrame(sp, 5); // 1 フレーム 5 tick
    }
    anim.Reset();
    ticksRemaining = 0;
    alive = false; // 生成直後は非アクティブ
}

void AttackEffectEnemy::Reset()
{
    // プール復帰時の初期化（再生位置・残カウントをクリア）
    ticksRemaining = 0;
    alive = false;
}

void AttackEffectEnemy::PlayAt(const DxPlus::Vec2& worldPos) noexcept
{
    // 指定ワールド座標から再生を開始
    position = worldPos;
    anim.Reset();
    ticksRemaining = 10 * 5; // 10 フレーム * 5 tick
    alive = true;
}

void AttackEffectEnemy::Update()
{
    // シンプルな寿命管理とフレーム進行のみ
    if (ticksRemaining > 0)
    {
        anim.Update();
        --ticksRemaining;
        if (ticksRemaining == 0) alive = false;
    }
}

void AttackEffectEnemy::CameraDraw(float camX, float camY)
{
    // カメラ補正済みスクリーン座標での描画
    if (ticksRemaining > 0)
    {
        DxPlus::Vec2 screenPos = position - DxPlus::Vec2{ camX, camY };
        anim.Draw(screenPos);
    }
}
