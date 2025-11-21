#include "Sword.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Player.h"

void Sword::Init()
{
	sprite = RM().GridAt(ResourceKeys::Player_Sword);
}

void Sword::Reset()
{
	t = 0.0f;
	UpdateMotion();
}

void Sword::Update()
{
	UpdateMotion();
}

void Sword::UpdateMotion()
{
	if (!player->IsSwordSwinging())
	{
		alive = false;
		return;
	}

	float add = 1.0f / static_cast<float>(Const::SWORD_DURATION);
	
	t = std::min(t + add, 1.0f);

	const int sign = player->facingSign();

	position = player->GetPosition() + player->GetCenterOffset()
		+ DxPlus::Vec2(12.0f * sign, 0.0f);

	float localDeg =  -60.0f + 300 * t;

	localDeg = std::min(localDeg, 120.0f) * sign;

	DxPlus::Debug::SetFormatString(L"localDeg : %g", localDeg);

	rotation = DxPlus::Deg2Rad * localDeg;
}
