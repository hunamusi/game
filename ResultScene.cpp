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
    // Enter（Return）でタイトルへ戻り、起動時の数値へリセット
    if (DxLib::CheckHitKey(KEY_INPUT_RETURN))
    {
        // シーン切り替え（タイトルへ）
        Scene* gameScene = SceneManager::GetInstance().GetScene(SceneID::Title);
        SetNextScene(gameScene);
    }
}
void ResultScene::Render() const
{

    const int white = DxLib::GetColor(255, 255, 255);
    DxPlus::Text::DrawString(L"Result",
        { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.25f },
        white, DxPlus::Text::TextAlign::MIDDLE_CENTER, { 2, 2 }, 0, fontHandle);
}
