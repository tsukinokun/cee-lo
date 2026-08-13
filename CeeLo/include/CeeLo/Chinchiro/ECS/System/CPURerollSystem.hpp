//-------------------------------------------------------------
//! @file   CPURerollSystem.hpp
//! @brief  CPURerollSystemクラスの宣言
//! @author 山﨑愛
//-------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : CeeLo::Chinchiro::ECS
namespace CeeLo::Chinchiro::ECS {

    //-------------------------------------------------------------
    //! @class  CPURerollSystem
    //! @brief  CPUControllerComponent::rerollDelayTimerを消化し、
    //!         0になったらCPU側のサイコロを自動で投げ直すシステム。
    //-------------------------------------------------------------
    class CPURerollSystem : public Tsukino::ECS::ISystem {
    public:
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;
    };

}    // namespace CeeLo::Chinchiro::ECS
