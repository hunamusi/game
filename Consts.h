// =============================
// Core/Consts.h
// =============================
#pragma once

namespace Const
{
    // ===== プレイヤー関連 =====
    constexpr float PLAYER_SPEED = 120.0f;
    constexpr float MAX_PROJECTILES = 12;
   constexpr DxPlus::Vec2 PLAYER_CENTER_OFFSET = { 0.0f,-48.0f };
   constexpr float PROJECTILE_SPEED = 12.0f;
   constexpr int MAX_ATTRACTIVIENESS = 20;
   constexpr int MAX_ATTACKTIMER = 2;
                    
   //=====　ショット関連 =====
   constexpr int MAX_PLAYER_WALK_COUNT = 3;


    // ===== 敵関連 =====
    constexpr float ENEMY_SPEED = 4.0f;
    constexpr float MOVE_DISTANCE = 100.0f;
    constexpr float ENEMY_MAX = 10;

    // ===== ゲーム内共通 =====

    constexpr int TURN_MAX = 100;

    // =====TileMap Layer =====
    constexpr int LAYER_BACKGROUND = 0;
    constexpr int LAYER_TERRAIN = 1;
}
