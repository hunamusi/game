// =============================
// Gameplay/Actors/Player.cpp
// =============================
// 概要: プレイヤー操作主体のアクター実装。
// 責務:
//  - 入力取得（DxPlus::Input）と移動ベクトルの更新
//  - アニメーションの遷移・更新（移動時のみ進行）
//  - ショット発射と、発射トリガ時の近接攻撃委譲（GameContext::AttackFront）
//  - 歩数管理（PlayerWalkCount/ALL）と HUD 表示用カウンタ更新
// 設計メモ:
//  - 実座標系はワールド座標、描画はカメラ相対（CameraDraw）
//  - プロジェクタイル発射は GC().SpawnProjectile の戻り値で成否を判定
//  - スレッドセーフ想定なし（メインスレッドでの更新・描画前提）
//  - 速度や当たり判定寸法は Consts 由来、必要に応じて調整
// 注意:
//  - グローバル歩数は他システムでも参照されるため、無闇にリセットしないこと
//  - currentAnim の切替時は Reset() を忘れず、視覚的破綻を防止
#include "Player.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "AnimationUtil.h"
#include "GameContext.h"

int PlayerWalkCount = 0;
int PlayerWalkCountALL = 0;

void Player::Init()
{
    sprite = RM().GridAt(ResourceKeys::Player, 1, 2);

    AnimationUtil::BuildWalk(animLeft,  2, RM(), ResourceKeys::Player, 8);
    AnimationUtil::BuildWalk(animRight, 3, RM(), ResourceKeys::Player, 8);
    AnimationUtil::BuildWalk(animUp,    1, RM(), ResourceKeys::Player, 8);
    AnimationUtil::BuildWalk(animDown,  0, RM(), ResourceKeys::Player, 8);

    currentAnim = &animDown;
}

void Player::Reset()
{
    position = {540 , 420 };
    velocity = {};

    if (currentAnim) currentAnim->Reset();
    isAttacking = false;
    attackTicks = 0;
}

void Player::Update()
{
    prevPos = position;
    using namespace DxPlus::Input;

    int buttonDown = GetButtonDown(PLAYER1);
    int button = GetButton(PLAYER1);
    bool left = (buttonDown & BUTTON_LEFT) != 0;
    bool right = (buttonDown & BUTTON_RIGHT) != 0;
    bool up = (buttonDown & BUTTON_UP) != 0;
    bool down = (buttonDown & BUTTON_DOWN) != 0;
    bool shift = (button & BUTTON_TRIGGER4) != 0;

    AnimationClip* nextAnim{ nullptr };
    bool isMoving{ false };
    float vx{}, vy{};


    if (PlayerCount >= 0 && shift)
    {
        if (left && !right)
        {
            nextAnim = &animLeft;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCountALL++;


        }
        else if (!left && right)
        {
            nextAnim = &animRight;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCountALL++;
        }
        if (up && !down)
        {
            nextAnim = &animUp;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCountALL++;
        }
        else if (!up && down)
        {
            nextAnim = &animDown;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCountALL++;
        }
        PlayerWalkCount = std::min(PlayerWalkCount, Const::MAX_PLAYER_WALK_COUNT);
    }
    else if (PlayerCount >= 0)
    {
        if (left && !right)
        {

            vx = -Const::PLAYER_SPEED;
            nextAnim = &animLeft;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCountALL++;


        }
        else if (!left && right)
        {

            vx = Const::PLAYER_SPEED;
            nextAnim = &animRight;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCountALL++;
        }
        if (up && !down)
        {
            vy = -Const::PLAYER_SPEED;
            nextAnim = &animUp;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCountALL++;
        }
        else if (!up && down)
        {
            vy = Const::PLAYER_SPEED;
            nextAnim = &animDown;
            isMoving = true;

            PlayerWalkCount++;
            PlayerWalkCountALL++;
        }
        PlayerWalkCount = std::min(PlayerWalkCount, Const::MAX_PLAYER_WALK_COUNT);
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
    int buttonDowns = GetButtonDown(PLAYER1);
    bool shoot = (buttonDowns & BUTTON_TRIGGER2) != 0;

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
            // 追加: 発射に成功したタイミングで正面1マス攻撃を行う
            DxPlus::Vec2 faceDir;
            const float EPS2 = 0.0001f;
            if (std::fabs(prevVelocity.x) > EPS2 || std::fabs(prevVelocity.y) > EPS2)
            {
                faceDir = prevVelocity.Normalize();
            }
            else
            {
                if (currentAnim == &animLeft) faceDir = DxPlus::Vec2{ -1.0f, 0.0f };
                else if (currentAnim == &animRight) faceDir = DxPlus::Vec2{ 1.0f, 0.0f };
                else if (currentAnim == &animUp) faceDir = DxPlus::Vec2{ 0.0f, -1.0f };
                else faceDir = DxPlus::Vec2{ 0.0f, 1.0f };
            }
            // GameContext に正面攻撃を委譲
            GC().AttackFront(this, faceDir);


            DxPlus::Vec2 backPos = position - faceDir.Normalize();
            backPos.y -= 10.0f;
            GC().SpawnEnemyHitEffect(backPos);

            // 攻撃表示を開始
            isAttacking = true;
            attackTicks = ATTACK_DRAW_TICKS;

            PlayerWalkCount = 0;
        }
    }

    // 攻撃表示カウントダウン
    if (isAttacking)
    {
        --attackTicks;
        if (attackTicks <= 0) { isAttacking = false; attackTicks = 0; }
    }

}

void Player::Equip(EquipType newEquip)
{
    // 同じ装備なら何もしない
    if (currentEquip == newEquip)
        return;

    // 今の装備を外す
    switch (currentEquip)
    {
    case EquipType::Ishab:
        attack -= 10;
        break;

    case EquipType::Sword:
        attack -= 5;
        break;

    case EquipType::Shield:
        defense -= 5;
        break;

    default:
        break;
    }

    // 新しい装備を付ける
    switch (newEquip)
    {
    case EquipType::Ishab:
        attack += 10;
        break;

    case EquipType::Sword:
        attack += 5;
        break;

    case EquipType::Shield:
        defense += 5;
        break;

    default:
        break;
    }

    // 現在装備を更新
    currentEquip = newEquip;
}

void Player::CameraDraw(float camX, float camY)
{
    DxPlus::Vec2 camPos = position - DxPlus::Vec2{ camX, camY };

    if (isAttacking)
    {
        // 攻撃時は Player の (3,0) フレームを直接取得して描画
        const DxPlus::Sprite::SpriteBase* atkFrame = RM().GridAt(ResourceKeys::Player, 3, 0);
        if (atkFrame) atkFrame->Draw(camPos,{0.9f,0.9f});
        else if (currentAnim) currentAnim->Draw(camPos,{0.9f,0.9f});
    }
    else
    {
        // 通常描画は基底の CameraDraw を利用
        if (currentAnim) currentAnim->Draw(camPos, { 0.9f, 0.9f });
        else if (sprite) sprite->Draw(camPos, { 0.9f, 0.9f });
    }
}
