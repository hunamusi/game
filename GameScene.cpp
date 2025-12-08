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
      


    gameContext->Reset();
}

void GameScene::Update()
{
    gameContext->Update();

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
