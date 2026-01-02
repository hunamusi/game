// =============================
// Scenes/Base/SceneManager.h
// =============================
#pragma once
#include "Scene.h"
#include "GameContext.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "ResultScene.h"
#include "MenuScene.h"

enum class SceneID { 
    Title
    , Game
    , Result 
    , Menu
};

class SceneManager
{
public:
    static SceneManager& GetInstance()
    {
        static SceneManager instance;
        return instance;
    }

    void Init();
    void Shutdown();
    void Run();
    void SetScene(Scene* newScene);
    Scene* GetScene(SceneID id);

    GameContext& GetGameState() { return gameContext; }

    // コピー禁止
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

private:
    SceneManager() = default;
    ~SceneManager() = default;

    GameContext gameContext;
    TitleScene  titleScene{ &gameContext };
    GameScene   gameScene{ &gameContext };
    ResultScene resultScene{ &gameContext };
    MenuScene menuScene{ &gameContext };

    Scene* scene = nullptr; // 現在のシーン
};
inline SceneManager& SM() { return SceneManager::GetInstance(); } // ショートカット
