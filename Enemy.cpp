//// =============================
//// Gameplay/Actors/Enemy.cpp
//// =============================
//#include "Enemy.h"
//#include "ResourceManager.h"
//#include "ResourceKeys.h"
//#include "Consts.h"
//#include "AnimationUtil.h"
//#include "map.h"
//#include "GameContext.h"
//#include <cmath>
//#include "Player.h"
//
//
//void Enemy::Init()
//{
//    sprite = RM().GridAt(ResourceKeys::Enemy_Yankee, 1, 2);
//
//    AnimationUtil::BuildWalk(animLeft, 3, RM(), ResourceKeys::Enemy_Yankee, 8);
//    AnimationUtil::BuildWalk(animRight, 1, RM(), ResourceKeys::Enemy_Yankee, 8);
//    AnimationUtil::BuildWalk(animUp, 0, RM(), ResourceKeys::Enemy_Yankee, 8);
//    AnimationUtil::BuildWalk(animDown, 2, RM(), ResourceKeys::Enemy_Yankee, 8);
//
//    currentAnim = &animDown;
//
//
//}
//
//void Enemy::Reset()
//{
//    int row, col;
//    const int maxAttempts = 30;
//    int attempts = 0;
//
//    while (attempts < maxAttempts)
//    {
//        row = rand() % Map::GetRows();
//        col = rand() % Map::GetCols();
//        attempts++;
//
//        // 壁タイルならスキップ
//        if (Map::GetTileData(row, col) == 1) continue;
//
//        // タイル中心座標を取得
//        DxPlus::Vec2 candidate = Map::GetTileCenterPosition(row, col);
//
//        // 他オブジェクトと衝突しないか確認
//        if (GC().IsPositionFree(candidate, Radius(), this))
//        {
//            position = candidate;
//            if (currentAnim) currentAnim->Reset();
//            startPosition = position;
//            // 初期は静止でもよい
//            velocity = { 0, 0 };
//            return;
//        }
//    }
//
//    // 見つからなかった場合はデフォルト位置
//    position = Map::GetTileCenterPosition(0, 0);
//    velocity = { 0, 0 };
//    if (currentAnim) currentAnim->Reset();
//    startPosition = position;
//}
//
//void Enemy::Update()
//{
//    using namespace DxPlus::Input;
//
//    // 現在位置を保存（向き変更の基準）
//    DxPlus::Vec2 oldPos = position;
//
//    // プレイヤーが存在するか確認
//    if (auto player = GC().GetPlayer())
//    {
//        if (IsButtonDown(PLAYER1, BUTTON_LEFT) || IsButtonDown(PLAYER1, BUTTON_RIGHT) || IsButtonDown(PLAYER1, BUTTON_UP) || IsButtonDown(PLAYER1, BUTTON_DOWN))
//        {
//            Player* playerPtr = player;
//            if (playerPtr == nullptr) return;
//
//            DxPlus::Vec2 playerPosition = playerPtr->GetPosition();
//
//            // タイル判定用のラムダ（ワールド座標 -> タイルが壁か）
//            const float tileW = Map::GetTileWidth();
//            const float tileH = Map::GetTileHeight();
//            auto IsTileBlocked = [&](const DxPlus::Vec2& worldPos)->bool
//                {
//                    int col = static_cast<int>(std::floor((worldPos.x - tileW / 2.0f) / tileW));
//                    int row = static_cast<int>(std::floor((worldPos.y - tileH / 2.0f) / tileH));
//                    if (row < 0 || row >= Map::GetRows() || col < 0 || col >= Map::GetCols()) return true; // 範囲外は通れない扱い
//                    return Map::GetTileData(row, col) == 1;
//                };
//
//            // 移動意図（移動はせずとも向くだけでも反映する）
//            bool intendedSet = false;
//            EnemyDirection intendedDir = direction;
//
//            // プレイヤーとの差分
//            float diffX = playerPosition.x - position.x;
//            float diffY = playerPosition.y - position.y;
//            float absX = std::fabs(diffX);
//            float absY = std::fabs(diffY);
//            const float EPS = 0.0001f;
//
//            // ヘルパー: X移動試行
//            auto TryMoveX = [&]()
//                {
//                    if (std::fabs(diffX) <= EPS) return;
//                    EnemyDirection dirX = diffX > 0.0f ? EnemyDirection::Right : EnemyDirection::Left;
//                    // 移動しようとした向きを記録（移動できなかった場合も向くだけはする）
//                    if (!intendedSet) { intendedDir = dirX; intendedSet = true; }
//
//                    float candidateX = position.x + (diffX > 0.0f ? 120.0f : -120.0f);
//                    DxPlus::Vec2 newPosX = { candidateX, position.y };
//
//                    // タイルが壁ではないこと、かつ他オブジェクトと衝突しないことを両方確認
//                    if (!IsTileBlocked(newPosX) && GC().IsPositionFree(newPosX, Radius(), this))
//                    {
//                        position.x = candidateX;
//                    }
//                };
//
//            // ヘルパー: Y移動試行
//            auto TryMoveY = [&]()
//                {
//                    if (std::fabs(diffY) <= EPS) return;
//                    EnemyDirection dirY = diffY > 0.0f ? EnemyDirection::Down : EnemyDirection::Up;
//                    // 移動しようとした向きを記録（移動できなかった場合も向くだけはする）
//                    if (!intendedSet) { intendedDir = dirY; intendedSet = true; }
//
//                    float candidateY = position.y + (diffY > 0.0f ? 120.0f : -120.0f);
//                    DxPlus::Vec2 newPosY = { position.x, candidateY };
//
//                    // タイルが壁ではないこと、かつ他オブジェクトと衝突しないことを両方確認
//                    if (!IsTileBlocked(newPosY) && GC().IsPositionFree(newPosY, Radius(), this))
//                    {
//                        position.y = candidateY;
//                    }
//                };
//
//            // 優先軸（より大きく動く軸を先に試す）
//            if (absX >= absY)
//            {
//                TryMoveX();
//                TryMoveY();
//            }
//            else
//            {
//                TryMoveY();
//                TryMoveX();
//            }
//
//            // 移動を試みた（または試みた向きがある）場合は向きを反映する
//            if (intendedSet)
//            {
//                AnimationClip* newAnim = currentAnim;
//                switch (intendedDir)
//                {
//                case EnemyDirection::Down:
//                    newAnim = &animDown;
//                    break;
//                case EnemyDirection::Left:
//                    newAnim = &animLeft;
//                    break;
//                case EnemyDirection::Up:
//                    newAnim = &animUp;
//                    break;
//                case EnemyDirection::Right:
//                    newAnim = &animRight;
//                    break;
//                }
//
//                // アニメ・向きを切り替え（移動できなかった場合でも切り替え）
//                if (newAnim != currentAnim)
//                {
//                    currentAnim = newAnim;
//                    currentAnim->Reset();
//                }
//                direction = intendedDir;
//            }
//        }
//
//        const int MAX_HP = 20;
//        float hpRatio = (MAX_HP <= 0) ? 0.0f : (static_cast<float>(hp) / static_cast<float>(MAX_HP));
//        if (hpRatio < 0.0f) hpRatio = 0.0f;
//        if (hpRatio > 1.0f) hpRatio = 1.0f;
//
//        // フレーム（黒）
//        DxPlus::Primitive2D::DrawRect(position, { 5000,10000 }, GetColor(0, 0, 0));
//        // 内部（赤）をHP比率で横幅だけ変える
//        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
//        // 内部幅は 49 を基準にする（元のコードに合わせる）
//        float innerHalfW = 49.0f * 0.5f * hpRatio;
//        float innerH = 99.0f;
//        // DrawRect は中心ベースで描く想定なので、左寄せではなく中央から幅を縮める方式
//        DxPlus::Primitive2D::DrawRect(position, { innerHalfW * 2.0f, innerH }, GetColor(255, 0, 0));
//        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
//
//    }
//    if (currentAnim) currentAnim->Update();
//
//}
//
//
//
//
//
//void Enemy::OnHit(int atk) noexcept
//{
//    int damege = atk < 1 ? 1 : atk;
//    hp = std::max(hp - damege, 0);
//    if (hp <= 0) { alive = 0; }
//}
//
//
//
//

// =============================
// Gameplay/Actors/Enemy.cpp
// =============================
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

    AnimationUtil::BuildWalk(animLeft, 3, RM(), ResourceKeys::Enemy_Yankee, 8);
    AnimationUtil::BuildWalk(animRight, 1, RM(), ResourceKeys::Enemy_Yankee, 8);
    AnimationUtil::BuildWalk(animUp, 0, RM(), ResourceKeys::Enemy_Yankee, 8);
    AnimationUtil::BuildWalk(animDown, 2, RM(), ResourceKeys::Enemy_Yankee, 8);

    currentAnim = &animDown;


}

void Enemy::Reset()
{
    int row, col;
    const int maxAttempts = 30;
    int attempts = 0;

    while (attempts < maxAttempts)
    {
        row = rand() % Map::GetRows();
        col = rand() % Map::GetCols();
        attempts++;

        // 壁タイルならスキップ
        if (Map::GetTileData(row, col) == 1) continue;

        // タイル中心座標を取得
        DxPlus::Vec2 candidate = Map::GetTileCenterPosition(row, col);

        // 他オブジェクトと衝突しないか確認
        if (GC().IsPositionFree(candidate, Radius(), this))
        {
            position = candidate;
            if (currentAnim) currentAnim->Reset();
            startPosition = position;
            // 初期は静止でもよい
            velocity = { 0, 0 };
            return;
        }
    }

    // 見つからなかった場合はデフォルト位置
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

    // プレイヤーが存在するか確認
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

            // 移動意図（移動はせずとも向くだけでも反映する）
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
                    // 移動しようとした向きを記録（移動できなかった場合も向くだけはする）
                    if (!intendedSet) { intendedDir = dirX; intendedSet = true; }

                    float candidateX = position.x + (diffX > 0.0f ? 120.0f : -120.0f);
                    DxPlus::Vec2 newPosX = { candidateX, position.y };

                    // タイルが壁ではないこと、かつ他オブジェクトと衝突しないことを両方確認
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
                    // 移動しようとした向きを記録（移動できなかった場合も向くだけはする）
                    if (!intendedSet) { intendedDir = dirY; intendedSet = true; }

                    float candidateY = position.y + (diffY > 0.0f ? 120.0f : -120.0f);
                    DxPlus::Vec2 newPosY = { position.x, candidateY };

                    // タイルが壁ではないこと、かつ他オブジェクトと衝突しないことを両方確認
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

            // 移動を試みた（または試みた向きがある）場合は向きを反映する
            if (intendedSet)
            {
                AnimationClip* newAnim = currentAnim;
                switch (intendedDir)
                {
                case EnemyDirection::Down:
                    newAnim = &animDown;
                    break;
                case EnemyDirection::Left:
                    newAnim = &animLeft;
                    break;
                case EnemyDirection::Up:
                    newAnim = &animUp;
                    break;
                case EnemyDirection::Right:
                    newAnim = &animRight;
                    break;
                }

                // アニメ・向きを切り替え（移動できなかった場合でも切り替え）
                if (newAnim != currentAnim)
                {
                    currentAnim = newAnim;
                    currentAnim->Reset();
                }
                direction = intendedDir;
            }
        }

        // NOTE:
        // ヘルスバー描画をここ（Update）で行うと「ワールド座標のままスクリーンに描画」してしまい、
        // カメラの位置や描画順によって他オブジェクトに被されたり巨大な矩形が画面全体を覆う原因になります。
        // 描画は CameraDraw で行うように変更しました（CameraDraw はカメラ補正済のスクリーン座標で描画されます）。
    }

    if (currentAnim) currentAnim->Update();


}

// CameraDraw をオーバーライドして、スプライトの描画後にカメラ補正済みの座標でHPバーを描画する
void Enemy::CameraDraw(float camX, float camY)
{
    // スプライト / アニメを描画（基底実装を利用）
    Entity2D::CameraDraw(camX, camY);

    // HPバーをスクリーン座標で描画
    const int MAX_HP = 20;
    float hpRatio = (MAX_HP <= 0) ? 0.0f : (static_cast<float>(hp) / static_cast<float>(MAX_HP));
    if (hpRatio < 0.0f) hpRatio = 0.0f;
    if (hpRatio > 1.0f) hpRatio = 1.0f;

    // ワールド -> スクリーン
    DxPlus::Vec2 screenPos = position - DxPlus::Vec2{ camX, camY };

    // サイズ・位置調整（必要に応じて微調整してください）
    const float frameW = 80.0f;
    const float frameH = 12.0f;
    float left = screenPos.x - frameW * 0.5f;
    float top = screenPos.y;

    // フレーム（黒）
    DxPlus::Primitive2D::DrawRect({ left, top }, { frameW, frameH }, GetColor(0, 0, 0));
    // 内部（赤）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
    float innerW = (frameW - 2.0f) * hpRatio;
    if (innerW < 0.0f) innerW = 0.0f;
    DxPlus::Primitive2D::DrawRect({ left + 1.0f, top + 1.0f }, { innerW, frameH - 2.0f }, GetColor(255, 0, 0));
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Enemy::OnHit(int atk) noexcept
{
    int damege = atk < 1 ? 1 : atk;
    hp = std::max(hp - damege, 0);
    if (hp <= 0) { alive = 0; }
}
