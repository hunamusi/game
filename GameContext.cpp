// =============================
// Core/GameContext.cpp
// =============================
// 概要: ゲーム全体の状態（文脈）をまとめて管理する中核モジュール。
// 責務:
//  - リソース初期化とワールド構築（壁/タイル/エンティティ/エフェクト/カメラ）
//  - 毎フレームの更新・描画のオーケストレーション
//  - 衝突/重なりの簡易判定、弾プールの遅延挿入、ヒットエフェクトの再生管理
// 設計メモ:
//  - `Entity2D::BindContext(this)` により全エンティティへコンテキストを注入する（必須）。
//  - 弾は即時 push ではなく保留バッファへ積み、イテレーション中のコンテナ破壊を回避。
//  - マップはタイル中心座標を基準に配置、壁は AABB、オブジェクトは円で簡易判定。
//  - スレッドセーフ想定なし（メインスレッドでの更新/描画）。
// 注意:
//  - 動的スポーン時は新規オブジェクトのみに Init/Reset/Bind を行うこと（既存エンティティの Reset 連鎖は位置リセットの原因）。
//  - `player` が必須の箇所では早期 return で防衛（null 安全）。
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


int a = 0;
int j = 0;
bool isStopping = false;     // 止めているかどうか
int stopTimer = 0;
float alpha = 0;
int b = 0;
int c = 0;
int Timer = 0;
int d = 0;

static int flashAlpha = 0;
static bool fadeIn = true;

static TreasureType g_treasureType[Map::GetRows()][Map::GetCols()];
static int g_tileStatus[Map::GetRows()][Map::GetCols()];

extern int PlayerWalkCount;
extern int PlayerWalkCountALL;
static DxPlus::Vec2 g_itemPopupPos{};

// ファイルスコープの保留バッファ：描画や反復処理中に直接 projectiles を変更しないために利用する
static std::vector<std::unique_ptr<Projectile>> g_pendingProjectiles;

void GameContext::Init()
{
    // 壁データをマップから読み取り、物理用 Wall を生成
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

    // エンティティ初期化（プレイヤー + 必要なら敵プール）
    entities.clear();
    entities.emplace_back(std::make_unique<Player>());
    player = static_cast<Player*>(entities.back().get());

    for (size_t i = 0; i < 0; ++i)
    {
        entities.push_back(std::make_unique<Enemy>());
    }

    projectiles.clear();
    projectiles.reserve(8);

    // 重要: コンテキストを全エンティティへ注入し、初期化を一度だけ行う
    for (auto& e : entities)
    {
        e->BindContext(this);
        e->Init();
    }
    if (player) player->Init();
    camera.SetTarget(player);

    // アイテム生成
    Items.clear();
    for (int i = 0; i < 6; ++i)
    {
        auto item = std::make_unique<Item>();
        item->BindContext(this);
        item->Init();
        Items.push_back(std::move(item));
    }

    // AttackEffectEnemy のプール初期化（軽量な一時オブジェクト）
    effects.clear();
    effects.reserve(16);
    for (int i = 0; i < 8; ++i)
    {
        auto ef = std::make_unique<AttackEffectEnemy>();
        ef->BindContext(this);
        ef->Init();
        effects.push_back(std::move(ef));
    }
}

void GameContext::Reset()
{
    // ステージ再開用の軽量リセット（プレイヤー/弾/エフェクト中心）
    player->Reset();
    projectiles.clear();
    g_pendingProjectiles.clear();

    // Reset effects（プールをそのまま再利用）
    for (auto& ef : effects) ef->Reset();
}

void GameContext::Update()
{
    // 防衛: プレイヤーが無効なら処理しない
    if (!player) {
        DxPlus::Debug::SetString(L"GameContext::Update: player == nullptr");
        return;
    }

    // フレーム開始時の全エンティティの位置を保存（移動キャンセル用）
    std::vector<DxPlus::Vec2> prevPositions;
    prevPositions.reserve(entities.size());
    for (auto& e : entities) prevPositions.push_back(e ? e->GetPosition() : DxPlus::Vec2{});

    camera.Update();


    // エネミーを数歩ごとに生成
    int WalkCount = PlayerWalkCount; // 注意: ローカルの 0 代入ではグローバルは変化しない

    if (EnemyCount < 10 && WalkCount < 10)
    {
        auto enemy = std::make_unique<Enemy>();
        enemy->BindContext(this);
        enemy->Init();
        enemy->Reset(); // 新規敵のみ
        entities.push_back(std::move(enemy));

        EnemyCount++;
        WalkCount = 0; // グローバルの歩数は変わらない点に注意
    }

    // エンティティの更新/移動反映
    for (auto& e : entities) if (e) e->Update();
    for (auto& e : entities) if (e) e->Step();



    // 保留中発射物を projectiles に移す（イテレーション安全）
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

    // Update effects（再生中のみ更新）
    for (auto& ef : effects) if (ef) ef->Update();

    // プレイヤーと他エンティティの重なりチェック（押し戻し）
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

    // 削除: 再生完了したエフェクトをクリア
    effects.erase(std::remove_if(effects.begin(), effects.end(), [](const std::unique_ptr<AttackEffectEnemy>& e){ return !e->IsAlive(); }), effects.end());

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

    // プレイヤーの足元タイル更新（ミニマップ/可視化用の訪問マーキング）
    float tw = Map::GetTileWidth();
    float th = Map::GetTileHeight();
    DxPlus::Vec2 pos = player->Position();
    int col = static_cast<int>(std::floor((pos.x - tw / 2.0f) / tw));
    int row = static_cast<int>(std::floor((pos.y - th / 2.0f) / th));
    if (row >= 0 && row < Map::GetRows() && col >= 0 && col < Map::GetCols())
    {
        for (int i = -1; i < 2; i++)
        {
            if (g_tileStatus[row][col + i] == 0) // 0: 空の床
            {
                g_tileStatus[row][col + i] = 2; // 2: 歩かれた (色を変える状態)
            }
            if (g_tileStatus[row + i][col] == 0) // 0: 空の床
            {
                g_tileStatus[row + i][col] = 2; // 2: 歩かれた (色を変える状態)
            }
            if (g_tileStatus[row + i][col + i] == 0) // 0: 空の床
            {
                g_tileStatus[row + i][col + i] = 2; // 2: 歩かれた (色を変える状態)
            }
            if (g_tileStatus[row + i][col - i] == 0) // 0: 空の床
            {
                g_tileStatus[row + i][col - i] = 2; // 2: 歩かれた (色を変える状態)
            }
            static bool opened = false;
        }
    }

    //プレイヤー正面1マスにいる敵を攻撃する（射撃ボタンかつ歩数満タンのとき）
    using namespace DxPlus::Input;
    int inputButton = GetButtonDown(PLAYER1);
    bool attack = (inputButton & BUTTON_TRIGGER2) != 0;
    if (attack && PlayerWalkCount == Const::MAX_PLAYER_WALK_COUNT)
    {
        // プレイヤーの向きは直近の移動（prevPos -> pos）から推定、移動がない場合は下向きデフォルト
        DxPlus::Vec2 prevPos = player->GetPrevPosition();
        DxPlus::Vec2 delta = pos - prevPos;
        const float EPS = 0.0001f;
        DxPlus::Vec2 dir{ 0.0f, 1.0f }; // デフォルト下
        if (std::fabs(delta.x) > EPS || std::fabs(delta.y) > EPS)
        {
            dir = delta.Normalize();
        }

        // 方向を4方向に丸める（左右優先）
        int dx = 0, dy = 0;
        if (std::fabs(dir.x) > std::fabs(dir.y))
        {
            dx = dir.x > 0 ? 1 : -1;
        }
        else
        {
            dy = dir.y > 0 ? 1 : -1;
        }

        int targetCol = col + dx;
        int targetRow = row + dy;
        if (targetRow >= 0 && targetRow < Map::GetRows() && targetCol >= 0 && targetCol < Map::GetCols())
        {
            for (auto& e : entities)
            {
                if (!e || !e->IsAlive() || !e->IsDamageable()) continue;
                DxPlus::Vec2 ePos = e->GetPosition() + e->GetCenterOffset();
                int eCol = static_cast<int>(std::floor((ePos.x - tw / 2.0f) / tw));
                int eRow = static_cast<int>(std::floor((ePos.y - th / 2.0f) / th));
                if (eCol == targetCol && eRow == targetRow)
                {
                    // プレイヤーの攻撃力 + 魅力度を渡す
                    e->OnHit(player->Attack() + Attractiveness);
                    break; // 正面1マスの最初の敵だけ攻撃
                }
            }
        }
    }
    
    bool plus = (inputButton & BUTTON_1) != 0;
    bool minus = (inputButton & BUTTON_2) != 0;
    if (plus)
    {
        attackType = static_cast<AttackType>(attackType+1);
    }
    if (minus)
    {
        attackType = static_cast<AttackType>(attackType-1);
    }


    // 削除処理（死んだエンティティ/弾を掃除）
    entities.erase(std::remove_if(entities.begin(), entities.end(), [](const std::unique_ptr<Entity2D>& p){ return !p->IsAlive(); }), entities.end());
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const std::unique_ptr<Projectile>& p){ return !p->IsAlive(); }), projectiles.end());

    // アイテム回収判定（距離の二乗で省コストに実装）
    for (auto& item : Items)
    {
        float dx = player->Position().x - item->Position().x;
        float dy = player->Position().y - item->Position().y;
        float dist2 = dx*dx + dy*dy;
        float r = player->Radius() + item->Radius();
        if (dist2 <= r * r)
        {
            Attractiveness++;
            ItemGet = true;
            item->Reset();
        }
    }

    if (ItemGet)
    {
        ItemGetTimer++;

        // 1フレーム目でプレイヤー位置を記録（表示開始位置）
        if (ItemGetTimer == 1 && player)
        {
            g_itemPopupPos = player->GetPosition() + player->GetCenterOffset();
        }

        // 一定時間で終了
        if (ItemGetTimer > 60) // 約1秒表示（60fps想定）
        {
            ItemGet = false;
            ItemGetTimer = 0;
        }
    }

    // 入力（魅力度増減）
    using namespace DxPlus::Input;
    int button = GetButtonDown(PLAYER1);
    bool up = (button & BUTTON_TRIGGER3) != 0;
    bool down = (button & BUTTON_TRIGGER4) != 0;
    if (up) ++Attractiveness;
    if (down) { --Attractiveness; if (Attractiveness < 0) Attractiveness = 0; }

    if (Attractiveness > Const::MAX_ATTRACTIVIENESS)
    {
        Attractiveness = Const::MAX_ATTRACTIVIENESS;
    }

    // デバッグ HUD 表示
    std::wstring text = L"Player Position("
        + std::to_wstring(static_cast<int>(player->GetPosition().x))
        + L"," + std::to_wstring(static_cast<int>(player->GetPosition().y))
        + L")  ショットカウント=" + std::to_wstring(static_cast<int>(PlayerWalkCount))
        + L"\n魅力度下げるShift上げるAlt"
        + std::to_wstring(static_cast<int>(Attractiveness))
        + L"\nAttackType上げる1下げる2:"
        + std::to_wstring(static_cast<int>(attackType));

    debugHudText = std::move(text);
}

namespace {
    constexpr float PI_F = 3.14159265358979323846f;
    inline float DegToRad(float deg) noexcept { return deg * (PI_F / 180.0f); }
}

// 1桁の数字スプライトを目的サイズで描画
static void DrawDigitWithSize(const DxPlus::Sprite::SpriteBase* s, const DxPlus::Vec2& center, float width, float height);


// 3桁の数字（例: Numbersの {0..9,0} を使用）を目的サイズで描画
static void DrawNumber3Ex(const DxPlus::Vec2& screenCenter, int value, float digitWidth, float digitHeight, float digitSpacing);




void GameContext::Draw() const
{
    if (!player) return;

    float camX = camera.GetX();
    float camY = camera.GetY();
    float tw = Map::GetTileWidth();
    float th = Map::GetTileHeight();
    const DxPlus::Sprite::SpriteBase* tileSprite = RM().GridAt(ResourceKeys::Tiles_City);
    const DxPlus::Sprite::SpriteBase* tileSprite2 = RM().GridAt(ResourceKeys::Tiles_City_2);
    const DxPlus::Sprite::SpriteBase* treasureitem = RM().GridAt(ResourceKeys::treasureItem);
    const DxPlus::Sprite::SpriteBase* UI = RM().GridAt(ResourceKeys::UI);
    const DxPlus::Sprite::SpriteBase* MapnameUI = RM().GridAt(ResourceKeys::MapnameUI);
    const DxPlus::Sprite::SpriteBase* turnUI = RM().GridAt(ResourceKeys::turnUI);
    const DxPlus::Sprite::SpriteBase* Shot = RM().GridAt(ResourceKeys::Item);
    const DxPlus::Sprite::SpriteBase* BuildingSprite = RM().GridAt(ResourceKeys::Building_big);
    const DxPlus::Sprite::SpriteBase* BuildingSprite_small = RM().GridAt(ResourceKeys::Building_small);


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
            float left = centerPos.x - tw / 2 - camX;
            float top = centerPos.y - th / 2 - camY;

            if (tileStatus == 2)
            {
                DrawBox(static_cast<int>(left), static_cast<int>(top), static_cast<int>(left + tw), static_cast<int>(top + th), GetColor(150, 150, 150), TRUE);
            }
            if (tileStatus == 0 || tileStatus == 2)
            {
                float leftf = std::floor(centerPos.x - tw / 2 - camX);
                float topf = std::floor(centerPos.y - th / 2 - camY);
                tileSprite->Draw(DxPlus::Vec2(leftf, topf));
            }
            else if (tileStatus == 5)
            {
                float leftf = std::floor(centerPos.x - tw / 2 - camX);
                float topf = std::floor(centerPos.y - th / 2 - camY);
                tileSprite2->Draw(DxPlus::Vec2(leftf, topf));

            }

            else if (tileStatus == 4)
            {
                float left = std::floor(centerPos.x - tw / 2 - camX);
                float top = std::floor(centerPos.y - th / 2 - camY);

                DxPlus::Primitive2D::DrawRect(
                    { left, top },
                    { tw, th },
                    GetColor(255, 255, 255));
                TreasureType treasureType = g_treasureType[row][col];
                switch (treasureType)
                {
                case TreasureType::Ishab:
                {
                    treasureitem->Draw(DxPlus::Vec2(centerPos.x - tw / 2 - camX, centerPos.y - th / 2 - camY));
                    break;
                }
                case TreasureType::Sword:
                {
                    BuildingSprite->Draw(DxPlus::Vec2(centerPos.x - tw / 2 - camX, centerPos.y - th / 2 - camY));
                    break;
                }
                case TreasureType::Shield:
                {
                    BuildingSprite_small->Draw(DxPlus::Vec2(centerPos.x - tw / 2 - camX, centerPos.y - th / 2 - camY));
                    break;
                }
                case TreasureType::AttractUp:
                {
                    tileSprite->Draw(DxPlus::Vec2(left, top));
                    break;
                }
                case TreasureType::AttractDown:
                {
                    Shot->Draw(DxPlus::Vec2(left, top));
                }

                }

                //float left = std::floor(centerPos.x - tw / 2 - camX);
                //float top = std::floor(centerPos.y - th / 2 - camY);
                //BuildingSprite->Draw(DxPlus::Vec2(centerPos.x-tw/ 2 - camX,((centerPos.y-490)/ 2 - camY)));
            }

        }
    }

    // アイテム・壁・エンティティ・弾の描画
    for (auto& item : Items) item->CameraDraw(camX, camY);
    for (auto& w : Walls)
    {
        float left = w->GetPos().x - w->GetWidth() / 2 - camX;
        float top = w->GetPos().y - w->GetHeight() / 2 - camY;
        DxPlus::Primitive2D::DrawRect({ left, top }, { w->GetWidth(), w->GetHeight() }, GetColor(0, 255, 0));
    }
    for (auto& e : entities)
    {
        if (e.get() == player) continue; // プレイヤーは最後に描画する
        e->CameraDraw(camX, camY);
    }

    // エフェクトはここで描画して、プレイヤーや他エンティティがエフェクトの上に来るようにする
    for (auto& ef : effects) if (ef && ef->IsAlive()) ef->CameraDraw(camX, camY);

    if (player && player->IsAlive()) player->CameraDraw(camX, camY);
    //if (DxPlus::Input::GetButtonDown(DxPlus::Input::PLAYER1) & DxPlus::Input::BUTTON_START)
    //{
    //    a = 1;
    //}
    //if (fadeIn)
    //{
    //    flashAlpha += 5;           // フェードイン速度
    //    if (flashAlpha >= 100)
    //    {
    //        flashAlpha = 100;
    //        fadeIn = false;
    //    }
    //}
    //else
    //{
    //    flashAlpha -= 5;           // フェードアウト速度
    //    if (flashAlpha <= 0)
    //    {
    //        flashAlpha = 0;
    //        fadeIn = true;
    //    }
    //}

    //// 描画
    //int gameLeft = 130;
    //int gameTop = 0;
    //int gameRight = 1280 - 300;
    //int gameBottom = 720;

    //SetDrawBlendMode(DX_BLENDMODE_ALPHA, flashAlpha);
    //DrawBox(gameLeft, gameTop, gameRight, gameBottom, GetColor(255, 0, 0), TRUE);
    //SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);


    if (a == 1)
    {
        if (!isStopping)
        {
            j += 2;

            int x = 1600 - 10 * j;

            if (x <= 700)
            {
                isStopping = true;
                stopTimer = 0;
            }
        }
        else
        {
            // 停止中
            stopTimer++;

            if (stopTimer >= 200) // 3秒停止
            {
                isStopping = false;  // 停止解除
                a = 2;               // ← 停止後のフェーズに切り替え
            }
        }
    }
    else if (a == 2)
    {
        // 停止後の移動をここで担当する
        j += 3;    // ← 一気に速くしたいならここを調整する
    }
    // 描画
    turnUI->Draw(DxPlus::Vec2(1600 - 10 * j, 350));

    if (DxPlus::Input::GetButtonDown(DxPlus::Input::PLAYER1) & DxPlus::Input::BUTTON_TRIGGER3)
    {
        b = 1;
    }
    if (b == 1)
    {
        int x = 450 - d * 10;
        d++;
        alpha += 10;
        if (alpha > 255)alpha = 255;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        MapnameUI->Draw(DxPlus::Vec2(x, 80));
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        if (x < 250)
        {
            d = 0;
            b = 2;
        }

    }
    if (b == 2)
    {
        int x = 250 - d * 20;
        alpha += 4;
        if (alpha > 255)alpha = 255;
        if (alpha == 255)
        {
            Timer++;
        }
        if (Timer >= 120)
        {
            d++;
        }
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        MapnameUI->Draw(DxPlus::Vec2(x, 80));
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);


    }

    UI->Draw(DxPlus::Vec2(640, 360));

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);

    DxPlus::Primitive2D::DrawRect({ 1047,360 }, { 200.0f / Const::MAX_ATTRACTIVIENESS * Attractiveness,24.0f},DxLib::GetColor(255,0,0));

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    int value = std::clamp(Const::TURN_MAX - PlayerWalkCountALL, 0, 999);
    DrawNumber3Ex({ 1160.0f, 44.0f }, value, 120.0f, 118.0f, 38.0f);


    // ミニマップ（小さいため全タイル走査でOK）
    const float miniScale = 0.05f;
    const float miniX = 1050.0f, miniY = 150.0f;
    for (int row = 0; row < Map::GetRows(); ++row)
    {
        for (int col = 0; col < Map::GetCols(); ++col)
        {
            DxPlus::Vec2 c = Map::GetTileCenterPosition(row, col);
            float MinX = c.x * miniScale + miniX;
            float MinY = c.y * miniScale + miniY;
            float tw2 = tw * miniScale;
            float th2 = th * miniScale;
            float left = MinX - tw2 / 2;
            float top = MinY - th2 / 2;
            if (g_tileStatus[row][col] == 2)
            {
                DrawBox(static_cast<int>(MinX - tw2 / 2), static_cast<int>(MinY - th2 / 2), static_cast<int>(MinX + tw2 / 2), static_cast<int>(MinY + th2 / 2), GetColor(150, 150, 150), TRUE);
            }
            if (g_tileStatus[row][col] == 3) {
                DrawBox(static_cast<int>(MinX - tw2 / 2), static_cast<int>(MinY - th2 / 2), static_cast<int>(MinX + tw2 / 2), static_cast<int>(MinY + th2 / 2), GetColor(0, 0, 255), TRUE);
            }


        }
    }

    for (auto& item : Items)
    {
        float scale = 0.05f;
        float offsetX = 1050;
        float offsetY = 150;
        float iconSize = 120 * scale;
        DxPlus::Vec2 pos = item->GetPosition();

        // ミニマップ座標に変換
        float miniX = pos.x * scale + offsetX;
        float miniY = pos.y * scale + offsetY;

        float left = miniX - iconSize / 2;
        float top = miniY - iconSize / 2;

        // 赤い四角で描画
        DrawBox(
            (int)left,
            (int)top,
            (int)(left + iconSize),
            (int)(top + iconSize),
            GetColor(255, 0, 0),
            TRUE
        );
    }
    for (auto& Playre : entities)
    {
        float scale = 0.05f;
        float offsetX = 1050;
        float offsetY = 150;
        float iconSize = 120 * scale;
        DxPlus::Vec2 pos = Playre->GetPosition();

        // ミニマップ座標に変換
        float miniX = pos.x * scale + offsetX;
        float miniY = pos.y * scale + offsetY;

        float left = miniX - iconSize / 2;
        float top = miniY - iconSize / 2;

        // 赤い四角で描画
        DrawBox(
            (int)left,
            (int)top,
            (int)(left + iconSize),
            (int)(top + iconSize),
            GetColor(0, 255, 0),
            TRUE
        );
    }

   //HUD
    DxPlus::Vec2 hudCenter{57.0f, 639.0f};
    float hudRadius = 50.0f;
    unsigned int cols = DxLib::GetColor(50,50,50);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    const_cast<GameContext*>(this)->SetCircleSweepDeg(static_cast<float>(360 - PlayerWalkCount * 360 / Const::MAX_PLAYER_WALK_COUNT));
    // 分割数を抑えて頂点生成コストを下げる
    DrawFilledSectorScreen(hudCenter, hudRadius, {1.0f,1.0f}, 0.0f, 64, cols);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    const int black = DxLib::GetColor(0, 0, 0);
    DxPlus::Text::DrawString(
        debugHudText.c_str(),
        { 10.0f, 10.0f },                 // 左上に表示（必要に応じて座標調整）
        black,
        DxPlus::Text::TextAlign::TOP_LEFT,
        { 1.0f, 1.0f },
        0.0,
        -1);

    if (ItemGet)
    {
        // 経過に応じて少し上に移動し、フェードアウト
        float t = static_cast<float>(ItemGetTimer);
        float rise = t * 0.5f; // 上昇量
        int alpha = static_cast<int>(std::max(0.0f, 255.0f - t * 4.0f)); // フェード（約1秒で消える）

        // ワールド→スクリーン座標へ変換
        float camX = camera.GetX();
        float camY = camera.GetY();
        DxPlus::Vec2 screenPos{ g_itemPopupPos.x - camX, g_itemPopupPos.y - camY - rise };

        // 半透明で文字を描画
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DxPlus::Text::DrawString(
            L"魅力度+1",
            screenPos,
            DxLib::GetColor(255, 255, 0),                 // 目立つ黄色
            DxPlus::Text::TextAlign::MIDDLE_CENTER,              // 中央揃え
            { 1.0f, 1.0f },
            0.0f,
            -1);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

bool GameContext::SpawnProjectile(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel) noexcept
{
    // プール上限を超えないように保留バッファ含めてチェック
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
    // 円 vs 円（他エンティティ）
    for (const auto& e : entities)
    {
        if (!e || !e->IsAlive() || e.get() == ignore) continue;
        if (Collision2D::CircleVsCircle(pos, radius, e->GetPosition() + e->GetCenterOffset(), e->Radius())) return false;
    }
    // 円 vs AABB（壁）
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
    // スクリーン座標に直接扇形ポリゴンを出力（HUD 用）
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

void GameContext::AttackFront(const Player* attacker, const DxPlus::Vec2& dir) noexcept
{
    // プレイヤーの正面方向（丸めた 4 方向）に応じたタイルを走査し、敵へダメージ適用
    if (!attacker) return;

    // タイル計算
    float tw = Map::GetTileWidth();
    float th = Map::GetTileHeight();
    DxPlus::Vec2 pos = attacker->GetPosition() + attacker->GetCenterOffset();
    int col = static_cast<int>(std::floor((pos.x - tw / 2.0f) / tw));
    int row = static_cast<int>(std::floor((pos.y - th / 2.0f) / th));

    if (row < 0 || row >= Map::GetRows() || col < 0 || col >= Map::GetCols()) return;

    // 方向を4方向に丸める（左右優先）
    int dx = 0, dy = 0;
    if (std::fabs(dir.x) > std::fabs(dir.y))
    {
        dx = dir.x > 0 ? 1 : -1;
    }
    else
    {
        dy = dir.y > 0 ? 1 : -1;
    }


    switch (attackType)
    {
    case GameContext::frontSquare1:
        for (auto& e : entities)
        {
            int targetCol = col + dx;
            int targetRow = row + dy;
            if (targetRow < 0 || targetRow >= Map::GetRows() || targetCol < 0 || targetCol >= Map::GetCols()) return;

            if (!e || !e->IsAlive() || !e->IsDamageable()) continue;
            DxPlus::Vec2 ePos = e->GetPosition() + e->GetCenterOffset();
            int eCol = static_cast<int>(std::floor((ePos.x - tw / 2.0f) / tw));
            int eRow = static_cast<int>(std::floor((ePos.y - th / 2.0f) / th));
            if (eCol == targetCol && eRow == targetRow)
            {
                e->OnHit(attacker->Attack() + Attractiveness);

            }
        }

        break;
    case GameContext::flontSquere2:
        for (int dist = 1; dist <= 2; ++dist)
        {
            int targetCol = col + dx * dist;
            int targetRow = row + dy * dist;
            if (targetRow < 0 || targetRow >= Map::GetRows() || targetCol < 0 || targetCol >= Map::GetCols()) continue;

            for (auto& e : entities)
            {
                if (!e || !e->IsAlive() || !e->IsDamageable()) continue;
                DxPlus::Vec2 ePos = e->GetPosition() + e->GetCenterOffset();
                int eCol = static_cast<int>(std::floor((ePos.x - tw / 2.0f) / tw));
                int eRow = static_cast<int>(std::floor((ePos.y - th / 2.0f) / th));
                if (eCol == targetCol && eRow == targetRow)
                {
                    e->OnHit(attacker->Attack() + Attractiveness);

                }
            }
        }
        break;
    case GameContext::flontWide3:

        // 3マス（正面 + 左右）を走査して、それぞれのタイルにいる敵にダメージを与える
        // 左右の定義: 進行方向が左右なら上下（row-1,row,row+1）、進行方向が上下なら左右（col-1,col,col+1）
        for (int side = -1; side <= 1; ++side)
        {
            int targetCol = col + dx;
            int targetRow = row + dy;

            if (dx != 0) // 右/左を向いている → 左右は行方向にオフセット
            {
                targetRow = targetRow + side;
            }
            else // 上/下を向いている → 左右は列方向にオフセット
            {
                targetCol = targetCol + side;
            }

            if (targetRow < 0 || targetRow >= Map::GetRows() || targetCol < 0 || targetCol >= Map::GetCols()) continue;

            // そのタイルにいる敵を探索して攻撃（タイル中の全ダメージ可能エンティティに当てる）
            for (auto& e : entities)
            {
                if (!e || !e->IsAlive() || !e->IsDamageable()) continue;
                DxPlus::Vec2 ePos = e->GetPosition() + e->GetCenterOffset();
                int eCol = static_cast<int>(std::floor((ePos.x - tw / 2.0f) / tw));
                int eRow = static_cast<int>(std::floor((ePos.y - th / 2.0f) / th));
                if (eCol == targetCol && eRow == targetRow)
                {
                    e->OnHit(attacker->Attack() + Attractiveness);
                }
            }
        }
        break;
    case GameContext::flontWide6:
        for (int dist = 1; dist <= 2; ++dist)
        {
            for (int side = -1; side <= 1; ++side)
            {
                int targetCol = col + dx * dist;
                int targetRow = row + dy * dist;

                if (dx != 0) // 右/左を向いている → 左右は行方向にオフセット
                {
                    targetRow = targetRow + side;
                }
                else // 上/下を向いている → 左右は列方向にオフセット
                {
                    targetCol = targetCol + side;
                }

                if (targetRow < 0 || targetRow >= Map::GetRows() || targetCol < 0 || targetCol >= Map::GetCols()) continue;

                // そのタイルにいる敵を探索して攻撃（タイル中の全ダメージ可能エンティティに当てる）
                for (auto& e : entities)
                {
                    if (!e || !e->IsAlive() || !e->IsDamageable()) continue;
                    DxPlus::Vec2 ePos = e->GetPosition() + e->GetCenterOffset();
                    int eCol = static_cast<int>(std::floor((ePos.x - tw / 2.0f) / tw));
                    int eRow = static_cast<int>(std::floor((ePos.y - th / 2.0f) / th));
                    if (eCol == targetCol && eRow == targetRow)
                    {
                        e->OnHit(attacker->Attack() + Attractiveness);
                    }
                }
            }
        }

        break;
    case GameContext::backAndForthAndAround:
    
        // プレイヤーの上下左右（4方向）をすべて攻撃する
        int dmg = attacker->Attack() + Attractiveness;
        const std::pair<int, int> offsets[4] = {
            {0, -1}, // 上
            {-1, 0}, // 左
            {1, 0},  // 右
            {0, 1}   // 下
        };

        for (auto off : offsets)
        {
            int targetCol = col + off.first;
            int targetRow = row + off.second;

            if (targetRow < 0 || targetRow >= Map::GetRows() || targetCol < 0 || targetCol >= Map::GetCols()) continue;

            for (auto& e : entities)
            {
                if (!e || !e->IsAlive() || !e->IsDamageable()) continue;
                DxPlus::Vec2 ePos = e->GetPosition() + e->GetCenterOffset();
                int eCol = static_cast<int>(std::floor((ePos.x - tw / 2.0f) / tw));
                int eRow = static_cast<int>(std::floor((ePos.y - th / 2.0f) / th));
                if (eCol == targetCol && eRow == targetRow)
                {
                    e->OnHit(dmg);
                }
            }
        }
    
    break;
    }
}

void GameContext::SpawnEnemyHitEffect(const DxPlus::Vec2& worldPos) noexcept
{
    // 可能ならプールから非アクティブ個体を再利用
    for (auto& ef : effects)
    {
        if (!ef->IsAlive())
        {
            ef->PlayAt(worldPos);
            return;
        }
    }

    // 見つからなければ新規確保（オーバーアロケーションを避けるため、必要に応じて上限設定を検討）
    auto ef = std::make_unique<AttackEffectEnemy>();
    ef->BindContext(this);
    ef->Init();
    ef->PlayAt(worldPos);
    effects.push_back(std::move(ef));
}

// 1桁の数字スプライトを目的サイズで描画
static void DrawDigitWithSize(const DxPlus::Sprite::SpriteBase* s, const DxPlus::Vec2& center, float width, float height)
{
    if (!s) return;
    const int handle = s->GetID();
    if (handle < 0) return;

    const int left = static_cast<int>(center.x - width * 0.22f);
    const int top = static_cast<int>(center.y - height * 0.22f);
    const int right = static_cast<int>(center.x + width * 0.22f);
    const int bottom = static_cast<int>(center.y + height * 0.22f);

    DxLib::DrawExtendGraph(left, top, right, bottom, handle, TRUE);
}

// 3桁の数字（例: Numbersの {0..9,0} を使用）を目的サイズで描画
static void DrawNumber3Ex(const DxPlus::Vec2& screenCenter, int value, float digitWidth, float digitHeight, float digitSpacing)
{
    value = std::clamp(value, 0, 999);

    auto digitToGridX = [](int d) { return (d == 0) ? 9 : (d - 1); };
    const int d2 = (value / 100) % 10;
    const int d1 = (value / 10) % 10;
    const int d0 = (value / 1) % 10;

    const auto* sHund = RM().GridAt(ResourceKeys::Numbers, digitToGridX(d2), 0);
    const auto* sTen = RM().GridAt(ResourceKeys::Numbers, digitToGridX(d1), 0);
    const auto* sOne = RM().GridAt(ResourceKeys::Numbers, digitToGridX(d0), 0);
    if (!sHund || !sTen || !sOne) return;

    DxPlus::Vec2 posHund{ screenCenter.x - digitSpacing, screenCenter.y };
    DxPlus::Vec2 posTen{ screenCenter.x, screenCenter.y };
    DxPlus::Vec2 posOne{ screenCenter.x + digitSpacing,  screenCenter.y };

    DrawDigitWithSize(sHund, posHund, digitWidth, digitHeight);
    DrawDigitWithSize(sTen, posTen, digitWidth, digitHeight);
    DrawDigitWithSize(sOne, posOne, digitWidth, digitHeight);
}
