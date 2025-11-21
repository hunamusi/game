// =============================
// Core/Map.h 
// =============================
class Map
{
public:
    // タイルのワールドサイズ (const float TILE_W = 120.0f; などをクラスに移動)
    static constexpr float GetTileWidth() { return 120.0f; }
    static constexpr float GetTileHeight() { return 120.0f; }

    // マップの行数と列数を取得
    static constexpr int GetRows() { return 6; }
    static constexpr int GetCols() { return 20; }

    // 指定された座標のタイルデータ（1:壁, 0:空）を取得
    static int GetTileData(int row, int col)
    {
        // ... (以前の回答で提案した静的ローカル変数 mapData の定義と取得ロジック)
        static const int mapData[6][20] =
        {
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
        };

        if (row < 0 || row >= GetRows() || col < 0 || col >= GetCols()) {
            return 0;
        }
        return mapData[row][col];
    }

    // ★ 追加: タイル座標からワールド中心座標を計算するメソッド
    static DxPlus::Vec2 GetTileCenterPosition(int row, int col)
    {
        float TILE_W = GetTileWidth();
        float TILE_H = GetTileHeight();

        // 元の計算式: x = col * TILE_W + TILE_W / 2;
        //            y = row * TILE_H + TILE_H / 2;
        float x = col * TILE_W + (TILE_W / 2);
        float y = row * TILE_H + (TILE_H / 2);

        return { x, y };
    }
};