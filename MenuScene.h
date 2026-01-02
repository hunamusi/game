#pragma once
#include "Scene.h"

class MenuScene final : public Scene
{
public:
    explicit MenuScene(class GameContext* context) : Scene(context) {}
    void Init() override;
    void Update() override;
    void Render() const override;

private:
    int fontHandle{ -1 };
    int frameCount{ 0 };

    enum Menu
    {
        no = -1,
        start,
        option,
        end
    };

    Menu menu;
};

