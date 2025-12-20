// 概要: プレイヤー弾（プロジェクタイル）の実装。
// 責務:
//  - 位置/速度の保持とフレーム更新（Step）
//  - 画面外判定による自己破棄
//  - スプライトのスケール描画（当たり判定サイズに追従）
// 設計メモ:
//  - 生成は GameContext::SpawnProjectile 経由（プールと整合）。
//  - 描画はカメラ相対座標で行うため、`CameraDraw` 内でスクリーン変換。
//  - ヒット検出は GameContext 側で弾と敵の突き合わせを実行。
#include "Projectile.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "Player.h"
#include "DxPlus/Sprite.h"
#include "DxLib.h"
#include <cmath>
#include"GameContext.h"

void Projectile::Init()
{
    sprite = RM().GridAt(ResourceKeys::Player_Shot);
}

void Projectile::Reset(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel, const DxPlus::Vec2& s) noexcept
{
    // スポーン時の初期化（位置・速度・サイズ）
    position = pos;
    velocity = vel;
    size = s;
}

void Projectile::Step()
{
    // 1 フレーム分の移動（基底の速度反映）
    Entity2D::Step();

    // シンプルな画面外判定。ワールド境界は暫定値。
    if (position.x < 0 || position.x > 3500
        || position.y < 0 || position.y > 2500)
    {
        Kill();
    }
}

void Projectile::CameraDraw(float camX, float camY)
{
    // ワールド -> スクリーン
    DxPlus::Vec2 camPos = position - DxPlus::Vec2{ camX, camY };

    // 当たりサイズに基づく描画矩形の算出
    const float halfWf = size.x * 0.5f;
    const float halfHf = size.y * 0.5f;
    const int gx = static_cast<int>(std::round(camPos.x));
    const int gy = static_cast<int>(std::round(camPos.y));
    const int left = gx - static_cast<int>(halfWf);
    const int top = gy - static_cast<int>(halfHf);
    const int right = gx + static_cast<int>(halfWf);
    const int bottom = gy + static_cast<int>(halfHf);

    int Attractiveness = GC().GetAttractiveness();
    (void)Attractiveness; // 現状はサイズ/描画に未使用なら警告抑止

    size = { 64.0f,64.0f }; // TODO: 定数化（Const::PROJECTILE_SIZE 等）
    camPos.x -= 20;         // TODO: スプライト原点調整を汎用化

    // スプライトがあればスケールして描画
    if (sprite && sprite->IsLoaded()) {
        int w = 0, h = 0;
        DxLib::GetGraphSize(sprite->GetID(), &w, &h);
        DxPlus::Vec2 scale{ 1.0f, 1.0f };
        if (w > 0 && h > 0) {
            scale.x = size.x / static_cast<float>(w);
            scale.y = size.y / static_cast<float>(h);
        }
        sprite->Draw(camPos, scale);
        return;
    }

    // フォールバック: 単色矩形での描画
    if (graphHandle > 0) {
        DrawExtendGraph(left, top, right, bottom, graphHandle, TRUE);
    }
}


