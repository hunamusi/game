// =============================
// Core/GameContext.h
// =============================
#pragma once
#include "DxPlus/DxPlus.h"
#include <memory>
#include <vector>
#include "Entity2D.h"
#include "Projectile.h"
#include "Camera.h"
#include "Wall.h"
#include <cmath>
#include "Item.h"
#include "Player.h"

// ゲーム全体で共有する"文脈（コンテキスト）"。
// プレイヤー・敵・マップ・アイテムなどを束ね、
// シーン（Title / Game / Result）から参照できるようにする。
class GameContext
{
public:
    GameContext() = default;
    ~GameContext() = default;
    // 一度だけ呼ぶ初期化（リソースロードは ResourceManager 側で行う）
    void Init();

    // ステージ再開・リトライ用の軽量リセット
    void Reset();

    // プレイの更新と描画（必要なシーンで呼ぶ）
    void Update();
    void Draw() const;
    bool SpawnProjectile(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel) noexcept;
    bool IsPositionFree(const DxPlus::Vec2& pos, float radius, const Entity2D* ignore) const noexcept;
    Player* GetPlayer() const noexcept { return player; }

    // --- 追加 public API ---
    void SetCircleAngleDeg(float deg) noexcept { circleAngleDeg = deg; }
    float GetCircleAngleDeg() const noexcept { return circleAngleDeg; }
    void SetCircleSweepDeg(float deg) noexcept { circleSweepDeg = deg; }
    float GetCircleSweepDeg() const noexcept { return circleSweepDeg; }
    void SetCircleSegments(int seg) noexcept { circleSegments = seg > 0 ? seg : 1; }
    int GetCircleSegments() const noexcept { return circleSegments; }



    // --- スクリーン座標で扇形を描画（カメラを考慮しない） ---
    void DrawFilledSectorScreen(
        const DxPlus::Vec2& screenPos,
        float radius,
        const DxPlus::Vec2& scale = {1.0f, 1.0f},
        float rotationRad = 0.0f,
        int numSegments = 128,
        int color = DxLib::GetColor(255, 255, 255)) const;

    int GetAttractiveness() const noexcept { return Attractiveness; }


private:
    const DxPlus::Sprite::SpriteBase* backgroundSpr{ nullptr };
    std::vector<std::unique_ptr<Entity2D>> entities;
    Player* player{ nullptr };
    // ゲームの描画視点を制御するカメラ
    Camera camera;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<std::unique_ptr<Wall>> Walls;
    std::vector<std::unique_ptr<Item>> Items;

    float circleAngleDeg{ 270.0f };    // 開始角度（度）
    float circleSweepDeg{ 360.0f };   // 掃引角（度）
    int circleSegments{ 1000 };        // 円弧を近似する分割数
    int Attractiveness = 0; //魅力度
};
