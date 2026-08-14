//-------------------------------------------------------------
//! @file   RoundComponentSerialization.hpp
//! @brief  RoundComponentのcerealシリアライズ定義
//! @author 山﨑愛
//-------------------------------------------------------------
#pragma once
#include <CeeLo/Chinchiro/ECS/Component/RoundComponent.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>

// 名前空間 : CeeLo::Chinchiro::ECS
namespace CeeLo::Chinchiro::ECS {

    //-------------------------------------------------------------
    //! @brief  RoundComponentのcerealシリアライズ定義
    //-------------------------------------------------------------
    template <class Archive>
    void save(Archive& archive, const RoundComponent& round) {
        // kind/subValue/judgedは判定結果のランタイム状態のため保存しない
        archive(cereal::make_nvp("dice", round.dice));
    }

    //-------------------------------------------------------------
    //! @brief  RoundComponentのcerealデシリアライズ定義
    //-------------------------------------------------------------
    template <class Archive>
    void load(Archive& archive, RoundComponent& round) {
        archive(round.dice);
        // ロード直後は必ず未判定状態にする
        round.kind     = Hand::None;
        round.subValue = 0;
        round.judged   = false;
    }

}    // namespace CeeLo::Chinchiro::ECS
