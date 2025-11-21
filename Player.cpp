// =============================
// Gameplay/Actors/Player.cpp
// =============================
#include "Player.h"
#include <algorithm>
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "AnimationUtil.h"
#include "GameContext.h"
#include "Sword.h"
#include "Flame.h"

void Player::Init()
{
    sprite = RM().GridAt(ResourceKeys::Player, 1, 2);

    AnimationUtil::BuildWalk(animLeft,  3, RM(), ResourceKeys::Player, 8);
    AnimationUtil::BuildWalk(animRight, 1, RM(), ResourceKeys::Player, 8);
    //AnimationUtil::BuildWalk(animUp,    0, RM(), ResourceKeys::Player, 8);
    //AnimationUtil::BuildWalk(animDown,  2, RM(), ResourceKeys::Player, 8);

    currentAnim = &animRight;
}

void Player::Reset()
{
    position = {570,480};
    velocity = {};

    if (currentAnim) currentAnim->Reset();
}

#define BOOLSTR(b) ((b) ? L"true" : L"false")

void Player::Update()
{
    using namespace DxPlus::Input;
    int button = GetButton(PLAYER1);

    bool left = (button & BUTTON_LEFT) != 0;
    bool right = (button & BUTTON_RIGHT) != 0;

    // キャラクターが動く方向
    float moveDir = { 0.0f };
    if (left ^ right)
    {
        moveDir = left ? -1.0f : 1.0f;
    }

    // キャラクターが目指す速度
    float targetSpeed = moveDir * Const::PLAYER_MAX_SPEED;

    // 急ブレーキ中か
    bool isBraking{ false };
    if (std::abs(velocity.x) > Const::EPS &&
        moveDir != 0.0f &&
        velocity.x * targetSpeed < 0.0f)
    {
        isBraking = true;
    }

    //  1フレームで現在の速度をどれだけ目標速度に近づけるか
    float step{ 0.0f };
    if (moveDir != 0.0f)
    {
        step = (isBraking) ? Const::PLAYER_BLAKE : Const::PLAYER_ACCELERATION;
    }
    else
    {
        step = Const::PLAYER_DECELERATION;
    }

    // 目標速度に近づける
    if (velocity.x < targetSpeed) velocity.x = std::min(velocity.x + step, targetSpeed);
    if (velocity.x > targetSpeed) velocity.x = std::max(velocity.x - step, targetSpeed);

    // プレイヤーの速度を制限
    velocity.x = std::clamp(velocity.x, -Const::PLAYER_MAX_SPEED, Const::PLAYER_MAX_SPEED);

    // プレイヤーのジャンプ入力処理
   /* jumpPressed = GetButtonDown(PLAYER1) & BUTTON_TRIGGER1;
    jumpHeld = (button & BUTTON_TRIGGER1);*/

    // アニメーション
    AnimationClip* nextAnim{ nullptr };
    if (velocity.x > 0.0f)
    {
        nextAnim = &animRight;
        facingLeft = false;
    }
    if (velocity.x < 0.0f)
    {
        nextAnim = &animLeft;
        facingLeft = true;
    }
    if (nextAnim && (currentAnim != nextAnim))
    {
        currentAnim = nextAnim;
        currentAnim->Reset();
    }

    if (currentAnim)
    {
        currentAnim->Update();
    }

  /*  int buttonDown = GetButtonDown(PLAYER1);
    bool swords = (buttonDown & BUTTON_TRIGGER2) != 0; 
    if (CanStartSword() && swords)
    {
        isSwordSwinging = true;
        swordTimer = Const::SWORD_DURATION;

        auto swordPtr = std::make_unique<Sword>(this);
        sword = swordPtr.get();         
        GC().SpawnEntity(std::move(swordPtr));

    }

    if (sword && isSwordSwinging) {
        float r = sword->GetRotation();
        DxPlus::Vec2 tip = position + DxPlus::Vec2(sinf(r), -cosf(r)) * 128.0f;

        GC().SpawnEntity(std::make_unique<Flame>(tip, 16.0f));
    }

    if (isSwordSwinging)
    {
        if (--swordTimer <= 0)
        {
            isSwordSwinging = false;
        }
    };*/
    int buttons = GetButton(PLAYER1);
    bool fire = (buttons & BUTTON_TRIGGER3) != 0;
    if (fire)
    {
        DxPlus::Vec2Int mousePosInt = DxPlus::Input::GetMousePosition();

        DxPlus::Vec2 emitterPos(static_cast<float>(mousePosInt.x),
            static_cast<float>(mousePosInt.y));

        GC().SpawnEntity(std::make_unique<Flame>(emitterPos, 60.0f));
    }
}



void Player::Step()
{
    // ジャンプ開始処理
    if (isGrounded && jumpPressed)
    {
        velocity.y = -Const::JUMP_SPEED;
        isGrounded = false;
    }

    // 重力を加えて落下速度を更新する
    velocity.y = std::min(velocity.y + Const::GRAVITY, Const::TERMINAL_VELOCITY);
    
    Entity2D::Step();

    const float groundY = DxPlus::CLIENT_HEIGHT * 0.9f;
    if (position.y > groundY)
    {
        position.y = groundY;
        velocity.y = 0.0f;
        isGrounded = true;
    }
    else
    {
        isGrounded = false;
    }
}
