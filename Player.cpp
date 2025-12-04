// =============================
// Gameplay/Actors/Player.cpp
// =============================
#include "Player.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "AnimationUtil.h"
#include "GameContext.h"

int PlayerWalkCount = 0;

void Player::Init()
{
    sprite = RM().GridAt(ResourceKeys::Player, 1, 2);

    AnimationUtil::BuildWalk(animLeft,  3, RM(), ResourceKeys::Player, 8);
    AnimationUtil::BuildWalk(animRight, 1, RM(), ResourceKeys::Player, 8);
    AnimationUtil::BuildWalk(animUp,    0, RM(), ResourceKeys::Player, 8);
    AnimationUtil::BuildWalk(animDown,  2, RM(), ResourceKeys::Player, 8);

    currentAnim = &animDown;
}

void Player::Reset()
{
    position = {540 , 300 };
    velocity = {};

    if (currentAnim) currentAnim->Reset();
}

void Player::Update()
{
    prevPos = position;
    using namespace DxPlus::Input;

    int button = GetButtonDown(PLAYER1);
    bool left = (button & BUTTON_LEFT) != 0;
    bool right = (button & BUTTON_RIGHT) != 0;
    bool up = (button & BUTTON_UP) != 0;
    bool down = (button & BUTTON_DOWN) != 0;

    AnimationClip* nextAnim{ nullptr };
    bool isMoving{ false };
    float vx{}, vy{};
    if (PlayerCount >= 0)
    {
        if (left && !right)
        {

            vx = -Const::PLAYER_SPEED;
            nextAnim = &animLeft;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCount = std::min(PlayerWalkCount, Const::MAX_PLAYER_WALK_COUNT);

        }
        else if (!left && right)
        {

            vx = Const::PLAYER_SPEED;
            nextAnim = &animRight;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCount = std::min(PlayerWalkCount, Const::MAX_PLAYER_WALK_COUNT);
        }
        if (up && !down)
        {
            vy = -Const::PLAYER_SPEED;
            nextAnim = &animUp;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCount = std::min(PlayerWalkCount, Const::MAX_PLAYER_WALK_COUNT);
        }
        else if (!up && down)
        {
            vy = Const::PLAYER_SPEED;
            nextAnim = &animDown;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCount = std::min(PlayerWalkCount, Const::MAX_PLAYER_WALK_COUNT);
        }
    }


    velocity = { vx, vy };
    if (vx != 0 || vy != 0)
    {
        prevVelocity = velocity;
    }
    if (nextAnim && currentAnim != nextAnim)
    {
        currentAnim = nextAnim;
        currentAnim->Reset();
    }

    if (isMoving && currentAnim) currentAnim->Update();
    int buttonDown = GetButtonDown(PLAYER1);
    bool shoot = (buttonDown & BUTTON_TRIGGER2) != 0;

    if (shoot && PlayerWalkCount == Const::MAX_PLAYER_WALK_COUNT)
    {
        DxPlus::Vec2 dir{ 0.0f, 0.0f };
        const float EPS = 0.0001f;
        if (std::fabs(prevVelocity.x) > EPS || std::fabs(prevVelocity.y) > EPS)
        {
            dir = prevVelocity.Normalize();
        }
        else
        {
            if (currentAnim == &animLeft) dir = DxPlus::Vec2{ -1.0f, 0.0f };
            else if (currentAnim == &animRight) dir = DxPlus::Vec2{ 1.0f, 0.0f };
            else if (currentAnim == &animUp) dir = DxPlus::Vec2{ 0.0f, -1.0f };
            else dir = DxPlus::Vec2{ 0.0f, 1.0f }; // down
        }

        dir = dir.Normalize() * Const::PROJECTILE_SPEED;
        DxPlus::Vec2 spawnPos = { position.x - 10, position.y - 40 };

        // 変更: SpawnProjectile の成否を受け取り、成功時のみカウントをリセット
        bool spawned = GC().SpawnProjectile(spawnPos, dir);
        if (spawned) {
            PlayerWalkCount = 0;
        }
    }


}
