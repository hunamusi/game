// =============================
// Scenes/Title/TitleScene.cpp
// =============================
#include "TitleScene.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "ResourceKeys.h"



void TitleScene::Init()
{
    DxLib::SetBackgroundColor(16, 128, 224);
    frameCount = 0;
    game = start;
    fontHandle = RM().GetFont(ResourceKeys::Font_Title);
}

void TitleScene::Update()
{
    using namespace DxPlus::Input;

    // コントローラ／キーボードの押下瞬間を取得
    int btnDown = GetButtonDown(PLAYER1);

    // START でゲーム開始（現在選択が start のとき）
    if ((btnDown & BUTTON_START) != 0 && game == start)
    {
        Scene* gameScene = SceneManager::GetInstance().GetScene(SceneID::Game);
        SetNextScene(gameScene);
        return;
    }

    if ((btnDown & BUTTON_START) != 0 && game == end)
    {
        SM().Shutdown();
        return;
    }

    // 上 (W) 押下で選択を上へ（enum の値を -1）
    if ((btnDown & BUTTON_UP) != 0)
    {
        int v = static_cast<int>(game) - 1;
        if (v < static_cast<int>(Game::start)) v = static_cast<int>(Game::start);
        game = static_cast<Game>(v);
    }

    // 下 (S) 押下で選択を下へ（enum の値を +1）
    if ((btnDown & BUTTON_DOWN) != 0)
    {
        int v = static_cast<int>(game) + 1;
        if (v > static_cast<int>(Game::end)) v = static_cast<int>(Game::end);
        game = static_cast<Game>(v);
    }

    frameCount++;
}

void TitleScene::Render() const
{
    const int white = DxLib::GetColor(255, 255, 255);
    DxPlus::Text::DrawString(L"2D GameProgramming II", 
        { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.25f }, 
        white, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2, 2 }, 0, fontHandle);


    switch (game)
    {
    case TitleScene::start:
    {
        const int yellow1 = DxLib::GetColor(255, 255, 0);
        if (frameCount & 0x20)
        {
            DxPlus::Text::DrawString(L"GameStart",
                { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f  },
                yellow1, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        }
        DxPlus::Text::DrawString(L"Option",
            { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f + 75},
            yellow1, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        DxPlus::Text::DrawString(L"Quit",
            { DxPlus::CLIENT_WIDTH * 0.5f , DxPlus::CLIENT_HEIGHT * 0.75f +150},
            yellow1, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        break;
    }

    case TitleScene::option:
    {
        const int yellow2 = DxLib::GetColor(255, 255, 0);
        if (frameCount & 0x20)
        {
            DxPlus::Text::DrawString(L"Option",
                { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f + 75},
                yellow2, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        }
        DxPlus::Text::DrawString(L"GameStart",
            { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f },
            yellow2, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        DxPlus::Text::DrawString(L"Quit",
            { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f + 150 },
            yellow2, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        break;
    }

    case TitleScene::end:
    {
        const int yellow3 = DxLib::GetColor(255, 255, 0);
        if (frameCount & 0x20)
        {
            DxPlus::Text::DrawString(L"Quit",
                { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f + 150},
                yellow3, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        }
        DxPlus::Text::DrawString(L"GameStart",
            { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f  },
            yellow3, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        DxPlus::Text::DrawString(L"Option",
            { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f + 75 },
            yellow3, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        break;
    }

    default:
        break;
    }

}
