#include "Projectile.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "Player.h"
#include "DxPlus/Sprite.h"
#include "DxLib.h"
#include <cmath>
#include"GameContext.h"

void Projectile::Init()
{
	sprite = RM().GridAt(ResourceKeys::Player_Shot);
}

void Projectile::Reset(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel, const DxPlus::Vec2& s) noexcept
{
	position = pos;
	velocity = vel;
	size = s;
}

void Projectile::Step()
{
	Entity2D::Step();

	if (position.x < 0 || position.x > 3500
		|| position.y < 0 || position.y > 2500)
	{
		Kill();
	}
}

void Projectile::CameraDraw(float camX, float camY)
{
	// ワールド座標 -> スクリーン座標
	DxPlus::Vec2 camPos = position - DxPlus::Vec2{ camX, camY };

	// 判定用のピクセル矩形を計算
	const float halfWf = size.x * 0.5f;
	const float halfHf = size.y * 0.5f;
	const int gx = static_cast<int>(std::round(camPos.x));
	const int gy = static_cast<int>(std::round(camPos.y));
	const int left = gx - static_cast<int>(halfWf);
	const int top = gy - static_cast<int>(halfHf);
	const int right = gx + static_cast<int>(halfWf);
	const int bottom = gy + static_cast<int>(halfHf);

	int Attractiveness = GC().GetAttractiveness();

	if (Attractiveness == 0)
	{
		size = { 32.0f,32.0f };
		
	}
	else if (Attractiveness == 1)
	{
		size = { 64.0f,64.0f };
		camPos.x -= 20;

	}
	else if (Attractiveness <= 2)
	{
		size = { 128.0f,128.0f };
		camPos.x -= 50;
	}

	// スプライトがあれば元画像サイズからスケールを計算して描画
	if (sprite && sprite->IsLoaded()) {
		int w = 0, h = 0;
		DxLib::GetGraphSize(sprite->GetID(), &w, &h);
		DxPlus::Vec2 scale{ 1.0f, 1.0f };
		if (w > 0 && h > 0) {
			scale.x = size.x / static_cast<float>(w);
			scale.y = size.y / static_cast<float>(h);
		}
		sprite->Draw(camPos, scale);
		return;
	}

	// フォールバック：直接拡大描画
	if (graphHandle > 0) {
		DrawExtendGraph(left, top, right, bottom, graphHandle, TRUE);
	}
}
