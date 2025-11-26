// =============================
// Core/GameContext.cpp
// =============================
#include "GameContext.h"
#include "Entity2D.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Player.h"
#include "Enemy.h"
#include "Consts.h"
#include "projectedfslib.h"
#include "Collision2D.h"
#include <algorithm>
#include "Wall.h"
#include "map.h"
#include <cmath>

static int g_tileStatus[Map::GetRows()][Map::GetCols()];


void GameContext::Init()
{
    Walls.clear();

    // 修正: マップ全体を走査し、状態管理配列を初期化
    for (int row = 0; row < Map::GetRows(); row++)
    {
        for (int col = 0; col < Map::GetCols(); col++)
        {
            // g_tileStatus を Map::GetTileData の初期値で初期化
            int status = Map::GetTileData(row, col);
            g_tileStatus[row][col] = status;

            // status が '1' (壁) の場合のみ Wall オブジェクトを生成
            if (status == 1)
            {
                // タイル座標 (row, col) に基づいて、ワールド空間での中心座標を計算する
                DxPlus::Vec2 pos = Map::GetTileCenterPosition(row, col);

                // Walls (std::vector<std::unique_ptr<Wall>>) に新しい Wall オブジェクトを追加する
                Walls.emplace_back(
                    std::make_unique<Wall>(
                        pos, // Wallオブジェクトの中心座標
                        Map::GetTileWidth(), // Wallの幅 (タイルサイズ)
                        Map::GetTileHeight() // Wallの高さ (タイルサイズ)
                    )
                );
            }
        }
    }
    backgroundSpr = RM().GridAt(ResourceKeys::Background);

    entities.emplace_back(std::make_unique<Player>());
    player = static_cast<Player*>(entities.back().get());
    entities.push_back(std::make_unique<Enemy>());
    entities.push_back(std::make_unique<Enemy>());
    entities.push_back(std::make_unique<Enemy>());
    entities.push_back(std::make_unique<Enemy>());
    entities.push_back(std::make_unique<Enemy>());
    projectiles.clear();
    projectiles.reserve(8);
    for (auto& e : entities)
    {
        e->BindContext(this);
        e->Init();
    }
    player->Init();
    camera.SetTarget(player);
}

void GameContext::Reset()
{
    for (auto& e : entities) e->Reset();
    projectiles.clear();
}

// Core/GameContext.cpp (修正箇所)

// Core/GameContext.cpp (修正箇所)

void GameContext::Update()
{
    player->Update();
    camera.Update();
    for (auto& e : entities) e->Update();
    for (auto& e : entities) e->Step();

    DxPlus::Vec2 prev = player->GetPrevPosition();
    DxPlus::Vec2& p = player->Position();

    float pw = 120;
    float ph = 120;
    const float Y_COLLISION_OFFSET = 0.0f; // 描画とのズレがないか確認するために 0.0f でテスト

    float overlapX = 0;
    float overlapY = 0;

    for (auto& w : Walls)
    {
        DxPlus::Vec2 wp = w->position;
        float ww = w->width;
        float wh = w->height;

        // 壁の境界座標 (wTop, wBottom, wLeft, wRight)
        float wLeft = wp.x - ww / 2;
        float wRight = wp.x + ww / 2;
        float wTop = wp.y - wh / 2;
        float wBottom = wp.y + wh / 2;

        // ------------------------------------------------------------------
        // ★ 1. X軸の押し出し処理 (X軸の移動のみで衝突するかを prev.y で確認)
        // ------------------------------------------------------------------
        if (Collision2D::AABB({ p.x, prev.y }, pw, ph, wp, ww, wh))
        {
            // X軸境界を計算 (p.x は移動後の値)
            float pLeft = p.x - pw / 2;
            float pRight = p.x + pw / 2;

            if (p.x > prev.x) { // 右へ移動して衝突
                overlapX = pRight - wLeft;
                p.x -= overlapX;
            }
            else if (p.x < prev.x) { // 左へ移動して衝突
                overlapX = wRight - pLeft;
                p.x += overlapX;
            }
        }

        // ------------------------------------------------------------------
        // ★ 2. Y軸の押し出し処理 (Y軸の移動のみで衝突するかを、修正後の p.x で確認)
        // ------------------------------------------------------------------
        if (Collision2D::AABB({ p.x, p.y }, pw, ph, wp, ww, wh))
        {
            // Y軸境界を計算 (p.y は移動後の値)
            float pTop = p.y - ph / 2;
            float pBottom = p.y + ph / 2 + Y_COLLISION_OFFSET;

            if (p.y > prev.y) { // 下へ移動して衝突
                overlapY = pBottom - wTop;
                p.y -= overlapY;
            }
            else if (p.y < prev.y) { // 上へ移動して衝突
                overlapY = wBottom - pTop;
                p.y += overlapY;
            }
        }
    }
    // --- プレイヤーの足元のタイルの状態を更新する ---
    float tw = Map::GetTileWidth();
    float th = Map::GetTileHeight();
    DxPlus::Vec2 pos = player->Position();

    // ワールド座標からタイルインデックス (col, row) を計算
    // Map::GetTileCenterPositionの逆算に基づいて、左上隅の座標からタイルインデックスを求める
    int col = static_cast<int>(std::floor((pos.x - tw / 2) / tw));
    int row = static_cast<int>(std::floor((pos.y - th / 2) / th));

    // 境界チェックと状態更新
    if (row >= 0 && row < Map::GetRows() && col >= 0 && col < Map::GetCols())
    {
        // g_tileStatus を直接書き換える
        if (g_tileStatus[row][col] == 0) // 0: 空の床
        {
            g_tileStatus[row][col] = 2; // 2: 歩かれた (色を変える状態)
        }
    }
    for (auto& p : projectiles)
    {
        if (!p->IsAlive())continue;

        for (auto& e : entities)
        {
            if (!e->IsAlive() ||
                !e->IsDamageable())continue;

            if (Collision2D::CircleVsCircle(
                p->GetPosition(), p->Radius(),
                e->GetPosition() + e->GetCenterOffset(), e->Radius()))
            {
                e->OnHit(p->Attack());
                p->Kill();
                break;
            }
        }
        
    }
    //存在が消えたEntity2Dをentitiesから削除する
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [](const std::unique_ptr<Entity2D>& p)->bool
            {
                return !p->IsAlive();
            }
        ),
        entities.end()
    );
    //存在が消えたProjectileをprojectilesから削除する
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const std::unique_ptr<Projectile>& p)->bool
            {
                return !p->IsAlive();
            }
        ), projectiles.end()

    );

	std::wstring text = std::wstring(L"Player Position(") +
		std::to_wstring(static_cast<int>(player->GetPosition().x)) +
		L"," +
		std::to_wstring(static_cast<int>(player->GetPosition().y)) +
		L")";
	DxPlus::Debug::SetString(text);
}

void GameContext::Draw() const
{
    float camX = camera.GetX();
    float camY = camera.GetY();
    float tw = Map::GetTileWidth();
    float th = Map::GetTileHeight();
    // --- 床（空のタイル）の描画 ---
    for (int row = 0; row < Map::GetRows(); row++)
    {
        for (int col = 0; col < Map::GetCols(); col++)
        {
            int tileStatus = g_tileStatus[row][col]; // g_tileStatusを参照

            // 壁（1）以外のタイル（0:空、2:歩かれた）を描画する
            if (tileStatus != 1)
            {
                DxPlus::Vec2 centerPos = Map::GetTileCenterPosition(row, col);
                float left = centerPos.x - tw / 2 - camX;
                float top = centerPos.y - th / 2 - camY;

                unsigned int color;
                if (tileStatus == 2) {
                    // 歩かれたタイル: 灰色
                    color = GetColor(150, 150, 150);
                }
                else {
                    // 空のタイル: デフォルトの濃い色
                    color = GetColor(50, 50, 100);
                }

                DxPlus::Primitive2D::DrawRect(
                    { left, top },
                    { tw, th },
                    color
                );
            }
        }
    }
    for (auto& w : Walls)
    {
        float left = w->GetPos().x - w->GetWidth() / 2 - camX;
        float top = w->GetPos().y - w->GetHeight() / 2 - camY;

        DxPlus::Primitive2D::DrawRect(
            { left, top },
            { w->GetWidth(), w->GetHeight() },
            GetColor(0, 255, 0)
        );
    }
    for(auto&e:entities)
    {
        e->CameraDraw(camX, camY);
    }
}

void GameContext::SpawnProjectile(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel) noexcept
{
    if (projectiles.size() >= Const::MAX_PROJECTILES) { return; }
    auto pr = std::make_unique<Projectile>();
    pr->Init();
    pr->Reset(pos, vel);
    projectiles.push_back(std::move(pr));


}


