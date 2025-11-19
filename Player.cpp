// =============================
// Gameplay/Actors/Player.cpp
// =============================
#include "Player.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "AnimationUtil.h"
#include "GameContext.h"
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
    position = { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.5f };
    velocity = {};

    if (currentAnim) currentAnim->Reset();
}

void Player::Update()
{
    using namespace DxPlus::Input;

    int button = GetButtonDown(PLAYER1);
    bool left = (button & BUTTON_LEFT) != 0;
    bool right = (button & BUTTON_RIGHT) != 0;
    bool up = (button & BUTTON_UP) != 0;
    bool down = (button & BUTTON_DOWN) != 0;

    AnimationClip* nextAnim{ nullptr };
    bool isMoving{ false };
    float vx{}, vy{};
    if (left && !right)
    {
        vx = -Const::PLAYER_SPEED;
        nextAnim = &animLeft;
        isMoving = true;
    }
    else if (!left && right)
    {
        vx = Const::PLAYER_SPEED;
        nextAnim = &animRight;
        isMoving = true;
    }
    if (up && !down)
    {
        vy = -Const::PLAYER_SPEED;
        nextAnim = &animUp;
        isMoving = true;
    }
    else if (!up && down)
    {
        vy = Const::PLAYER_SPEED;
        nextAnim = &animDown;
        isMoving = true;
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

    if (shoot)
    {
       DxPlus::Vec2 dir= prevVelocity.Normalize();
       dir*=Const::PROJECTILE_SPEED;
       GC().SpawnProjectile(position,dir);
    }
}

