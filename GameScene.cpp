// =============================
// Scenes/Game/GameScene.cpp
// =============================
#include "GameScene.h"
#include "SceneManager.h"
#include "DxPlus/DxPlus.h"

extern int PlayerWalkCountALL;


void GameScene::Init()
{
    DxLib::SetBackgroundColor(64, 64, 128);


    if (!initialized) {
        gameContext->Reset(); // 初回のみ
        initialized = true;
    }
}

void GameScene::Update()
{
    gameContext->Update();

    using namespace DxPlus::Input;

    // コントローラ／キーボードの押下瞬間を取得
    int btnDown = GetButtonDown(PLAYER1);

    if ((btnDown & BUTTON_START) != 0)
    {
        Scene* gameScene = SceneManager::GetInstance().GetScene(SceneID::Menu);
        SetNextScene(gameScene);
        return;
    }

    if (PlayerWalkCountALL >= Const::TURN_MAX)
    {
        Scene* ResultScene = SceneManager::GetInstance().GetScene(SceneID::Result);
        SetNextScene(ResultScene);
    }
}

void GameScene::Render() const
{
    gameContext->Draw();
}
