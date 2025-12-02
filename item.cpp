#include "item.h"
#include "DxPlus/DxPlus.h"
#include "GameContext.h"
#include "map.h"
#include <cmath>
#include "ResourceKeys.h"
#include "ResourceManager.h"
void Item::Init()
{
    ItemTipe = rand() % 2;
    switch (ItemTipe)
    {
    case 0:
    {
        sprite = RM().GridAt(ResourceKeys::Enemy);
        break;
    }
    case 1:
    {
        sprite = RM().GridAt(ResourceKeys::Item);
        break;
    }      
    }
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

                // タイル中心の座標を取得
                DxPlus::Vec2 candidate = Map::GetTileCenterPosition(row, col);

                // 重なりチェック
                if (GC().IsPositionFree(candidate, Radius(), this))
                {
                    position = candidate;
                    return;
                }
                DxPlus::Primitive2D::DrawRect({ position }, { 20,20 }, GetColor(255, 0, 0));

            
        }

    
}

void Item::Update()
{
   
}

void Item::Draw()
{
}
	


