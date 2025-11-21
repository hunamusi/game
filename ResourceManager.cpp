// =============================
// Resources/ResourceManager.cpp
// =============================
#include "ResourceManager.h"
#include "DxPlus/DxPlus.h"
#include "ResourceKeys.h"

ResourceManager& ResourceManager::GetInstance()
{
    static ResourceManager instance;
    return instance;
}

void ResourceManager::LoadAll()
{
    LoadTextureAsSpriteLeftTop(ResourceKeys::Background, L"./Data/Images/2025-11-18_162755.png");
    LoadGridBottom(ResourceKeys::Player, L"./Data/Images/player.png", { 0,0 }, { 3,4 }, { 96,128 });
    LoadGridBottom(ResourceKeys::Enemy_Yankee, L"./Data/Images/yankee.png", { 0,0 }, { 3,4 }, { 96, 128 });
    LoadTextureAsSpriteLeftTop(ResourceKeys::Player_Shot, L"./Data/Images/player_shot.png");
    LoadFont(ResourceKeys::Font_Title, L"./Data/Fonts/Bitcount/static/Bitcount-Light.ttf");

    RegisterTileset(ResourceKeys::Tiles_Background, L"./Data/MapData/BackChip.png", 64, 64);
    {
        std::vector<int8_t> backTypes
        {
            Tile_None,  Tile_None,  Tile_None,  Tile_None,  Tile_None,  Tile_None,  Tile_None,  Tile_None,
            Tile_Water, Tile_Water, Tile_Water, Tile_Water, Tile_Water, Tile_Water, Tile_Water, Tile_Water,
        };
        Tileset* ts = GetTileset(ResourceKeys::Tiles_Background);
        const size_t totalTiles = static_cast<size_t>(ts->tileCountX * ts->tileCountY);
        backTypes.resize(totalTiles, Tile_None);
        if (ts) ts->types = std::move(backTypes);
    }

    RegisterTileset(ResourceKeys::Tiles_Terrain, L"./Data/MapData/TerrainChip.png", 64, 64);
    {
        std::vector<int8_t> terrainTypes
        {
            Tile_Solid, Tile_Solid, Tile_Solid, Tile_Solid, Tile_Solid, Tile_Solid, Tile_Platform, Tile_Platform,
            Tile_Solid, Tile_Solid, Tile_Solid, Tile_Solid, Tile_Solid, Tile_Solid, Tile_Platform, Tile_Platform,
        };
        Tileset* ts = GetTileset(ResourceKeys::Tiles_Terrain);
        const size_t totalTiles = static_cast<size_t>(ts->tileCountX * ts->tileCountY);
        terrainTypes.resize(totalTiles, Tile_None);
        if (ts) ts->types = std::move(terrainTypes);
    }
}

void ResourceManager::UnloadAll()
{
    UnloadGrids();
    UnloadFonts();
}

// ===============================[  GRIDS  ]===================================

const DxPlus::Sprite::SpriteBase* ResourceManager::GridAt(const std::wstring& key, int x, int y)
{
    auto it = grids.find(key);
    if (it == grids.end()) return nullptr;
    const auto& g = it->second;
    if (x < 0 || x >= g.num.x || y < 0 || y >= g.num.y) return nullptr;
    int idx = y * g.num.x + x;
    return &g.frames[idx];
}

void ResourceManager::UnloadGrids()
{
    for (auto& kv : grids) {
        for (auto& f : kv.second.frames) {
            int gid = f.GetID();
            if (gid >= 0) DxPlus::Sprite::Delete(gid);
        }
    }
    grids.clear();
}
Tileset* ResourceManager::GetTileset(const std::wstring& key)
{
    auto it = tilesets.find(key);
    if (it == tilesets.end()) return nullptr;
    return &it->second;
}

int ResourceManager::GetTileHandle(const std::wstring& key, int tileID)
{
    const Tileset* ts = GetTileset(key);
    if (!ts) return -1;
    if (tileID < 0 || tileID >= static_cast<int>(ts->handles.size())) return -1;
    return ts->handles[tileID];
}
int8_t ResourceManager::GetTileType(const std::wstring& key, int tileID)
{
    const Tileset* ts = GetTileset(key);
    if (!ts)return-1;
    if (tileID < 0 || tileID >= static_cast<int>(ts->handles.size())) return TileType::Tile_None;
    return ts->types[tileID];
}
void ResourceManager::UnregisterTileset(const std::wstring& key)
{
    auto it = tilesets.find(key);               // tilesetsからkeyのタイルセットを探す
    if (it == tilesets.end()) return;           // 結果がtilesets.end()なら、見つからなかった
    for (auto h : it->second.handles)           // 全ての画像でループする
    {
        if (h >= 0) DxPlus::Sprite::Delete(h);  // 画像IDが0以上ならその画像を削除する
    }
    tilesets.erase(it);                         // tilesetsから指定キーのタイルセットを削除する
}

// ===============================[  FONTS  ]===================================

int ResourceManager::GetFont(const std::wstring& fontName)
{
    auto it = fonts.find(fontName);
    if (it == fonts.end())
    {
        DxPlus::Utils::FatalError((L"Font not found: " + fontName).c_str());
    }
    return it->second.handle;
}

int ResourceManager::LoadFont(const std::wstring& fontName, const std::wstring& path)
{
    if (AddFontResourceExW(path.c_str(), FR_PRIVATE, 0) == 0)
    {
        DxPlus::Utils::FatalError((std::wstring(L"Failed to add font: ") + path).c_str());
    }

    int handle = DxPlus::Text::InitializeFont(fontName.c_str(), 40, 2);
    if (handle == -1)
    {
        DxPlus::Utils::FatalError((std::wstring(L"Failed to init font: ") + fontName).c_str());
    }

    fonts[fontName] = { handle, path };

    return handle;
}

bool ResourceManager::RegisterTileset(const std::wstring& key, const std::wstring& path, int chipW, int chipH)
{
    // DxLibの機能で画像全体の幅と高さを取得する
    int handle = DxLib::LoadGraph(path.c_str());
    if (handle < 0) return false;

    int iw{ 0 }, ih{ 0 };
    DxLib::GetGraphSize(handle, &iw, &ih);
    DxLib::DeleteGraph(handle);

    const int tileCountX = iw / chipW;  // 横方向のタイル数
    const int tileCountY = ih / chipH;  // 縦方向のタイル数
    if (tileCountX <= 0 || tileCountY <= 0) return false;


    std::vector<int> hs(tileCountX * tileCountY, -1);
    if (DxPlus::Sprite::LoadDivGraph(path, tileCountX * tileCountY, tileCountX, tileCountY,
        chipW, chipH, hs.data()) == -1)
    {
        DxPlus::Utils::FatalError((L"Failed to LoadDivGraph :" + path).c_str());
    }

    Tileset ts{};
    ts.key = key;
    ts.chipW = chipW;
    ts.chipH = chipH;
    ts.tileCountX = tileCountX;
    ts.tileCountY = tileCountY;
    ts.handles = std::move(hs);
    tilesets[key] = ts;

    return true;
}
void ResourceManager::UnloadFont(const std::wstring& fontName)
{
    auto it = fonts.find(fontName);
    if (it == fonts.end()) return;

    auto& info = it->second;

    if (info.handle != -1)
    {
        DxPlus::Text::DeleteFont(info.handle);
        info.handle = -1;
    }

    if (!info.path.empty())
    {
        RemoveFontResourceExW(info.path.c_str(), FR_PRIVATE, 0);
    }

    fonts.erase(it);
}

void ResourceManager::UnloadFonts()
{
    std::vector<std::wstring> keys;
    keys.reserve(fonts.size());
    for (const auto& pair : fonts)// kv:key-value
    {
        keys.push_back(pair.first);
    }

    for (const auto& name : keys)
    {
        UnloadFont(name);
    }
}
