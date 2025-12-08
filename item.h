#pragma once
#include "Entity2D.h"
#include "Player.h"
class Item:public Entity2D
{
public:
    Item() = default;
    ~Item() = default;

    void Init();
    void Reset();
    void Update()override;
    
};