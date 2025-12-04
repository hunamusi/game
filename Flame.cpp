//#include "Flame.h"
//#include"ResourceManager.h"
//#include"ResourceKeys.h"
//#include"Sword.h"
//#include"GameContext.h"
//
//Flame::Flame(const DxPlus::Vec2& emitterPos,float aliveTick)
//{
//	position = emitterPos;
//	AliveTick = aliveTick;
//}
//
//void Flame::Init()
//{
//	sprite = RM().GridAt(ResourceKeys::Fire);
//}
//
//void Flame::Update()
//{
//	frameCounter++;
//
//	if (frameCounter <= lifetimeFrames) 
//	{
//		for (int i = 0; i < emitPerFrame; ++i) 
//		{
//			SpawnParticle(); 
//		} 
//	}
//	// 更新フェーズ - 生存している粒子（Particle）を処理 
//	for (auto& p : particles) 
//	{
//		if (!p.alive) continue;
//
//
//
//		// 移動
//		p.velocity.y -= 0.005f;
//		p.velocity.x *= 0.98f; 
//		p.position += p.velocity;
//
//		// 回転 
//		p.rotation += p.anglarVelocity;
//
//		// 残り寿命を減らす 
//
//		p.remainingLifetime -= 1.0f; 
//		if (p.remainingLifetime <= 0)
//		{
//			// 粒子の寿命が尽きたら非アクティブ化 
//			p.alive = false; 
//			continue; 
//		} 
//
//		// 経過時間の計算 
//		p.normalizedTime = 1.0f -(p.remainingLifetime /(p.startLifetime));
//
//		// サイズと不透明度を計算 
//		p.size = p.startSize * (1.0f - p.normalizedTime);
//		p.alpha = p.startColorAlpha * (1.0f - p.normalizedTime);
//	}
//}
//
//void Flame::Draw() const
//{
//	for (const auto& p : particles) 
//	{
//		if (!p.alive) continue; 
//		DxLib::SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(p.alpha * 255.0f));
//		if (sprite) 
//		{
//			sprite->Draw(p.position, { p.size, p.size }, p.rotation, GetColor(p.r, p.g, p.b)); 
//		}
//	} 
//	DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
//	DxLib::SetDrawBright(255, 255, 255);
//}
//
//void Flame::SpawnParticle()
//{
//	 // 空きスロットを探し出す 
//	 Particle* slot{ nullptr }; 
//	 for (auto& p : particles) 
//	 {
//		 if (!p.alive) 
//		 { slot = &p;
//			 break;
//		 } 
//	 }
//	 if (!slot) return;
//
//
//	 // 発生位置・速度・見た目などを初期化 
//	 slot->position = position + DxPlus::Vec2( RandomRange(-2.0f, 2.0f), RandomRange(-2.0f, 2.0f));
//	 slot->velocity = { RandomRange(-0.6f, 0.6f), -baseUpSpeed + RandomRange(-0.4f, 0.4f) }; 
//
//	 slot->rotation = RandomRange(0.0f, DxPlus::PI * 2.0f); 
//	 slot->anglarVelocity = RandomRange(DxPlus::PI * -0.02f, DxPlus::PI * 0.02f);
//
//	 slot->startSize = RandomRange(0.3f, 0.7f); 
//	 slot->size = slot->startSize; 
//
//	 slot->startColorAlpha = RandomRange(0.2f, 0.5f); 
//	 slot->alpha = slot->startColorAlpha;
//	 
//	 slot-> startLifetime = AliveTick; 
//	 slot->remainingLifetime = slot->startLifetime;
//	 slot->alive = true;
//
//	 // 色分布（白 → ⻩ → 赤）
//	 float choice = RandomRange(0.0f, 1.0f);
//	 if (choice < 0.2f) 
//	 {
//		 // 白（時間は短め） 
//		 slot->r = (unsigned char)RandomRange(220, 255);
//		 slot->g = (unsigned char)RandomRange(220, 255); 
//		 slot->b = (unsigned char)RandomRange(180, 220);
//		 //slot->startLifetime = RandomRange(36.0f, 48.0f); 
//	 }
//	 else if (choice < 0.7f) 
//	 {
//		 // ⻩（時間は中ぐらい） 
//		 slot->r = (unsigned char)RandomRange(200, 255);
//		 slot->g = (unsigned char)RandomRange(100, 200);
//		 slot->b = (unsigned char)RandomRange(0, 40);
//		 //slot->startLifetime = RandomRange(36.0f, 54.0f); 
//	 }
//	 else 
//	 {
//		 // 赤（時間は⻑め） 
//		 slot->r = (unsigned char)RandomRange(150, 220);
//		 slot->g = (unsigned char)RandomRange(0, 60);
//		 slot->b = (unsigned char)RandomRange(0, 30);
//		 //slot->startLifetime = RandomRange(36.0f, 72.0f); 
//	 }
//
//	 slot->remainingLifetime = slot->startLifetime;
//	 slot->alive = true;
//}
//
//float Flame::RandomRange(float from, float to)
//{
//	if (from > to) std::swap(from, to); 
//	const float rate = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//	return from + (to - from) * rate;
//}
//
//bool Flame::IsAlive() const noexcept 
//{
//	if (frameCounter < lifetimeFrames) return true;
//	for (const auto& p : particles)
//	{
//		if (p.alive)
//		{
//			return true;
//		}
//	}
//	return false; 
//}

