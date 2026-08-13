//-------------------------------------------------------------
//! @file   DiceRespawnSystem.cpp
//! @brief  DiceRespawnSystemクラスの実装
//! @author 山﨑愛
//-------------------------------------------------------------
#include <CeeLo/Chinchiro/ECS/System/DiceRespawnSystem.hpp>
#include <CeeLo/Chinchiro/ECS/Component/DiceComponent.hpp>
#include <CeeLo/Chinchiro/ECS/Util/DiceThrowUtil.hpp>

#include <Tsukino/BuiltIn/ECS/Component/RigidbodyComponent.hpp>

// 名前空間 : CeeLo::Chinchiro::ECS
namespace CeeLo::Chinchiro::ECS {

    //-------------------------------------------------------------
    //! @brief  システムの更新
    //-------------------------------------------------------------
    void DiceRespawnSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        (void)deltaTime;

        auto view = registry.View<DiceComponent, Tsukino::BuiltIn::ECS::RigidbodyComponent>();

        view.each([&](entt::entity entity, DiceComponent& dice, Tsukino::BuiltIn::ECS::RigidbodyComponent& rigidbody) {
            // Respawning以外は対象外。RespawnDiceSetが要求したテレポートを
            // PhysicsSystemが（1フレーム前に）反映済みのサイコロのみここで拾う
            if(dice.state != DiceRollState::Respawning) {
                return;
            }

            // Kinematicテレポートは完了したので、通常の投下待ち（Dynamic＋位置フリーズ）へ引き継ぐ
            rigidbody.type        = Tsukino::BuiltIn::ECS::RigidbodyType::Dynamic;
            rigidbody.isTypeDirty = true;

            SetupDiceHover(registry, entity);
        });
    }

}    // namespace CeeLo::Chinchiro::ECS
