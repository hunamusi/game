#pragma once
#include "DxPlus/DxPlus.h"
#include "Player.h"
#include "Entity2D.h"

namespace Collision2D {

    /**
     * @brief 2つの円 (Circle) の衝突を判定します。
     * * @param a 1つ目の円の中心座標 (DxPlus::Vec2)
     * @param ra 1つ目の円の半径 (float)
     * @param b 2つ目の円の中心座標 (DxPlus::Vec2)
     * @param rb 2つ目の円の半径 (float)
     * @return bool 衝突していれば true
     */
    inline bool CircleVsCircle(
        const DxPlus::Vec2& a, float ra,
        const DxPlus::Vec2& b, float rb)
    {
        // X座標とY座標の差を計算 (ベクトル A -> B)
        float x = a.x - b.x;
        float y = a.y - b.y;

        // 衝突が起こる閾値 (半径の合計の二乗) を計算
        // 距離が (ra + rb) より小さければ衝突
        float rr = (ra + rb) * (ra + rb);

        // 2点間の距離の二乗を計算 (x^2 + y^2)
        // 速度向上のため、平方根 (sqrt) を使わず、距離の二乗で比較する
        return x * x + y * y < rr;
    }

    /**
     * @brief 2つの矩形 (AABB: 軸並行境界ボックス) の衝突を判定します。
     * * @param aPos 1つ目のAABBの中心座標
     * @param aW 1つ目のAABBの幅
     * @param aH 1つ目のAABBの高さ
     * @param bPos 2つ目のAABBの中心座標
     * @param bW 2つ目のAABBの幅
     * @param bH 2つ目のAABBの高さ
     * @return bool 衝突していれば true
     */
    bool AABB(const DxPlus::Vec2& aPos, float aW, float aH,
        const DxPlus::Vec2& bPos, float bW, float bH)
    {
        // --- 1つ目の矩形 A の境界座標を計算 ---
        // 中心座標から幅/高さの半分を引いたり足したりして、端の座標を求める
        float aLeft = aPos.x - aW / 2;
        float aRight = aPos.x + aW / 2;
        float aTop = aPos.y - aH / 2;
        float aBottom = aPos.y + aH / 2;

        // --- 2つ目の矩形 B の境界座標を計算 ---
        float bLeft = bPos.x - bW / 2;
        float bRight = bPos.x + bW / 2;
        float bTop = bPos.y - bH / 2;
        float bBottom = bPos.y + bH / 2;

        // --- 衝突判定ロジック (軸分離定理の否定) ---
        // 「衝突していない」条件がすべて偽であれば、「衝突している」と判断する。
        // 衝突していない条件 (水平軸):
        // 1. A の右端が B の左端より左にある (aRight < bLeft)
        // 2. A の左端が B の右端より右にある (aLeft > bRight)

        // 衝突している条件 (水平軸):
        // A の左端が B の右端より左にあり (aLeft < bRight),
        // かつ A の右端が B の左端より右にある (aRight > bLeft)

        return (aLeft < bRight && aRight > bLeft &&

            // 衝突している条件 (垂直軸):
            // A の上端が B の下端より上にあり (aTop < bBottom),
            // かつ A の下端が B の上端より下にある (aBottom > bTop)
            aTop < bBottom && aBottom > bTop);
    }