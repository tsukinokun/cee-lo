//-------------------------------------------------------------
//! @file   RegisterChinchiroComponents.hpp
//! @brief  チンチロ固有コンポーネントをPrefabFactoryへ登録する
//! @author 山﨑愛
//-------------------------------------------------------------
#pragma once
#include <Tsukino/Engine/ECS/Prefab/PrefabFactory.hpp>

#include <CeeLo/Chinchiro/ECS/Component/DiceComponent.hpp>
#include <CeeLo/Chinchiro/ECS/Component/RoundOwnerComponent.hpp>
#include <CeeLo/Chinchiro/ECS/Component/CPUControllerComponent.hpp>
#include <CeeLo/Chinchiro/ECS/Component/RoundComponent.hpp>
#include <CeeLo/Chinchiro/ECS/Component/PlayerComponent.hpp>

#include <CeeLo/Chinchiro/ECS/Serialization/RoundComponentSerialization.hpp>
#include <CeeLo/Chinchiro/ECS/Serialization/PlayerComponentSerialization.hpp>

// 名前空間 : CeeLo::Chinchiro::ECS
namespace CeeLo::Chinchiro::ECS {

    //-------------------------------------------------------------
    //! @brief  チンチロ固有の組み込みコンポーネントを工場に自動登録する
    //! @param  factory [in] 登録先のPrefabFactory
    //-------------------------------------------------------------
    inline void RegisterChinchiroComponents(Tsukino::Engine::ECS::Prefab::PrefabFactory& factory) {
        // 実運用では常にデフォルト値（あるいはInstantiate後にApplyOverrideで
        // 個別設定する）のため、シリアライザは実装せずアタッチのみ対応する。
        factory.RegisterComponent<DiceComponent>("DiceComponent");
        factory.RegisterComponent<RoundOwnerComponent>("RoundOwnerComponent");
        factory.RegisterComponent<CPUControllerComponent>("CPUControllerComponent");

        // dice / roundEntity がEntityRefのため、InstantiateGroupでバッチ生成した
        // 際に自動で参照解決される
        factory.RegisterComponent<RoundComponent>("RoundComponent");
        factory.RegisterComponent<PlayerComponent>("PlayerComponent");
    }

}    // namespace CeeLo::Chinchiro::ECS
