//#pragma once
//#include "Entity2D.h"
//
//class Flame : public Entity2D
//{
//public:
//	Flame(const DxPlus::Vec2& emitterPos, float aliveTick);
//	void Init() override; 
//	void Update() override;
//	void Draw() const override;
//	void SpawnParticle();
//	float RandomRange(float from, float to);
//	[[nodiscard]] bool IsAlive() const noexcept override;
//
//private:
//	float AliveTick;
//	struct Particle
//	{
//		DxPlus::Vec2 position{}; // ワールド上の現在位置 
//		DxPlus::Vec2 velocity{}; // 毎フレームの移動速度ベクトル 
//
//		float rotation{}; // 回転角度（ラジアン） 
//		float anglarVelocity{ DxPlus::Deg2Rad * 6 }; // 回転速度（ラジアン / フレーム）
//
//		float startSize{ 1.0f }; // 生成時のサイズ（基準スケール） 
//		float size{ 1.0f }; // 現在のサイズ（拡大・縮小後） 
//
//		float startLifetime; // 寿命の初期値（生成時に設定されるフレーム数） 
//		float remainingLifetime; // 残り寿命（フレーム数）
//		float normalizedTime{ 0.0f }; // 経過時間の割合（0.0f 〜 1.0f）
//		
//		float startColorAlpha{ 1.0f }; // 生成時の不透明度（アルファ値） 
//		float alpha{ 1.0f }; // 現在の不透明度（描画時に使用される）
//		unsigned char r{ 255 }; // 現在の色の赤成分（0 〜 255） 
//		unsigned char g{ 255 }; // 現在の色の緑成分（0 〜 255）
//		unsigned char b{ 255 }; // 現在の色の⻘成分（0 〜 255） 
//		
//		bool alive{ false }; // パーティクルが生存中かどうか（更新・描画対象か）
//	};
//	static constexpr int CAPACITY{ 256 }; // 最大粒子数 
//	static constexpr int MAX_EMIT_PER_FRAME{ 8 }; // 1 フレームの最大生成数
//
//	std::array<Particle, CAPACITY> particles;// Particle 型で要素数が CAPACITY である配列 
//	int emitPerFrame{ 4 }; // 1 フレームに生成する粒子数 
//	float baseUpSpeed{ 2.4f }; // 上昇の初速
//
//
//	int lifetimeFrames{ 5 * 60 };
//	int frameCounter{ 0 };
//
//};
//

