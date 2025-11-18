// =============================
// Gameplay/Base/Entity2D.h
// =============================
#pragma once
#include "DxPlus/DxPlus.h"
#include "AnimationClip.h"
#include "Consts.h"
#include "DxLib.h"
// 2D エンティティ（スプライト、位置、速度を保持）
class GameContext;
class Entity2D
{
public:
	Entity2D() = default;
	virtual ~Entity2D() = default;

	// 位置の取得／設定
	const DxPlus::Vec2& GetPosition() const { return position; }
	void SetPosition(const DxPlus::Vec2& p) { position = p; }

	// ライフサイクル処理
    virtual void Init() {}
    virtual void Reset() {}
    virtual void Update() = 0;
    virtual void Step() { position += velocity; }
	virtual void CameraDraw(float camX, float camY)
	{
		DxPlus::Vec2 camPos = position - DxPlus::Vec2{ camX, camY };

		if (currentAnim) currentAnim->Draw(camPos);
		else if (sprite) sprite->Draw(camPos);
		else if (graphHandle > 0) DrawGraph((int)camPos.x, (int)camPos.y, graphHandle, TRUE);
	}
    virtual void Draw() const
    {
        if (currentAnim) currentAnim->Draw(position);
        else if (sprite) sprite->Draw(position);
    }
	// 今回つくる関数はここに書いていけばよい
	void BindContext(GameContext*g)noexcept{gc=g;}
	[[nodiscard]] DxPlus::Vec2 GetCenterOffset() const noexcept { return centerOffset; }
	[[nodiscard]] bool IsAlive()const noexcept { return alive; }
	void Kill()noexcept { alive = false; }
	[[nodiscard]] virtual bool IsDamageable()const noexcept { return false; }
	[[nodiscard]] float Radius()const noexcept { return colliderRadius; }
	virtual void OnHit(int/*atk*/)noexcept{}
	[[nodiscard]] int Attack()const noexcept { return attack; }
protected:
	const DxPlus::Sprite::SpriteBase* sprite{ nullptr };
	DxPlus::Vec2 position{};
	DxPlus::Vec2 velocity{};
    // アニメーション関連
    AnimationClip animLeft{}, animRight{}, animUp{}, animDown{};
	AnimationClip* currentAnim{ nullptr };
	DxPlus::Vec2 centerOffset{ Const::PLAYER_CENTER_OFFSET };
	GameContext* gc{ nullptr };
	GameContext& GC()const noexcept { return*gc; }
	bool alive = true;
	float colliderRadius{ 32.0f };
	int hp{ 3 };
	int attack{ 1 };
	int graphHandle;


};
