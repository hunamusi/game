#include "MenuScene.h"
#include "SceneManager.h"

void MenuScene::Init()
{
}

void MenuScene::Update()
{
    using namespace DxPlus::Input;

    // コントローラ／キーボードの押下瞬間を取得
    int btnDownM = GetButtonDown(PLAYER1);

    // START でゲーム開始（現在選択が start のとき）
    if ((btnDownM & BUTTON_START) != 0 && menu == start)
    {
        Scene* gameScene = SceneManager::GetInstance().GetScene(SceneID::Game);
        SetNextScene(gameScene);
        return;
    }

    if ((btnDownM & BUTTON_START) != 0 && menu == end)
    {
        SM().Shutdown();
        return;
    }

    // 上 (W) 押下で選択を上へ（enum の値を -1）
    if ((btnDownM & BUTTON_UP) != 0)
    {
        int v = static_cast<int>(menu) - 1;
        if (v < static_cast<int>(Menu::start)) v = static_cast<int>(Menu::start);
        menu = static_cast<Menu>(v);
    }

    // 下 (S) 押下で選択を下へ（enum の値を +1）
    if ((btnDownM & BUTTON_DOWN) != 0)
    {
        int v = static_cast<int>(menu) + 1;
        if (v > static_cast<int>(Menu::end)) v = static_cast<int>(Menu::end);
        menu = static_cast<Menu>(v);
    }

    frameCount++;
}
void MenuScene::Render() const
{

    gameContext->Draw();

    const int white = DxLib::GetColor(255, 255, 255);
    DxPlus::Text::DrawString(L"Menu",
        { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.25f },
        white, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2, 2 }, 0, fontHandle);

    switch (menu)
    {
    case MenuScene::start:
    {
        const int yellow1 = DxLib::GetColor(255, 255, 0);
        if (frameCount & 0x20)
        {
            DxPlus::Text::DrawString(L"GameReStart",
                { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f },
                yellow1, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        }
        DxPlus::Text::DrawString(L"Option",
            { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f + 75 },
            yellow1, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        DxPlus::Text::DrawString(L"Quit",
            { DxPlus::CLIENT_WIDTH * 0.5f , DxPlus::CLIENT_HEIGHT * 0.75f + 150 },
            yellow1, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        break;
    }

    case MenuScene::option:
    {
        const int yellow2 = DxLib::GetColor(255, 255, 0);
        if (frameCount & 0x20)
        {
            DxPlus::Text::DrawString(L"Option",
                { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f + 75 },
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

    case MenuScene::end:
    {
        const int yellow3 = DxLib::GetColor(255, 255, 0);
        if (frameCount & 0x20)
        {
            DxPlus::Text::DrawString(L"Quit",
                { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f + 150 },
                yellow3, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2,2 }, 0, fontHandle);
        }
        DxPlus::Text::DrawString(L"GameStart",
            { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f },
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
