// =============================
// Scenes/Game/GameScene.cpp
// =============================
#include "GameScene.h"
#include "SceneManager.h"

void GameScene::Init()
{
    DxLib::SetBackgroundColor(64, 64, 128);
    gameContext->Reset();
}

void GameScene::Update()
{
    gameContext->Update();
}

void GameScene::Render() const
{
    gameContext->Draw();
}
