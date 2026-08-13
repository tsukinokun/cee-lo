//-------------------------------------------------------------
//! @file   ResultInputSystem.cpp
//! @brief  ResultInputSystemクラスの実装
//! @author 山﨑愛
//-------------------------------------------------------------
#include <CeeLo/Chinchiro/ECS/System/ResultInputSystem.hpp>
#include <CeeLo/Chinchiro/ECS/Component/GameStateComponent.hpp>
#include <CeeLo/Scene/ChinchiroScene.hpp>

#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/EngineIntegration/Scene/GameSceneManager.hpp>
#include <Tsukino/Core/Input/InputSystem.hpp>
#include <Tsukino/Core/Input/KeyCodes.hpp>

#include <memory>

// 名前空間 : CeeLo::Chinchiro::ECS
namespace CeeLo::Chinchiro::ECS {

    //-------------------------------------------------------------
    //! @brief  システムの更新
    //-------------------------------------------------------------
    void ResultInputSystem::Update(Tsukino::ECS::Registry& registry, float deltaTime) {
        (void)deltaTime;

        auto* context = registry.GetContext<Tsukino::EngineIntegration::EngineContext*>();
        if(!context || !context->inputSystem || !context->gameSceneManager) {
            return;
        }
        if(!registry.HasContext<GameStateComponent>()) {
            return;
        }

        GameStateComponent& state = registry.GetContext<GameStateComponent>();
        if(state.phase != GamePhase::Result) {
            return;
        }

        if(context->inputSystem->IsKeyPressed(Tsukino::Input::KeyCode::Space)) {
            // シーンを丸ごと読み直して再戦する（個別のリセット処理は行わない）
            context->gameSceneManager->ChangeScene(std::make_unique<CeeLo::ChinchiroScene>());
        }
    }

}    // namespace CeeLo::Chinchiro::ECS
