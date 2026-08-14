//-------------------------------------------------------------
//! @file   PlayerComponentSerialization.hpp
//! @brief  PlayerComponentのcerealシリアライズ定義
//! @author 山﨑愛
//-------------------------------------------------------------
#pragma once
#include <CeeLo/Chinchiro/ECS/Component/PlayerComponent.hpp>

#include <cereal/cereal.hpp>

// 名前空間 : CeeLo::Chinchiro::ECS
namespace CeeLo::Chinchiro::ECS {

    //-------------------------------------------------------------
    //! @brief  PlayerComponentのcerealシリアライズ定義
    //-------------------------------------------------------------
    template <class Archive>
    void save(Archive& archive, const PlayerComponent& player) {
        archive(cereal::make_nvp("phase", player.phase),
                cereal::make_nvp("rollCount", player.rollCount),
                cereal::make_nvp("eliminated", player.eliminated),
                cereal::make_nvp("roundEntity", player.roundEntity));
    }

    //-------------------------------------------------------------
    //! @brief  PlayerComponentのcerealデシリアライズ定義
    //-------------------------------------------------------------
    template <class Archive>
    void load(Archive& archive, PlayerComponent& player) {
        archive(player.phase, player.rollCount, player.eliminated, player.roundEntity);
    }

}    // namespace CeeLo::Chinchiro::ECS
