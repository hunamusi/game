// =============================
// Core/GameContext.cpp
// =============================
#include "GameContext.h"
#include "Entity2D.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Player.h"
#include "Enemy.h"
#include "Consts.h"
#include "projectedfslib.h"
#include "Collision2D.h"
#include <algorithm>
void GameContext::Init()
{
	backgroundSpr = RM().GridAt(ResourceKeys::Background);

    entities.emplace_back(std::make_unique<Player>());
    player = static_cast<Player*>(entities.back().get());
    entities.push_back(std::make_unique<Enemy>());
    entities.push_back(std::make_unique<Enemy>());
    entities.push_back(std::make_unique<Enemy>());
    entities.push_back(std::make_unique<Enemy>());
    entities.push_back(std::make_unique<Enemy>());
    projectiles.clear();
    projectiles.reserve(8);
    for (auto& e : entities)
    {
        e->BindContext(this);
        e->Init();
    }
    player->Init();
    camera.SetTarget(player);
}

void GameContext::Reset()
{
    for (auto& e : entities) e->Reset();
    projectiles.clear();
}

void GameContext::Update()
{
    player->Update();
    camera.Update();
    for (auto& e : entities) e->Update();
    for (auto& e : entities) e->Step();
    for (auto& p : projectiles)p->Update();
    for (auto& p : projectiles)p->Step();
    for (auto& p : projectiles)
    {
        if (!p->IsAlive())continue;

        for (auto& e : entities)
        {
            if (!e->IsAlive() ||
                !e->IsDamageable())continue;

            if (Collision2D::CircleVsCircle(
                p->GetPosition(), p->Radius(),
                e->GetPosition() + e->GetCenterOffset(), e->Radius()))
            {
                e->OnHit(p->Attack());
                p->Kill();
                break;
            }
        }
    }
    //存在が消えたEntity2Dをentitiesから削除する
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [](const std::unique_ptr<Entity2D>& p)->bool
            {
                return !p->IsAlive();
            }
        ),
        entities.end()
    );
    //存在が消えたProjectileをprojectilesから削除する
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const std::unique_ptr<Projectile>& p)->bool
            {
                return !p->IsAlive();
            }
        ), projectiles.end()

    );

	std::wstring text = std::wstring(L"Player Position(") +
		std::to_wstring(static_cast<int>(player->GetPosition().x)) +
		L"," +
		std::to_wstring(static_cast<int>(player->GetPosition().y)) +
		L")";
	DxPlus::Debug::SetString(text);
}

void GameContext::Draw() const
{
    float camX = camera.GetX();
    float camY = camera.GetY();
    backgroundSpr->Draw({});
    for(auto&e:entities)
    {
        e->CameraDraw(camX, camY);
    }
    for (auto& p : projectiles)p->Draw();
}

void GameContext::SpawnProjectile(const DxPlus::Vec2& pos, const DxPlus::Vec2& vel) noexcept
{
    if (projectiles.size() >= Const::MAX_PROJECTILES) { return; }
    auto pr = std::make_unique<Projectile>();
    pr->Init();
    pr->Reset(pos, vel);
    projectiles.push_back(std::move(pr));


}


