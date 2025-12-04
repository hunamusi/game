#pragma once
#include "Entity2D.h"

class Projectile : public Entity2D
{
public:
	void Init() override;
	void Reset(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel, const DxPlus::Vec2& size) noexcept;
	void Update() override {};
	void Step() override;
	void SetSize(const DxPlus::Vec2& s) noexcept { size = s; }
	const DxPlus::Vec2& GetSize() const noexcept { return size; }
	void CameraDraw(float camX, float camY);

private:
	DxPlus::Vec2 size{ 16.0f, 16.0f };
};
