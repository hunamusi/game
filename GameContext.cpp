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
const float TILE_W = 120.0f;
const float TILE_H = 120.0f;

void GameContext::Init()
{
    Walls.clear();
        // マップ全体を走査し、壁タイルを生成する
    for (int row = 0; row < Map::GetRows(); row++) // マップの行 (row) を端から端までループ (0 から GetRows()-1 まで)
    {
        for (int col = 0; col < Map::GetCols(); col++) // マップの列 (col) を端から端までループ (0 から GetCols()-1 まで)
        {
            // マップデータ (Map::GetTileData) が '1' (壁) であるかチェック
            if (Map::GetTileData(row, col) == 1)
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

    float pw = 30;
    float ph = 108;
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

   // backgroundSpr->Draw({ -camX, -camY });
    for (int i = 0;++i <= 30;) {
        DxPlus::Primitive2D::DrawRect({ 0+i*120 - camX,400- camY }, { 120,120 },GetColor(0+i*10,0+i*10,0+i*10));
    }
    for(auto&e:entities)
    {
        e->CameraDraw(camX, camY);
    }

    std::wstring Counttext = L"Player Count" + std::to_wstring(player->GetPlayerCount());
    DrawString(1000, 0, Counttext.c_str(), GetColor(255, 255, 255));
}

void GameContext::SpawnProjectile(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel) noexcept
{
    if (projectiles.size() >= Const::MAX_PROJECTILES) { return; }
    auto pr = std::make_unique<Projectile>();
    pr->Init();
    pr->Reset(pos, vel);
    projectiles.push_back(std::move(pr));


}


