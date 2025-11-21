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
// ゲーム全体で共有する"文脈（コンテキスト）"。
// プレイヤー・敵・マップ・アイテムなどを束ね、
// シーン（Title / Game / Result）から参照できるようにする。
class Player;
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
    void SpawnProjectile(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel)noexcept;
private:
    const DxPlus::Sprite::SpriteBase* backgroundSpr{ nullptr };
    std::vector<std::unique_ptr<Entity2D>> entities;
    Player* player=nullptr;
    Player* GetPlayer() const { return player; }
    // ゲームの描画視点を制御するカメラ
    Camera camera;
    std::vector<std::unique_ptr<Projectile>>projectiles;
    std::vector<std::unique_ptr<Wall>> Walls;

};
