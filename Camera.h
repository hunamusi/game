#pragma once
#include"DxPlus/DxPlus.h"
#include"Entity2D.h"     
#include "GameContext.h" 
#include <algorithm>

class Camera
{
public:
    // 追跡対象となるEntity2Dを設定します。
    void SetTarget(Entity2D* t)
    {
        target = t; // 追跡対象をセット
        // ターゲットが設定された場合、カメラの位置をターゲットの初期位置に即座に合わせます。
        if (target)
        {
            x = target->GetPosition().x;
            y = target->GetPosition().y;
        }
    }
    // カメラの状態を更新します（主にターゲットの追尾処理）。
    void Update()
    {
        if (!target) return; // 追跡対象がなければ何もしません。

        auto pos = target->GetPosition(); // ターゲットの現在の位置を取得
        // 最初の更新時のみ、カメラをターゲットの位置に瞬間移動させます（スムーズな追尾を開始しないため）。
        if (firstUpdate)
        {
            x = pos.x;
            y = pos.y;
            firstUpdate = false;
        }
        else
        {
            // スムーズな追尾処理（ラグ効果）
            // (pos.x - x) はターゲットとカメラの現在のX座標の差分（移動量）です。
            // これに followSpeed (0.1f) を掛けることで、差分の10%だけカメラをターゲットに近づけます。
            // これにより、カメラは常にターゲットを追いかけますが、少し遅れて追尾する効果が得られます。
            x += (pos.x - x) * followSpeed;
            y += (pos.y - y) * followSpeed;
        }
        float halfW = 2000 / 2.5f;
        float halfH = 720 / 2.5f;
        x = std::clamp(x, halfW, 3000 - halfW);
        y = std::clamp(y, halfH, 720 - halfH);

    }
    // カメラの**視点（左上隅）**のX座標を返します。
    // カメラの中心座標 x から、クライアント（画面）幅の半分を引くことで、
    // 画面の左端がどのワールド座標に来るかを計算しています。
    float GetX() const { return x - DxPlus::CLIENT_WIDTH / 2; }
    // カメラの**視点（左上隅）**のY座標を返します。
    // カメラの中心座標 y から、クライアント（画面）高さの半分を引くことで、
    // 画面の上端がどのワールド座標に来るかを計算しています。
    float GetY() const { return y - DxPlus::CLIENT_HEIGHT / 2; }
private:
    // カメラの中心のワールド座標（X軸）。
    float x = 0;
    // カメラの中心のワールド座標（Y軸）。
    float y = 0;
    // 追跡対象のエンティティへのポインタ。
    Entity2D* target = nullptr;
    // 追尾の速度を制御する係数（0.0f < followSpeed < 1.0f）。
    // 値が小さいほど、追尾が遅くスムーズになります。
    const float followSpeed = 0.1f;
    // 初回アップデートかどうかを判別するフラグ。
    bool firstUpdate = true;

};