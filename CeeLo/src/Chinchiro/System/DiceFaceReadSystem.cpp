//-------------------------------------------------------------
//! @file   DiceFaceReadSystem.cpp
//! @brief  DiceFaceReadSystemクラスの実装
//! @author 山﨑愛
//-------------------------------------------------------------
#include <CeeLo/Chinchiro/ECS/System/DiceFaceReadSystem.hpp>
#include <CeeLo/Chinchiro/ECS/Component/DiceComponent.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/RigidbodyComponent.hpp>
#include <Tsukino/Core/Log.hpp>
#include <string>

// 名前空間 : CeeLo::Chinchiro::ECS
namespace CeeLo::Chinchiro::ECS {
    namespace {
        // ワールド空間での重力方向（上向き判定の基準）
        const hlslpp::float3 kUpDirection = hlslpp::float3(0.0f, 1.0f, 0.0f);
    }

    //-------------------------------------------------------------
    //! @brief  システムの更新
    //-------------------------------------------------------------
    void DiceFaceReadSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        (void)deltaTime;

        auto view = registry.View<Tsukino::BuiltIn::ECS::TransformComponent, Tsukino::BuiltIn::ECS::RigidbodyComponent, DiceComponent>();

        view.each([&](entt::entity, Tsukino::BuiltIn::ECS::TransformComponent& transform,
                       Tsukino::BuiltIn::ECS::RigidbodyComponent& rigidbody, DiceComponent& dice) {
            // 静止確定していないサイコロは何もしない
            if(dice.state != DiceRollState::Settled) {
                return;
            }

            // 位置フリーズ中（Hovering中にDiceDebugOverrideSystemでSettledへ強制された
            // だけのもの等）は、実際の物理姿勢とconfirmedValueが対応していないため、
            // 上書きしてデバッグ機能を壊さないよう対象外にする。
            if(dice.confirmed && rigidbody.freezePositionX) {
                return;
            }

            float bestDot   = -1.0f;
            u8    bestValue = dice.faceValue[0];

            for(int i = 0; i < 6; ++i) {
                // ローカル法線をワールド回転で変換し、上向きベクトルとの内積を見る
                // 注意: hlslpp::mul(quaternion, float3) は逆回転（inverse）を返す。
                // 正しい順方向回転は mul(float3, quaternion)（このプロジェクトの行列mul規約と一致）。
                // 逆にすると、Y軸に沿う面(1,6)は回転方向に依らず値が変わらないため偶然正しく見える一方、
                // X/Z軸の面(2,3,4,5)は誤判定になっていた。
                hlslpp::float3 worldNormal = hlslpp::mul(dice.faceNormal[i], transform.rotation);
                float          dotValue    = hlslpp::dot(worldNormal, kUpDirection).x;

                if(dotValue > bestDot) {
                    bestDot   = dotValue;
                    bestValue = dice.faceValue[i];
                }
            }

            // Settled中は毎フレーム読み直す。まだ転がっている他のサイコロにぶつかられたり、
            // 不安定な着地姿勢からゆっくり傾いたりして、静止確定後にわずかに姿勢が変わることが
            // あり、1回読んだきりだと表示上の出目と実際に見えている目がズレる不具合になっていた。
#ifdef _DEBUG
            bool changed = dice.confirmed && dice.confirmedValue != bestValue;
            if(changed) {
                Tsukino::Core::Log::Info("[ChinchiroScene] Dice face changed after settle: "
                                          + std::to_string(dice.confirmedValue) + " -> " + std::to_string(bestValue));
            }
#endif

            dice.confirmedValue = bestValue;
            dice.confirmed      = true;
        });
    }

}    // namespace CeeLo::Chinchiro::ECS
