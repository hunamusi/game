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
    LoadTextureAsSpriteLeftTop(ResourceKeys::Background, L"./Data/Images/back.png");
    LoadGridBottom(ResourceKeys::Player, L"./Data/Images/player.png", { 0,0 }, { 3,4 }, { 96,128 });
    LoadGridBottom(ResourceKeys::Enemy_Yankee, L"./Data/Images/yankee.png", { 0,0 }, { 3,4 }, { 96, 128 });
    LoadTextureAsSpriteLeftTop(ResourceKeys::Player_Shot, L"./Data/Images/player_shot.png");
    LoadFont(ResourceKeys::Font_Title, L"./Data/Fonts/Bitcount/static/Bitcount-Light.ttf");
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
