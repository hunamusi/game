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

// ファイルスコープの保留バッファ：描画や反復処理中に直接 projectiles を変更しないために利用する
static std::vector<std::unique_ptr<Projectile>> g_pendingProjectiles;

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

    for (size_t i = 0; i < 10; i++)
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
    player->Init();
    camera.SetTarget(player);
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
    // player が nullptr のときは早期リターンしてクラッシュを防ぐ
    if (!player) {
        DxPlus::Debug::SetString(L"GameContext::Update: player == nullptr");
        return;
    }

    // フレーム開始時の全エンティティの位置を保存（移動キャンセル用）
    std::vector<DxPlus::Vec2> prevPositions;
    prevPositions.reserve(entities.size());
    for (auto& e : entities)
    {
        prevPositions.push_back(e ? e->GetPosition() : DxPlus::Vec2{});
    }

    camera.Update();
    for (auto& e : entities) e->Update();
    for (auto& e : entities) e->Step();

    // --- 保留中の発射物を安全なタイミングで projectiles に移す ---
    if (!g_pendingProjectiles.empty())
    {
        // 最大数を超えないように移動する
        size_t maxP = static_cast<size_t>(Const::MAX_PROJECTILES);
        while (!g_pendingProjectiles.empty() && projectiles.size() < maxP)
        {
            // back() を使って pop_back で移動（順序を気にしない場合これで十分）
            projectiles.push_back(std::move(g_pendingProjectiles.back()));
            g_pendingProjectiles.pop_back();
        }
        // もし保留が残っているなら破棄（Spawn側で既にチェックしているはずだが念のため）
        if (!g_pendingProjectiles.empty())
        {
            g_pendingProjectiles.clear();
        }
    }

    for (auto& p : projectiles) if (p) p->Update();
    for (auto& p : projectiles) if (p) p->Step();

    // プレイヤーが他エンティティと重なったらプレイヤーの移動を取り消す
    size_t playerIndex = SIZE_MAX;
    for (size_t i = 0; i < entities.size(); ++i)
    {
        if (entities[i].get() == player) { playerIndex = i; break; }
    }

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
            if (Collision2D::CircleVsCircle(playerCenter, playerR, eCenter, e->Radius()))
            {
                collided = true;
                break;
            }
        }

        if (collided)
        {
            // 移動をキャンセルして元の位置に戻す
            player->SetPosition(prevPositions[playerIndex]);
        }
    }

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

    for (auto& p : projectiles) {
        if (!p->IsAlive()) continue;
        // 存在してない弾は、とばす 
        for (auto& e : entities)
        {
            if (!e->IsAlive() || // 存在していない敵は、とばす 
                !e->IsDamageable())
                continue; // ダメージを受けないエンティティは、とばす 
            if (Collision2D::CircleVsCircle(
                // 円と円のあたり判定を行う 
                p->GetPosition(), p->Radius(),
                // 位置と半径を入れる 
                e->GetPosition() + e->GetCenterOffset(),
                e->Radius()))
            {
                e->OnHit(p->Attack());
                // 敵の OnHit 関数に弾の攻撃力を渡す 
                p->Kill();
                // 弾は 1 回あたったら消す 
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
    for (auto& item : Items)
    {
        float dx = player->Position().x - item->Position().x;
        float dy = player->Position().y - item->Position().y;

        float dist2 = dx * dx + dy * dy;
        float r = player->Radius() + item->Radius();

        if (dist2 <= r * r)
        {
            item->Reset();
        }
    }

    using namespace DxPlus::Input; 
    int button = GetButtonDown(PLAYER1);
    bool up = (button & BUTTON_TRIGGER3) != 0;
    bool down = (button & BUTTON_TRIGGER4) != 0;

    if (up)
    {
        Attractiveness++;
    }
    if (down)
    {
        Attractiveness--;
        if (Attractiveness < 0)
        {
            Attractiveness = 0;
        }
    }


    std::wstring text =
        std::wstring(L"Player Position(") +
        std::to_wstring(static_cast<int>(player->GetPosition().x)) +
        L"," +
        std::to_wstring(static_cast<int>(player->GetPosition().y)) +
        L")  WalkCount=" +
        std::to_wstring(static_cast<int>(PlayerWalkCount)) +
        L"\n魅力度上げるShift下げるAlt" +
        std::to_wstring(static_cast<int>(Attractiveness));
    DxPlus::Debug::SetString(text);
}

namespace {
    constexpr float PI_F = 3.14159265358979323846f;
    inline float DegToRad(float deg) noexcept { return deg * (PI_F / 180.0f); }
}

void GameContext::Draw() const
{
    // player が nullptr のときは早期リターンしてクラッシュを防ぐ
    if (!player) return;

    float camX = camera.GetX();
    float camY = camera.GetY();
    float tw = Map::GetTileWidth();
    float th = Map::GetTileHeight();
    const DxPlus::Sprite::SpriteBase* tileSprite = RM().GridAt(ResourceKeys::Tiles_City);
    const DxPlus::Sprite::SpriteBase* BuildingSprite = RM().GridAt(ResourceKeys::Building_big);
    const DxPlus::Sprite::SpriteBase* BuildingSprite_small = RM().GridAt(ResourceKeys::Building_small);
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
                    DrawBox(
                        (int)left,
                        (int)top,
                        (int)(left + tw),
                        (int)(top + th),
                        GetColor(150, 150, 150),
                        TRUE);
                }
                if (tileStatus == 0 || tileStatus == 2) {
                    // 空のタイル: デフォルトの濃い色
                    float left = std::floor(centerPos.x - tw / 2 - camX);
                    float top = std::floor(centerPos.y - th / 2 - camY);
                    tileSprite->Draw(DxPlus::Vec2(left, top));
                }
            }
        }
    }
    for (auto& item : Items)
    {
        item->CameraDraw(camX, camY);
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
    for (auto& e : entities)
    {
        e->CameraDraw(camX, camY);
    }

    for (auto& pr : projectiles)
    {
        if (pr && pr->IsAlive())
        {
            pr->CameraDraw(camX, camY);
        }
    }

    DxPlus::Primitive2D::DrawRect({ 990,0 }, { 290,720 }, GetColor(0, 0, 0));
    for (int row = 0; row < Map::GetRows(); row++)
    {
        for (int col = 0; col < Map::GetCols(); col++)
        {
            int tileStatus = g_tileStatus[row][col]; // g_tileStatusを参照
            DxPlus::Vec2 centerPos = Map::GetTileCenterPosition(row, col);
            float scale = 0.05;
            float x = 1050;
            float y = 0;
            float MinX = centerPos.x * scale + x;
            float MinY = centerPos.y * scale + y;
            float tw2 = tw * scale;
            float th2 = th * scale;
            float left = MinX - tw2 / 2;
            float top = MinY - th2 / 2;

            unsigned int color;
            if (tileStatus == 2) {
                // 歩かれたタイル: 灰色
                color = GetColor(150, 150, 150);
                DrawBox(
                    (int)left,
                    (int)top,
                    (int)(left + tw2),
                    (int)(top + th2),
                    GetColor(150, 150, 150),
                    TRUE);
            }
        }
    }

    DxPlus::Vec2 hudCenter{ 100.0f, 620.0f };
    float hudRadius = 100.0f;
    unsigned int col = DxLib::GetColor(255, 255, 255);
    unsigned int cols = DxLib::GetColor(50, 50, 50);


    int kuroomu = DxPlus::Sprite::Load(L"./Data/Images/クローム.png");
    DxPlus::Sprite::Draw(kuroomu, { hudCenter.x - 100,hudCenter.y - 100 }, { 0.16,0.16 });
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    // HUD の円と同じ位置・大きさで扇形を塗りつぶす（スクリーン座標を直接使う）
    const_cast<GameContext*>(this)->SetCircleSweepDeg(static_cast<float>(360 - PlayerWalkCount * 360 / Const::MAX_PLAYER_WALK_COUNT));
    DrawFilledSectorScreen(hudCenter, hudRadius, { 1.0f,1.0f }, 0.0f, 256, cols);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

bool GameContext::SpawnProjectile(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel) noexcept
{
    // 発射中 + 保留中の合計が上限を越えないようにする
    size_t total = projectiles.size() + g_pendingProjectiles.size();
    if (total >= static_cast<size_t>(Const::MAX_PROJECTILES)) {
        return false;
    }
    auto pr = std::make_unique<Projectile>();
    pr->BindContext(this);
    pr->Init();
    pr->Reset(pos, vel, pr->GetSize());

    // 直接 projectiles に追加せず、保留バッファへ格納する（Draw や他の反復中の破壊を防ぐ）
    g_pendingProjectiles.push_back(std::move(pr));
    return true;
}
bool GameContext::IsPositionFree(const DxPlus::Vec2& pos, float radius, const Entity2D* ignore) const noexcept
{
    // エンティティ同士の衝突チェック（既存）
    for (const auto& e : entities)
    {
        if (!e) continue;
        if (!e->IsAlive()) continue;
        if (e.get() == ignore) continue;

        if (Collision2D::CircleVsCircle(pos, radius, e->GetPosition() + e->GetCenterOffset(), e->Radius()))
        {
            return false;
        }
    }

    // 壁（タイル由来）の衝突チェックを追加
    for (const auto& w : Walls)
    {
        if (!w) continue;

        // 壁は矩形（中心座標 wp と幅/高さ）として扱う
        DxPlus::Vec2 wp = w->GetPos();
        float halfW = w->GetWidth() * 0.5f;
        float halfH = w->GetHeight() * 0.5f;

        // 円の中心 pos に対して矩形に最も近い点を求める
        float closestX = std::max(wp.x - halfW, std::min(pos.x, wp.x + halfW));
        float closestY = std::max(wp.y - halfH, std::min(pos.y, wp.y + halfH));

        float dx = pos.x - closestX;
        float dy = pos.y - closestY;

        if (dx * dx + dy * dy <= radius * radius)
        {
            return false;
        }
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
    // 画面座標をそのまま使う（カメラオフセットを適用しない）
    DxPlus::Vec2 center = screenPos;

    int segs = std::max(1, numSegments > 0 ? numSegments : circleSegments);
    std::vector<DxPlus::Vec2> verts;
    verts.reserve(static_cast<size_t>(segs) + 2);
    verts.push_back(center); // triangle fan の中心

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
