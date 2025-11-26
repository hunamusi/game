// =============================
// Gameplay/Actors/Enemy.cpp
// =============================
#include "Enemy.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "AnimationUtil.h"
#include "GameContext.h"
#include "Explosion.h"
#include "Player.h"
#include <cmath>

void Enemy::Init()
{


    sprite = RM().GridAt(ResourceKeys::Enemy);
}

void Enemy::Reset()
{
    // 初期生成とリトライ
    const int maxAttempts = 20;
    int attempts = 0;
    do
    {
        position = { static_cast<float>(rand() % DxPlus::CLIENT_WIDTH), static_cast<float>(rand() % DxPlus::CLIENT_HEIGHT) };
        snapToGrid(position);
        ++attempts;
        // ループはここで止まり、IsPositionFree を使って他のエネミーと重ならない位置を探す
    } while (attempts < maxAttempts && !GC().IsPositionFree(position, Radius(), this));

    // 最終的に見つからなければそのまま配置（重複はまれ）
}

void Enemy::Update()
{
    using namespace DxPlus::Input;

    if (IsButtonDown(PLAYER1, BUTTON_TRIGGER1))
    {
        const int maxAttempts = 20;
        int attempts = 0;
        DxPlus::Vec2 candidate;
        do
        {
            candidate = { static_cast<float>(rand() % DxPlus::CLIENT_WIDTH), static_cast<float>(rand() % DxPlus::CLIENT_HEIGHT) };
            snapToGrid(candidate);
            ++attempts;
        } while (attempts < maxAttempts && !GC().IsPositionFree(candidate, Radius(), this));

        position = candidate;
    }

    if (IsButtonDown(PLAYER1, BUTTON_START))
    {
        Player* playerPtr = GC().GetPlayer();
        if (playerPtr == nullptr) return;

        DxPlus::Vec2 playerPosition = playerPtr->GetPosition();

        // X 軸移動候補
        float diffX = playerPosition.x - position.x;
        if (std::fabs(diffX) > 0.0001f)
        {
            float candidateX = position.x + (diffX > 0.0f ? 120.0f : -120.0f);
            DxPlus::Vec2 newPosX = { candidateX, position.y };
            if (GC().IsPositionFree(newPosX, Radius(), this))
            {
                position.x = candidateX;
            }
        }

        // Y 軸移動候補（X移動後の位置を基準にチェックする）
        float diffY = playerPosition.y - position.y;
        if (std::fabs(diffY) > 0.0001f)
        {
            float candidateY = position.y + (diffY > 0.0f ? 120.0f : -120.0f);
            DxPlus::Vec2 newPosY = { position.x, candidateY };
            if (GC().IsPositionFree(newPosY, Radius(), this))
            {
                position.y = candidateY;
            }
        }
    }


}