// =============================
// Core/map.h 
// =============================
#include "mapdata.h"
#include "DxPlus/DxPlus.h"
class Map
{
public:
    // タイルのワールドサイズ (const float TILE_W = 120.0f; などをクラスに移動)
    static constexpr float GetTileWidth() { return 120.0f; }
    static constexpr float GetTileHeight() { return 120.0f; }

    // マップの行数と列数を取得
    static constexpr int GetRows() { return 20; }
    static constexpr int GetCols() { return 30; }
    static constexpr int ROWS = 20;
    static constexpr int COLS = 30;
    // ↑ 定数: マップの行数(20)と列数(30)を定義しています。
    //   'static constexpr' は、コンパイル時に値が決定される定数であることを示します。

 /*   static const int (*CurrentMap)[30];*/
    inline static const int (*CurrentMap)[30] = nullptr;
    // ↑ 静的メンバ変数: 現在使用されているマップデータを指すポインタです。
    //   型は「30個のintの配列へのポインタ」で、これは2次元配列（int map[ROWS][30]）を指すために使われます。
    //   'const' は、ポインタが指す配列の内容が変更されないことを示します。

    static void SetMap(const int(*map)[30])
    {
        CurrentMap = map;
    }
    // ↑ 静的関数: 外部から渡された2次元配列のポインタを CurrentMap に設定します。
    //   これにより、クラス全体で同じマップデータにアクセスできるようになります。

    static int GetTileData(int row, int col)
    {
        // row または col が有効な範囲外かどうかをチェックします。
        // COLSのチェックは通常 'col >= COLS' となりますが、ここでは 'col > COLS' となっています。
        // もしマップのインデックスが 0 から COLS-1 の範囲であれば、'col >= COLS' が正しい境界チェックです。
        if (row < 0 || row >= ROWS || col < 0 || col > COLS)
            return 0; // 範囲外の場合は、タイルのデータとして 0 を返します。

        return CurrentMap[row][col]; // 有効な範囲内であれば、その座標のタイルのデータを返します。
    }
    // ↑ 静的関数: 指定された (row, col) 座標のタイルのデータを取得します。
    //   アクセス前に境界チェックを行うことで、プログラムの安全性を高めています。
    
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
    DxPlus::Sprite::SpriteBase* Tile{ nullptr };
};
