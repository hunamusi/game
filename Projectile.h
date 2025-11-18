#pragma once
#include"Entity2D.h"
class Projectile :public Entity2D
{
public:
	Projectile()=default;
	void Init()override;
	void Reset(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel)noexcept;
	void Update()override;
	void Step()override;
};