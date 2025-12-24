// =============================
// Gameplay/Actors/Enemy.cpp
// =============================
// 概要: 追跡/接近を主体としたエネミー実装。
// 責務:
//  - 初期出現位置の決定（タイルからランダム、壁/重なり回避）
//  - プレイヤーの位置を参照したシンプルな4方向移動
//  - アニメーションの向き切替と進行
//  - 被弾時の HP 管理とヒットエフェクト発火（GameContext 経由）
// 設計メモ:
//  - コンテキスト（GameContext）は `BindContext` 経由でセットされる前提。未バインドでの `Update` 呼び出しは禁止。
//  - 当たり判定は円形（Radius）で簡易に実装。壁との判定は GameContext 側に委譲。
//  - 入力に応じた移動はデバッグ/同期用途（プレイヤー歩数連動など）に限定し、製品では AI へ置換可能。
#include "Enemy.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "AnimationUtil.h"
#include "map.h"
#include "GameContext.h"
#include <cmath>
#include "Player.h"


void Enemy::Init()
{
    sprite = RM().GridAt(ResourceKeys::Enemy_Yankee, 1, 2);

    AnimationUtil::BuildWalk(animLeft, 2, RM(), ResourceKeys::Enemy_Yankee, 8);
    AnimationUtil::BuildWalk(animRight, 3, RM(), ResourceKeys::Enemy_Yankee, 8);
    AnimationUtil::BuildWalk(animUp, 1, RM(), ResourceKeys::Enemy_Yankee, 8);
    AnimationUtil::BuildWalk(animDown, 0, RM(), ResourceKeys::Enemy_Yankee, 8);

    currentAnim = &animDown;
}

void Enemy::Reset()
{
    // 出現位置は壁タイルを避け、ほかのオブジェクトと重ならないタイル中心へ配置
    int row, col;
    const int maxAttempts = 30;
    int attempts = 0;

    while (attempts < maxAttempts)
    {
        row = rand() % Map::GetRows();
        col = rand() % Map::GetCols();
        attempts++;

        if (Map::GetTileData(row, col) == 1) continue; // 壁は除外

        DxPlus::Vec2 candidate = Map::GetTileCenterPosition(row, col);

        if (GC().IsPositionFree(candidate, Radius(), this))
        {
            position = candidate;
            if (currentAnim) currentAnim->Reset();
            startPosition = position;
            velocity = { 0, 0 };
            return;
        }
    }

    // 妥協: 見つからない場合のフォールバック
    position = Map::GetTileCenterPosition(0, 0);
    velocity = { 0, 0 };
    if (currentAnim) currentAnim->Reset();
    startPosition = position;
}

void Enemy::Update()
{
    using namespace DxPlus::Input;

    // 現在位置を保存（向き変更の基準）
    DxPlus::Vec2 oldPos = position;

    // プレイヤーが存在するか確認（コンテキスト未設定の安全策は製品では assert 推奨）
    if (auto player = GC().GetPlayer())
    {
        if (IsButtonDown(PLAYER1, BUTTON_LEFT) || IsButtonDown(PLAYER1, BUTTON_RIGHT) || IsButtonDown(PLAYER1, BUTTON_UP) || IsButtonDown(PLAYER1, BUTTON_DOWN))
        {
            Player* playerPtr = player;
            if (playerPtr == nullptr) return;

            DxPlus::Vec2 playerPosition = playerPtr->GetPosition();

            // タイル判定用のラムダ（ワールド座標 -> タイルが壁か）
            const float tileW = Map::GetTileWidth();
            const float tileH = Map::GetTileHeight();
            auto IsTileBlocked = [&](const DxPlus::Vec2& worldPos)->bool
                {
                    int col = static_cast<int>(std::floor((worldPos.x - tileW / 2.0f) / tileW));
                    int row = static_cast<int>(std::floor((worldPos.y - tileH / 2.0f) / tileH));
                    if (row < 0 || row >= Map::GetRows() || col < 0 || col >= Map::GetCols()) return true; // 範囲外は通れない扱い
                    return Map::GetTileData(row, col) == 1;
                };

            // 移動意図（移動不可でも向きだけは反映）
            bool intendedSet = false;
            EnemyDirection intendedDir = direction;

            // プレイヤーとの差分
            float diffX = playerPosition.x - position.x;
            float diffY = playerPosition.y - position.y;
            float absX = std::fabs(diffX);
            float absY = std::fabs(diffY);
            const float EPS = 0.0001f;

            // ヘルパー: X移動試行
            auto TryMoveX = [&]()
                {
                    if (std::fabs(diffX) <= EPS) return;
                    EnemyDirection dirX = diffX > 0.0f ? EnemyDirection::Right : EnemyDirection::Left;
                    if (!intendedSet) { intendedDir = dirX; intendedSet = true; }

                    float candidateX = position.x + (diffX > 0.0f ? 120.0f : -120.0f);
                    DxPlus::Vec2 newPosX = { candidateX, position.y };

                    if (!IsTileBlocked(newPosX) && GC().IsPositionFree(newPosX, Radius(), this))
                    {
                        position.x = candidateX;
                    }
                };

            // ヘルパー: Y移動試行
            auto TryMoveY = [&]()
                {
                    if (std::fabs(diffY) <= EPS) return;
                    EnemyDirection dirY = diffY > 0.0f ? EnemyDirection::Down : EnemyDirection::Up;
                    if (!intendedSet) { intendedDir = dirY; intendedSet = true; }

                    float candidateY = position.y + (diffY > 0.0f ? 120.0f : -120.0f);
                    DxPlus::Vec2 newPosY = { position.x, candidateY };

                    if (!IsTileBlocked(newPosY) && GC().IsPositionFree(newPosY, Radius(), this))
                    {
                        position.y = candidateY;
                    }
                };

            // 優先軸（より大きく動く軸を先に試す）
            if (absX >= absY)
            {
                TryMoveX();
                TryMoveY();
            }
            else
            {
                TryMoveY();
                TryMoveX();
            }

            // 移動を試みた（または試みた向きがある）場合は向きを反映
            if (intendedSet)
            {
                AnimationClip* newAnim = currentAnim;
                switch (intendedDir)
                {
                case EnemyDirection::Down:  newAnim = &animDown;  break;
                case EnemyDirection::Left:  newAnim = &animLeft;  break;
                case EnemyDirection::Up:    newAnim = &animUp;    break;
                case EnemyDirection::Right: newAnim = &animRight; break;
                }

                if (newAnim != currentAnim)
                {
                    currentAnim = newAnim;
                    currentAnim->Reset();
                }
                direction = intendedDir;
            }
        }

        // NOTE: ヘルスバー描画は CameraDraw で実施（スクリーン座標化済）
    }

    if (currentAnim) currentAnim->Update();
}

void Enemy::CameraDraw(float camX, float camY)
{
    // スプライト / アニメを描画（基底実装を利用）
    Entity2D::CameraDraw(camX, camY);

    // HPバーをスクリーン座標で描画
    const int MAX_HP = 20;
    float hpRatio = (MAX_HP <= 0) ? 0.0f : (static_cast<float>(hp) / static_cast<float>(MAX_HP));
    if (hpRatio < 0.0f) hpRatio = 0.0f;
    if (hpRatio > 1.0f) hpRatio = 1.0f;

    DxPlus::Vec2 screenPos = position - DxPlus::Vec2{ camX, camY };

    const float frameW = 80.0f;
    const float frameH = 8.0f;
    float left = screenPos.x - frameW * 0.5f;
    float top = screenPos.y -120.0f;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
    float innerW = (frameW - 2.0f) * hpRatio;
    if (innerW < 0.0f) innerW = 0.0f;
    DxPlus::Primitive2D::DrawRect({ left + 1.0f, top + 1.0f }, { innerW, frameH - 2.0f }, GetColor(0, 255, 0));
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Enemy::OnHit(int atk) noexcept
{
    int damege = atk < 1 ? 1 : atk;
    int prevHp = hp;
    hp = std::max(hp - damege, 0);

    // HP が減少したタイミングでエフェクト再生
    if (hp < prevHp)
    {
        if (gc) gc->SpawnEnemyHitEffect(position);
    }

    if (hp <= 0) { alive = 0; }
}
