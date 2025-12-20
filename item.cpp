// 概要: ステージ内に配置される取得可能アイテム。
// 責務:
//  - 初期化時にスプライト設定とスポーン位置の決定
//  - 壁タイル回避と他オブジェクトとの非衝突を保証した配置（Reset）
//  - 取得は GameContext 側での距離判定により実現
// 設計メモ:
//  - アイテム自体は Update での能動的挙動を持たない（必要なら回転/アニメ追加）。
//  - 配置ランダムは簡易実装で最大試行回数を設け、見つからなければフォールバック。
#include "item.h"
#include "DxPlus/DxPlus.h"
#include "GameContext.h"
#include "map.h"
#include <cmath>
#include "ResourceKeys.h"
#include "ResourceManager.h"
void Item::Init()
{
    sprite = RM().GridAt(ResourceKeys::Enemy);
	Reset();
}
void Item::Reset()
{
    int row, col;

    const int maxAttempts = 30;
    int attempts = 0;

   
        while (attempts < maxAttempts)
        {
            row = rand() % Map::GetRows();
            col = rand() % Map::GetCols();
            attempts++;

            // 壁タイルはスキップ
            if (Map::GetTileData(row, col) == 1) continue;

            // タイル中心座標を取得
            DxPlus::Vec2 candidate = Map::GetTileCenterPosition(row, col);

            // 占有チェック
            if (GC().IsPositionFree(candidate, Radius(), this))
            {
                position = candidate;
                return;
            }
        }
        // 見つからない場合のフォールバック
        position = Map::GetTileCenterPosition(0, 0);

    
}

void Item::Update()
{
    // 受動オブジェクトのため毎フレーム処理は無し（将来の演出追加ポイント）
}


