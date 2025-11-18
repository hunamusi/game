// =============================
// Scenes/Result/ResultScene.cpp
// =============================
#include "ResultScene.h"
#include "SceneManager.h"

void ResultScene::Init()
{
    DxLib::SetBackgroundColor(128, 64, 0);
}

void ResultScene::Update()
{
}

void ResultScene::Render() const
{
    gameContext->Draw();

    const int white = DxLib::GetColor(255, 255, 255);
    DxPlus::Text::DrawString(L"Result",
        { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.25f },
        white, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2, 2 }, 0, fontHandle);
}
