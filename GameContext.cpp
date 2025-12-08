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
#include "item.h"
#include <cmath>

static int g_tileStatus[Map::GetRows()][Map::GetCols()];

extern int PlayerWalkCount;
extern int PlayerWalkCountALL;

// ファイルスコープの保留バッファ：描画や反復処理中に直接 projectiles を変更しないために利用する
static std::vector<std::unique_ptr<Projectile>> g_pendingProjectiles;

void GameContext::Init()
{
    Walls.clear();

    // マップ全体を走査し、状態管理配列を初期化
    for (int row = 0; row < Map::GetRows(); ++row)
    {
        for (int col = 0; col < Map::GetCols(); ++col)
        {
            int status = Map::GetTileData(row, col);
            g_tileStatus[row][col] = status;
            if (status == 1)
            {
                DxPlus::Vec2 pos = Map::GetTileCenterPosition(row, col);
                Walls.emplace_back(std::make_unique<Wall>(pos, Map::GetTileWidth(), Map::GetTileHeight()));
            }
        }
    }

    backgroundSpr = RM().GridAt(ResourceKeys::Background);

    entities.clear();
    entities.emplace_back(std::make_unique<Player>());
    player = static_cast<Player*>(entities.back().get());

    for (size_t i = 0; i < 20; ++i)
    {
        entities.push_back(std::make_unique<Enemy>());
    }

    projectiles.clear();
    projectiles.reserve(8);

    for (auto& e : entities)
    {
        e->BindContext(this);
        e->Init();
    }
    if (player) player->Init();
    camera.SetTarget(player);

    Items.clear();
    for (int i = 0; i < 6; ++i)
    {
        auto item = std::make_unique<Item>();
        item->BindContext(this);
        item->Init();
        Items.push_back(std::move(item));
    }
}

void GameContext::Reset()
{
    for (auto& e : entities) e->Reset();
    projectiles.clear();
    g_pendingProjectiles.clear();
}

void GameContext::Update()
{
    if (!player) {
        DxPlus::Debug::SetString(L"GameContext::Update: player == nullptr");
        return;
    }

    // フレーム開始時の全エンティティの位置を保存（移動キャンセル用）
    std::vector<DxPlus::Vec2> prevPositions;
    prevPositions.reserve(entities.size());
    for (auto& e : entities) prevPositions.push_back(e ? e->GetPosition() : DxPlus::Vec2{});

    camera.Update();
    for (auto& e : entities) if (e) e->Update();
    for (auto& e : entities) if (e) e->Step();

    // 保留中発射物を projectiles に移す
    if (!g_pendingProjectiles.empty())
    {
        size_t maxP = static_cast<size_t>(Const::MAX_PROJECTILES);
        while (!g_pendingProjectiles.empty() && projectiles.size() < maxP)
        {
            projectiles.push_back(std::move(g_pendingProjectiles.back()));
            g_pendingProjectiles.pop_back();
        }
        if (!g_pendingProjectiles.empty()) g_pendingProjectiles.clear();
    }

    for (auto& pr : projectiles) if (pr) pr->Update();
    for (auto& pr : projectiles) if (pr) pr->Step();

    // プレイヤーと他エンティティの重なりチェック
    size_t playerIndex = SIZE_MAX;
    for (size_t i = 0; i < entities.size(); ++i) if (entities[i].get() == player) { playerIndex = i; break; }

    if (playerIndex != SIZE_MAX)
    {
        DxPlus::Vec2 playerCenter = player->GetPosition() + player->GetCenterOffset();
        float playerR = player->Radius();
        bool collided = false;
        for (size_t j = 0; j < entities.size(); ++j)
        {
            if (j == playerIndex) continue;
            auto& e = entities[j];
            if (!e || !e->IsAlive()) continue;
            DxPlus::Vec2 eCenter = e->GetPosition() + e->GetCenterOffset();
            if (Collision2D::CircleVsCircle(playerCenter, playerR, eCenter, e->Radius())) { collided = true; break; }
        }
        if (collided) player->SetPosition(prevPositions[playerIndex]);
    }

    // 壁との簡易衝突解決（既存ロジック維持）
    DxPlus::Vec2 prev = player->GetPrevPosition();
    DxPlus::Vec2& p = player->Position();
    float pw = 120.0f, ph = 120.0f;
    const float Y_COLLISION_OFFSET = 0.0f;
    for (auto& w : Walls)
    {
        DxPlus::Vec2 wp = w->GetPos();
        float ww = w->GetWidth();
        float wh = w->GetHeight();
        float wLeft = wp.x - ww / 2;
        float wRight = wp.x + ww / 2;
        float wTop = wp.y - wh / 2;
        float wBottom = wp.y + wh / 2;

        if (Collision2D::AABB({ p.x, prev.y }, pw, ph, wp, ww, wh))
        {
            float pLeft = p.x - pw / 2;
            float pRight = p.x + pw / 2;
            if (p.x > prev.x) { p.x -= (pRight - wLeft); }
            else if (p.x < prev.x) { p.x += (wRight - pLeft); }
        }
        if (Collision2D::AABB({ p.x, p.y }, pw, ph, wp, ww, wh))
        {
            float pTop = p.y - ph / 2;
            float pBottom = p.y + ph / 2 + Y_COLLISION_OFFSET;
            if (p.y > prev.y) { p.y -= (pBottom - wTop); }
            else if (p.y < prev.y) { p.y += (wBottom - pTop); }
        }
    }

    // プレイヤーの足元タイル更新
    float tw = Map::GetTileWidth();
    float th = Map::GetTileHeight();
    DxPlus::Vec2 pos = player->Position();
    int col = static_cast<int>(std::floor((pos.x - tw / 2.0f) / tw));
    int row = static_cast<int>(std::floor((pos.y - th / 2.0f) / th));
    if (row >= 0 && row < Map::GetRows() && col >= 0 && col < Map::GetCols())
    {
        if (g_tileStatus[row][col] == 0) g_tileStatus[row][col] = 2;
    }

    // 弾と敵の判定（簡単な O(n*m) 維持）
    for (auto& pr : projectiles)
    {
        if (!pr || !pr->IsAlive()) continue;
        for (auto& e : entities)
        {
            if (!e || !e->IsAlive() || !e->IsDamageable()) continue;
            if (Collision2D::CircleVsCircle(pr->GetPosition(), pr->Radius(), e->GetPosition() + e->GetCenterOffset(), e->Radius()))
            {
                e->OnHit(pr->Attack());
                pr->RegisterHit(); // 直接 Kill() せず、Projectile 側でヒット数を管理する
                break;
            }
        }
    }

    // 削除処理
    entities.erase(std::remove_if(entities.begin(), entities.end(), [](const std::unique_ptr<Entity2D>& p){ return !p->IsAlive(); }), entities.end());
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const std::unique_ptr<Projectile>& p){ return !p->IsAlive(); }), projectiles.end());

    // アイテム回収判定
    for (auto& item : Items)
    {
        float dx = player->Position().x - item->Position().x;
        float dy = player->Position().y - item->Position().y;
        float dist2 = dx*dx + dy*dy;
        float r = player->Radius() + item->Radius();
        if (dist2 <= r*r) item->Reset();
    }

    // 入力（魅力度増減）
    using namespace DxPlus::Input;
    int button = GetButtonDown(PLAYER1);
    bool up = (button & BUTTON_TRIGGER3) != 0;
    bool down = (button & BUTTON_TRIGGER4) != 0;
    if (up) ++Attractiveness;
    if (down) { --Attractiveness; if (Attractiveness < 0) Attractiveness = 0; }

        std::wstring text = L"Player Position(" 
            + std::to_wstring(static_cast<int>(player->GetPosition().x))
            + L"," + std::to_wstring(static_cast<int>(player->GetPosition().y))
            + L")  残りターン=" + std::to_wstring(static_cast<int>(Const::TURN_MAX - PlayerWalkCountALL))
            + L")  ショットカウント=" + std::to_wstring(static_cast<int>(PlayerWalkCount))
            + L"\n魅力度下げるShift上げるAlt" 
            + std::to_wstring(static_cast<int>(Attractiveness));
        DxPlus::Debug::SetString(text);

}

namespace {
    constexpr float PI_F = 3.14159265358979323846f;
    inline float DegToRad(float deg) noexcept { return deg * (PI_F / 180.0f); }
}

void GameContext::Draw() const
{
    if (!player) return;

    float camX = camera.GetX();
    float camY = camera.GetY();
    float tw = Map::GetTileWidth();
    float th = Map::GetTileHeight();
    const DxPlus::Sprite::SpriteBase* tileSprite = RM().GridAt(ResourceKeys::Tiles_City);

    // 画面サイズ（DxPlus 定義を使用）
    int screenW = DxPlus::CLIENT_WIDTH;
    int screenH = DxPlus::CLIENT_HEIGHT;

    // 視界のワールド座標（余裕1タイル分）
    float viewLeft = camX - tw;
    float viewTop = camY - th;
    float viewRight = camX + static_cast<float>(screenW) + tw;
    float viewBottom = camY + static_cast<float>(screenH) + th;

    int minCol = static_cast<int>(std::floor(viewLeft / tw));
    int maxCol = static_cast<int>(std::floor(viewRight / tw));
    int minRow = static_cast<int>(std::floor(viewTop / th));
    int maxRow = static_cast<int>(std::floor(viewBottom / th));

    minCol = std::max(0, minCol);
    minRow = std::max(0, minRow);
    maxCol = std::min(Map::GetCols() - 1, maxCol);
    maxRow = std::min(Map::GetRows() - 1, maxRow);

    // タイル描画（カリング済み範囲のみ）
    for (int row = minRow; row <= maxRow; ++row)
    {
        for (int col = minCol; col <= maxCol; ++col)
        {
            int tileStatus = g_tileStatus[row][col];
            if (tileStatus == 1) continue; // 壁はタイル描画しない

            DxPlus::Vec2 centerPos = Map::GetTileCenterPosition(row, col);
            float left = centerPos.x - tw/2 - camX;
            float top = centerPos.y - th/2 - camY;

            if (tileStatus == 2)
            {
                DrawBox(static_cast<int>(left), static_cast<int>(top), static_cast<int>(left+tw), static_cast<int>(top+th), GetColor(150,150,150), TRUE);
            }
            if (tileStatus == 0 || tileStatus == 2)
            {
                float leftf = std::floor(centerPos.x - tw/2 - camX);
                float topf = std::floor(centerPos.y - th/2 - camY);
                tileSprite->Draw(DxPlus::Vec2(leftf, topf));
            }
        }
    }

    // アイテム・壁・エンティティ・弾の描画
    for (auto& item : Items) item->CameraDraw(camX, camY);
    for (auto& w : Walls)
    {
        float left = w->GetPos().x - w->GetWidth()/2 - camX;
        float top = w->GetPos().y - w->GetHeight()/2 - camY;
        DxPlus::Primitive2D::DrawRect({left, top}, {w->GetWidth(), w->GetHeight()}, GetColor(0,255,0));
    }
    for (auto& e : entities)
    {


        e->CameraDraw(camX, camY);
    }

    for (auto& pr : projectiles) if (pr && pr->IsAlive()) pr->CameraDraw(camX, camY);

    // ミニマップ（小さいため全タイル走査でOK）
    DxPlus::Primitive2D::DrawRect({990,0}, {290,720}, GetColor(0,0,0));
    const float miniScale = 0.05f;
    const float miniX = 1050.0f, miniY = 0.0f;
    for (int row = 0; row < Map::GetRows(); ++row)
    {
        for (int col = 0; col < Map::GetCols(); ++col)
        {
            if (g_tileStatus[row][col] != 2) continue;
            DxPlus::Vec2 c = Map::GetTileCenterPosition(row, col);
            float MinX = c.x * miniScale + miniX;
            float MinY = c.y * miniScale + miniY;
            float tw2 = tw * miniScale;
            float th2 = th * miniScale;
            DrawBox(static_cast<int>(MinX - tw2/2), static_cast<int>(MinY - th2/2), static_cast<int>(MinX + tw2/2), static_cast<int>(MinY + th2/2), GetColor(150,150,150), TRUE);
        }
    }

    // HUD
    DxPlus::Vec2 hudCenter{100.0f, 620.0f};
    float hudRadius = 100.0f;
    unsigned int cols = DxLib::GetColor(50,50,50);

    static int kuroomuHandle = -1;
    if (kuroomuHandle == -1) kuroomuHandle = DxPlus::Sprite::Load(L"./Data/Images/クローム.png");
    if (kuroomuHandle != -1) DxPlus::Sprite::Draw(kuroomuHandle, {hudCenter.x-100, hudCenter.y-100}, {0.16f, 0.16f});

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    const_cast<GameContext*>(this)->SetCircleSweepDeg(static_cast<float>(360 - PlayerWalkCount * 360 / Const::MAX_PLAYER_WALK_COUNT));
    // 分割数を抑えて頂点生成コストを下げる
    DrawFilledSectorScreen(hudCenter, hudRadius, {1.0f,1.0f}, 0.0f, 64, cols);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

}

bool GameContext::SpawnProjectile(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel) noexcept
{
    size_t total = projectiles.size() + g_pendingProjectiles.size();
    if (total >= static_cast<size_t>(Const::MAX_PROJECTILES)) return false;
    auto pr = std::make_unique<Projectile>();
    pr->BindContext(this);
    pr->Init();
    pr->Reset(pos, vel, pr->GetSize());
    g_pendingProjectiles.push_back(std::move(pr));
    return true;
}

bool GameContext::IsPositionFree(const DxPlus::Vec2& pos, float radius, const Entity2D* ignore) const noexcept
{
    for (const auto& e : entities)
    {
        if (!e || !e->IsAlive() || e.get() == ignore) continue;
        if (Collision2D::CircleVsCircle(pos, radius, e->GetPosition() + e->GetCenterOffset(), e->Radius())) return false;
    }
    for (const auto& w : Walls)
    {
        if (!w) continue;
        DxPlus::Vec2 wp = w->GetPos();
        float halfW = w->GetWidth() * 0.5f;
        float halfH = w->GetHeight() * 0.5f;
        float closestX = std::max(wp.x - halfW, std::min(pos.x, wp.x + halfW));
        float closestY = std::max(wp.y - halfH, std::min(pos.y, wp.y + halfH));
        float dx = pos.x - closestX;
        float dy = pos.y - closestY;
        if (dx*dx + dy*dy <= radius*radius) return false;
    }
    return true;
}

void GameContext::DrawFilledSectorScreen(
    const DxPlus::Vec2& screenPos,
    float radius,
    const DxPlus::Vec2& scale,
    float rotationRad,
    int numSegments,
    int color) const
{
    DxPlus::Vec2 center = screenPos;
    int segs = std::max(1, numSegments > 0 ? numSegments : circleSegments);
    std::vector<DxPlus::Vec2> verts;
    verts.reserve(static_cast<size_t>(segs) + 2);
    verts.push_back(center);
    float startRad = DegToRad(circleAngleDeg);
    float sweepRad = DegToRad(circleSweepDeg);
    float cosR = std::cos(rotationRad);
    float sinR = std::sin(rotationRad);
    for (int i = 0; i <= segs; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(segs);
        float ang = startRad - t * sweepRad;
        float x = std::cos(ang) * radius * scale.x;
        float y = std::sin(ang) * radius * scale.y;
        float rx = x * cosR - y * sinR;
        float ry = x * sinR + y * cosR;
        verts.push_back({ center.x + rx, center.y + ry });
    }
    DxPlus::Primitive2D::DrawPolygon(verts, color, true);
}
