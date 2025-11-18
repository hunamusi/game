// =============================
// Gameplay/Actors/Enemy.cpp
// =============================
#include "Enemy.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "AnimationUtil.h"

void Enemy::Init()
{
    sprite = RM().GridAt(ResourceKeys::Enemy_Yankee, 1, 2);

    AnimationUtil::BuildWalk(animLeft,  3, RM(), ResourceKeys::Enemy_Yankee, 8);
    AnimationUtil::BuildWalk(animRight, 1, RM(), ResourceKeys::Enemy_Yankee, 8);
    AnimationUtil::BuildWalk(animUp,    0, RM(), ResourceKeys::Enemy_Yankee, 8);
    AnimationUtil::BuildWalk(animDown,  2, RM(), ResourceKeys::Enemy_Yankee, 8);

    currentAnim = &animDown;
}

void Enemy::Reset()
{
    position = {
        static_cast<float>(rand()%DxPlus::CLIENT_WIDTH), static_cast<float>(rand()%DxPlus::CLIENT_HEIGHT)
    };
    velocity = { 0, 4 };

    if (currentAnim) currentAnim->Reset();
    startPosition = position;
}

void Enemy::Update()
{
    switch (direction)
    {
    case EnemyDirection::Down:
        if (position.y >= startPosition.y + Const::MOVE_DISTANCE) SetMoveDirection(EnemyDirection::Left); break;
    case EnemyDirection::Left:
        if (position.x <= startPosition.x - Const::MOVE_DISTANCE) SetMoveDirection(EnemyDirection::Up); break;
    case EnemyDirection::Up:
        if (position.y <= startPosition.y - Const::MOVE_DISTANCE) SetMoveDirection(EnemyDirection::Right); break;
    case EnemyDirection::Right:
        if (position.x >= startPosition.x + Const::MOVE_DISTANCE) SetMoveDirection(EnemyDirection::Down); break;
        break;
    }

    if (currentAnim) currentAnim->Update();
}

void Enemy::OnHit(int atk) noexcept
{
    int damege = atk < 1 ? 1 : atk;
    hp = std::max(hp-damege, 0);
    if (hp <= 0){alive = 0;}
}

void Enemy::SetMoveDirection(EnemyDirection dir)
{
    direction = dir;
    switch (direction)
    {
    case EnemyDirection::Down:
        velocity = { 0, Const::ENEMY_SPEED };
        currentAnim = &animDown;
        break;
    case EnemyDirection::Left:
        velocity = { -Const::ENEMY_SPEED, 0 };
        currentAnim = &animLeft;
        break;
    case EnemyDirection::Up:
        velocity = { 0, -Const::ENEMY_SPEED };
        currentAnim = &animUp;
        break;
    case EnemyDirection::Right:
        velocity = { Const::ENEMY_SPEED, 0 };
        currentAnim = &animRight;
        break;
    }
}
