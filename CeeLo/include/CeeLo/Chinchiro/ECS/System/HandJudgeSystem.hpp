//-------------------------------------------------------------
//! @file   HandJudgeSystem.hpp
//! @brief  HandJudgeSystemクラスの宣言
//! @author 山﨑愛
//-------------------------------------------------------------
#pragma once
#include <Tsukino/Core/ECS/System/ISystem.hpp>

// 名前空間 : CeeLo::Chinchiro::ECS
namespace CeeLo::Chinchiro::ECS {

    //-------------------------------------------------------------
    //! @class  HandJudgeSystem
    //! @brief  RoundComponentが持つ3つのサイコロの出目から役を判定するシステム
    //! @details 3つ全てのDiceComponent::confirmedがtrueになったら役を判定し、
    //!          RoundComponent::judged を true にする。
    //!          DiceFaceReadSystem より後に登録すること。
    //-------------------------------------------------------------
    class HandJudgeSystem : public Tsukino::ECS::ISystem {
    public:
        void Update(Tsukino::ECS::Registry& registry, float deltaTime) override;
    };

}    // namespace CeeLo::Chinchiro::ECS
