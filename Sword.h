#pragma once
#include"Entity2D.h"

class Player;
class Sword : public Entity2D
{
public:
	Sword(Player* owner) : player(owner) {}
	void Init()override;
	void Reset()override;
	void Update() override;
	void UpdateMotion();

private:

	Player* player = nullptr;
	float t = 0.0f;


};