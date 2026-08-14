//-------------------------------------------------------------
//! @file    ChinchiroScene.cpp
//! @brief   チンチロゲームの実装
//! @author  山﨑愛
//-------------------------------------------------------------
#include <CeeLo/Scene/ChinchiroScene.hpp>

#include <Tsukino/EngineIntegration/EngineAPI.hpp>
#include <Tsukino/EngineIntegration/EngineContext.hpp>
#include <Tsukino/Engine/Asset/AssetManager.hpp>
#include <Tsukino/Engine/ECS/Prefab/PrefabFactory.hpp>

#include <Tsukino/Core/Path.hpp>
#include <Tsukino/Core/Log.hpp>

// 必要なシステムとコンポーネントのインクルード
#include <Tsukino/EngineIntegration/ECS/System/TransformSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/CameraSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SpriteRendererSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/FontRendererSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/AudioSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/PhysicsSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/ModelSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/AnimationSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/DirectionalLightSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/SkyAtmosphereSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/DebugCameraSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/EffectSystem.hpp>
#include <Tsukino/EngineIntegration/ECS/System/HeightmapGenerationSystem.hpp>

#include <Tsukino/BuiltIn/ECS/Component/TransformComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/CameraComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpriteComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/AudioComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/ModelComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/AnimationPlayerComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SkeletonOutputComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/AnimationControllerComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/SpringBoneComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/EffectComponent.hpp>
#include <Tsukino/BuiltIn/ECS/Component/TerrainGenerationRequestComponent.hpp>

#include <Tsukino/BuiltIn/ECS/Serialization/TransformComponentSerialization.hpp>
#include <Tsukino/BuiltIn/ECS/Serialization/CameraComponentSerialization.hpp>

// チンチロ固有のコンポーネント
#include <CeeLo/Chinchiro/ECS/RegisterChinchiroComponents.hpp>
#include <CeeLo/Chinchiro/ECS/Component/RoundOwnerComponent.hpp>
#include <CeeLo/Chinchiro/ECS/Component/GameStateComponent.hpp>
#include <CeeLo/Chinchiro/ECS/Component/UILabelTags.hpp>
#include <CeeLo/Chinchiro/ECS/Util/DiceThrowUtil.hpp>

// チンチロ固有のシステム
#include <CeeLo/Chinchiro/ECS/System/DiceRestDetectionSystem.hpp>
#include <CeeLo/Chinchiro/ECS/System/DiceFaceReadSystem.hpp>
#include <CeeLo/Chinchiro/ECS/System/HandJudgeSystem.hpp>
#include <CeeLo/Chinchiro/ECS/System/DiceBoundsSystem.hpp>
#include <CeeLo/Chinchiro/ECS/System/DiceRespawnSystem.hpp>
#include <CeeLo/Chinchiro/ECS/System/DiceHoverLimitSystem.hpp>
#include <CeeLo/Chinchiro/ECS/System/TurnRuleSystem.hpp>
#include <CeeLo/Chinchiro/ECS/System/CompareSystem.hpp>
#include <CeeLo/Chinchiro/ECS/System/RollTriggerSystem.hpp>
#include <CeeLo/Chinchiro/ECS/System/CPURerollSystem.hpp>
#include <CeeLo/Chinchiro/ECS/System/ChinchiroUISystem.hpp>
#include <CeeLo/Chinchiro/ECS/System/ResultInputSystem.hpp>
#ifdef _DEBUG
#include <CeeLo/Chinchiro/ECS/System/DiceDebugOverrideSystem.hpp>
#endif

#include <entt/entt.hpp>
#include <hlsl++.h>
#include <array>

// 名前空間 : (無名) チンチロ固有のエンティティ生成ヘルパー
namespace {

    //-------------------------------------------------------------
    //! @brief  お椀エンティティをPrefabから生成する
    //! @param  context             [in] Prefabの組み立てに使うエンジンコンテキスト
    //! @param  registry            [in] ECSレジストリ
    //! @param  bowlModelHandle     [in] お椀の表示用モデルのアセットハンドル
    //! @param  bowlColModelHandle  [in] お椀の地形生成（コリジョン）専用の軽量モデルのアセットハンドル
    //! @param  centerX             [in] お椀の中心X座標（左右の配置に使用）
    //! @param  terrainSeed         [in] 地形生成の乱数シード（左右で同じ形にならないよう変える）
    //! @note   ModelComponent/TerrainGenerationRequestComponentはPrefab上は"null"にしてある。
    //!         AssetManager::Loadはパスごとの重複排除を行わないため、これらは従来通り
    //!         シーン起動時に1回だけLoadしたハンドルをApplyOverrideで注入する
    //!         （Prefab側でAssetRefのパスを直接解決させると、お椀2個・サイコロ6個分
    //!         同じモデルを毎回re-loadしてしまう）。
    //-------------------------------------------------------------
    Tsukino::ECS::Entity CreateBowl(Tsukino::EngineIntegration::EngineContext* context, Tsukino::ECS::Registry& registry,
                                     Tsukino::Asset::AssetHandle bowlModelHandle, Tsukino::Asset::AssetHandle bowlColModelHandle, float centerX,
                                     uint32_t terrainSeed) {
        const std::string prefabPath = "CeeLo/Assets/Prefabs/Bowl/Prefab.json";
        entt::entity      bowlEntity = context->prefabFactory->Instantiate(prefabPath, registry);

        Tsukino::BuiltIn::ECS::TransformComponent transformOverride{};
        transformOverride.position = hlslpp::float3(centerX, 0.0f, 0.0f);
        transformOverride.dirty    = true;    // 初回計算のためフラグを立てる
        context->prefabFactory->ApplyOverride<Tsukino::BuiltIn::ECS::TransformComponent>(registry, bowlEntity, transformOverride);

        Tsukino::BuiltIn::ECS::ModelComponent modelOverride{};
        modelOverride.modelHandle = bowlModelHandle;
        modelOverride.visible     = true;
        context->prefabFactory->ApplyOverride<Tsukino::BuiltIn::ECS::ModelComponent>(registry, bowlEntity, modelOverride);

        Tsukino::BuiltIn::ECS::TerrainGenerationRequestComponent terrainOverride{};
        terrainOverride.amplitude            = 15.0f;
        terrainOverride.noiseFrequency       = 0.08f;
        terrainOverride.seed                 = terrainSeed;
        terrainOverride.noiseType            = Tsukino::BuiltIn::ECS::TerrainNoiseType::Noise;
        terrainOverride.collisionModelHandle = bowlColModelHandle;    // 表示用より軽いメッシュで地形生成する
        context->prefabFactory->ApplyOverride<Tsukino::BuiltIn::ECS::TerrainGenerationRequestComponent>(registry, bowlEntity, terrainOverride);

        return bowlEntity;
    }

    //-------------------------------------------------------------
    //! @brief  CPU/プレイヤー双方のDice(計6個)・Round(2個)・Player(2個)エンティティを、
    //!         EntityRef参照込みで1つのバッチとしてまとめて生成する
    //! @param  context          [in]  Prefabの組み立てに使うエンジンコンテキスト
    //! @param  registry         [in]  ECSレジストリ
    //! @param  diceModelHandle  [in]  サイコロモデルのアセットハンドル
    //! @param  cpuBowlCenter    [in]  CPU側お椀の中心座標（RoundOwnerComponent/場外判定に使用）
    //! @param  playerBowlCenter [in]  プレイヤー側お椀の中心座標（同上）
    //! @param  outCpuEntity     [out] 生成されたCPU側PlayerComponentエンティティ
    //! @param  outPlayerEntity  [out] 生成された人間側PlayerComponentエンティティ
    //-------------------------------------------------------------
    void CreateRoundAndPlayers(Tsukino::EngineIntegration::EngineContext* context, Tsukino::ECS::Registry& registry,
                                Tsukino::Asset::AssetHandle diceModelHandle, const hlslpp::float3& cpuBowlCenter,
                                const hlslpp::float3& playerBowlCenter, Tsukino::ECS::Entity& outCpuEntity, Tsukino::ECS::Entity& outPlayerEntity) {
        using GroupEntry = Tsukino::Engine::ECS::Prefab::PrefabFactory::GroupEntry;

        const std::string dicePrefabPath = "CeeLo/Assets/Prefabs/Dice/Prefab.json";

        // ①Dice6個・Round2個・Player2個を名前付きで一括生成する。
        //   RoundComponent.dice / PlayerComponent.roundEntityはEntityRefのため、
        //   ここでバッチ内の名前（"#CpuDice0"等）が実体へ自動解決される。
        const std::vector<GroupEntry> entries = {
            {"CpuDice0",    dicePrefabPath                                },
            {"CpuDice1",    dicePrefabPath                                },
            {"CpuDice2",    dicePrefabPath                                },
            {"PlayerDice0", dicePrefabPath                                },
            {"PlayerDice1", dicePrefabPath                                },
            {"PlayerDice2", dicePrefabPath                                },
            {"CpuRound",    "CeeLo/Assets/Prefabs/Round/Cpu/Prefab.json"  },
            {"PlayerRound", "CeeLo/Assets/Prefabs/Round/Player/Prefab.json"},
            {"CpuPlayer",   "CeeLo/Assets/Prefabs/Player/Cpu/Prefab.json" },
            {"HumanPlayer", "CeeLo/Assets/Prefabs/Player/Human/Prefab.json"},
        };

        // TODO(temp-verify): remove after manual verification
        std::ofstream dbg2("Tsukino.Sandbox_debug_roundplayer.txt", std::ios::app);
        dbg2 << "checkpoint0: before InstantiateGroup\n";
        dbg2.flush();

        Tsukino::Engine::ECS::Prefab::PrefabFactory::PrefabInstance instance = context->prefabFactory->InstantiateGroup(entries, registry);

        dbg2 << "checkpoint1: after InstantiateGroup, size=" << instance.size() << "\n";
        for(const auto& [name, ent] : instance) {
            dbg2 << "  " << name << " -> " << static_cast<uint32_t>(ent) << "\n";
        }
        dbg2.flush();

        // ②各Diceの個別設定（位置・モデル・所属お椀）はEntityRefでは表現しないデータのため、
        //   ラベル/お椀と同様にInstantiate後にApplyOverrideで設定する。
        const std::array<const char*, 3> cpuDiceNames    = {"CpuDice0", "CpuDice1", "CpuDice2"};
        const std::array<const char*, 3> playerDiceNames = {"PlayerDice0", "PlayerDice1", "PlayerDice2"};

        auto setupDice = [&](const char* name, int index, const hlslpp::float3& bowlCenter) {
            dbg2 << "checkpoint2: setupDice " << name << "\n";
            dbg2.flush();
            entt::entity diceEntity = instance.at(name);

            // 3個ともお椀の中に収まり、かつ重ならないよう、投下待ち位置をX方向に少しずつずらす
            // （リスポン時の位置と一致させるため、ComputeDiceSpawnOffsetを共通で使う）
            Tsukino::BuiltIn::ECS::TransformComponent transformOverride{};
            transformOverride.position = bowlCenter + CeeLo::Chinchiro::ECS::ComputeDiceSpawnOffset(index);
            transformOverride.dirty    = true;    // 初回計算のためフラグを立てる
            context->prefabFactory->ApplyOverride<Tsukino::BuiltIn::ECS::TransformComponent>(registry, diceEntity, transformOverride);

            Tsukino::BuiltIn::ECS::ModelComponent modelOverride{};
            modelOverride.modelHandle = diceModelHandle;
            modelOverride.visible     = true;
            context->prefabFactory->ApplyOverride<Tsukino::BuiltIn::ECS::ModelComponent>(registry, diceEntity, modelOverride);

            // 場外判定の基準座標（所属するお椀の中心）
            CeeLo::Chinchiro::ECS::RoundOwnerComponent ownerOverride{};
            ownerOverride.bowlCenter = bowlCenter;
            context->prefabFactory->ApplyOverride<CeeLo::Chinchiro::ECS::RoundOwnerComponent>(registry, diceEntity, ownerOverride);

            // スペース入力で投下されるまで、空中で静止＋回転しながら待機させる
            CeeLo::Chinchiro::ECS::SetupDiceHover(registry, diceEntity);
        };

        for(int i = 0; i < 3; ++i) {
            setupDice(cpuDiceNames[i], i, cpuBowlCenter);
            setupDice(playerDiceNames[i], i, playerBowlCenter);
        }

        dbg2 << "checkpoint3: before final lookups\n";
        dbg2.flush();

        outCpuEntity    = instance.at("CpuPlayer");
        outPlayerEntity = instance.at("HumanPlayer");

        dbg2 << "checkpoint4: done\n";
        dbg2.flush();
    }

    //-------------------------------------------------------------
    //! @brief  UIラベルエンティティ（TransformComponent + FontComponent）をPrefabから生成する
    //! @param  context        [in] Prefabの組み立てに使うエンジンコンテキスト
    //! @param  registry       [in] ECSレジストリ
    //! @param  screenPosition [in] スクリーン座標（左上原点のピクセル座標）
    //-------------------------------------------------------------
    Tsukino::ECS::Entity CreateLabel(Tsukino::EngineIntegration::EngineContext* context, Tsukino::ECS::Registry& registry, const hlslpp::float3& screenPosition) {
        const std::string prefabPath = "CeeLo/Assets/Prefabs/Label/Prefab.json";
        entt::entity      labelEntity = context->prefabFactory->Instantiate(prefabPath, registry);

        // 画面上の位置はラベルごとに異なるため、インスタンス化後に個別上書きする
        Tsukino::BuiltIn::ECS::TransformComponent transformOverride{};
        transformOverride.position = screenPosition;
        transformOverride.dirty    = true;    // 初回計算のためフラグを立てる
        context->prefabFactory->ApplyOverride<Tsukino::BuiltIn::ECS::TransformComponent>(registry, labelEntity, transformOverride);

        return labelEntity;
    }

}    // namespace

// 名前空間 : CeeLo
namespace CeeLo {
    //-------------------------------------------------------------
    //! @brief  シーン固有の初期化処理
    //-------------------------------------------------------------
    void ChinchiroScene::OnInitialize(Tsukino::EngineIntegration::EngineAPI& api) {
        //-------------------------------------------------------------
        // コンテキストをレジストリから取得
        //-------------------------------------------------------------
        Tsukino::EngineIntegration::EngineContext* context = m_scene.GetRegistry().GetContext<Tsukino::EngineIntegration::EngineContext*>();
        //-------------------------------------------------------------
        // イベントバスをレジストリから取得
        //-------------------------------------------------------------
        Tsukino::ECS::EventBus& eventBus = m_scene.GetEventBus();

        //--------------------------------------------------------------
        // クリアカラーを透明に設定
        //--------------------------------------------------------------
        context->renderer->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        //--------------------------------------------------------------
        // システムの生成と追加
        //--------------------------------------------------------------
        // Transformは一番最初に計算する (優先度 0)
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::TransformSystem>(), 0);
        // アニメーションはTransformの後に更新する (優先度 2)
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::AnimationSystem>(), 2);

        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::HeightmapGenerationSystem>(), 3);

#ifdef _DEBUG
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::DebugCameraSystem>(), 4);
#endif
        // カメラは描画前に更新する (優先度 5)
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::CameraSystem>(), 5);

        // 人間の入力検知（開始・個別振り直し） (優先度 6)
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::RollTriggerSystem>(), 6);
        // CPUの「考え中」タイマー消化・自動振り直し (優先度 7)
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::CPURerollSystem>(), 7);
        // UIラベルのテキスト更新。FontRendererSystemより前に置くこと (優先度 8)
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::ChinchiroUISystem>(), 8);

        // フォント描画 (優先度 9)
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::FontRendererSystem>(), 9);
        // スプライトなど描画用のコマンド生成は後で行う (優先度 10)
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::SpriteRenderSystem>(), 10);
        // モデル描画 (優先度 10)
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::ModelSystem>(), 10);
        // エフェクト描画 (優先度 10)
        {
            auto effectSystem = std::make_shared<Tsukino::BuiltIn::ECS::EffectSystem>();
            m_scene.AddSystem(effectSystem, 10);
            effectSystem->Initialize(m_scene.GetRegistry(), eventBus);
            context->effectSystem = effectSystem.get();
        }
        // オーディオの更新 (優先度 11)
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::AudioSystem>(), 11);
        // コリジョンの更新は最後に行う (優先度 12)
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::PhysicsSystem>(eventBus), 12);
        // 場外に出たサイコロをお椀中心へ戻す (優先度 13)
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::DiceBoundsSystem>(), 13);
        // リスポン中（Kinematicテレポート）のサイコロを、PhysicsSystemがテレポートを
        // 反映した後にHovering（投下待ち）へ引き継ぐ (優先度 13。PhysicsSystemより後段であればよい)
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::DiceRespawnSystem>(), 13);
        // 投下待ち中のサイコロの角速度に上限をかける（待機時間に比例して勢いが
        // 増え続け、着地が乱れて出目誤判定の原因になっていた） (優先度 13)
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::DiceHoverLimitSystem>(), 13);
        // ライトの更新 (優先度 14)
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::DirectionalLightSystem>(), 14);
        // スカイアトモスフィアの更新 (優先度 15)
        m_scene.AddSystem(std::make_shared<Tsukino::BuiltIn::ECS::SkyAtmosphereSystem>(), 15);

        //--------------------------------------------------------------
        // サイコロの静止判定・出目確定・役判定
        //--------------------------------------------------------------
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::DiceRestDetectionSystem>(), 16);
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::DiceFaceReadSystem>(), 17);
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::HandJudgeSystem>(), 18);

        //--------------------------------------------------------------
        // ラウンド進行（目なし/ヒフミの再挑戦・3回失敗、勝敗比較）
        //--------------------------------------------------------------
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::TurnRuleSystem>(), 19);
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::CompareSystem>(), 20);

        // リザルト中のスペース入力でシーンを再読込する (優先度 21)
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::ResultInputSystem>(), 21);

#ifdef _DEBUG
        // 検証用：数字キーで役を強制発生させる（両お椀に同時に適用される。動作確認用）
        m_scene.AddSystem(std::make_shared<CeeLo::Chinchiro::ECS::DiceDebugOverrideSystem>(), 15);
#endif

        //--------------------------------------------------------------
        // アセットのロード
        //--------------------------------------------------------------
        Tsukino::Asset::AssetHandle bowlModelHandle = context->assetManager->Load(Tsukino::Core::Path("CeeLo/Assets/Models/Bowl.fbx"));
        // 地形生成（ヒートフィールドのレイキャストサンプリング）専用の軽量メッシュ。
        // 表示用のBowl.fbxをそのまま使うと三角形数が多く負荷が高いため分離している。
        Tsukino::Asset::AssetHandle bowlColModelHandle =
            context->assetManager->Load(Tsukino::Core::Path("CeeLo/Assets/Models/Bowl_col.fbx"));
        Tsukino::Asset::AssetHandle diceModelHandle = context->assetManager->Load(Tsukino::Core::Path("CeeLo/Assets/Models/Dice.fbx"));

        Tsukino::ECS::Registry& registry = m_scene.GetRegistry();

        //--------------------------------------------------------------
        // チンチロ固有コンポーネントをPrefabFactoryへ登録
        //--------------------------------------------------------------
        CeeLo::Chinchiro::ECS::RegisterChinchiroComponents(*context->prefabFactory);

        //--------------------------------------------------------------
        // お椀・サイコロを左右2セット生成する
        // CPU側 = 左（X-）、プレイヤー側 = 右（X+）
        //--------------------------------------------------------------
        // モデルアセットは実寸に近いスケールで作られている（お椀の実測AABBは約12x12x6ユニット）ため、
        // お椀の間隔もそれに合わせた現実的な値にする。
        // kOutOfBoundsRadius（DiceBoundsSystem.cpp、14.0f）より必ず大きい値にすること
        // （それより狭いと、自分のお椀から場外判定される前に隣のお椀へ到達してしまう）
        constexpr float kBowlOffsetX  = 16.0f;
        constexpr uint32_t kCpuTerrainSeed    = 12345;
        constexpr uint32_t kPlayerTerrainSeed = 54321;    // 左右で同じ地形にならないようシードを変える

        const hlslpp::float3 cpuBowlCenter    = hlslpp::float3(-kBowlOffsetX, 0.0f, 0.0f);
        const hlslpp::float3 playerBowlCenter = hlslpp::float3(kBowlOffsetX, 0.0f, 0.0f);

        CreateBowl(context, registry, bowlModelHandle, bowlColModelHandle, cpuBowlCenter.x, kCpuTerrainSeed);
        CreateBowl(context, registry, bowlModelHandle, bowlColModelHandle, playerBowlCenter.x, kPlayerTerrainSeed);

        //--------------------------------------------------------------
        // Dice・Round・PlayerをEntityRef参照込みで一括生成する
        //--------------------------------------------------------------
        Tsukino::ECS::Entity cpuEntity;
        Tsukino::ECS::Entity playerEntity;
        CreateRoundAndPlayers(context, registry, diceModelHandle, cpuBowlCenter, playerBowlCenter, cpuEntity, playerEntity);

        //--------------------------------------------------------------
        // GameStateComponent：ゲーム全体の進行状態（シングルトン）
        //--------------------------------------------------------------
        CeeLo::Chinchiro::ECS::GameStateComponent& gameState = registry.SetContext<CeeLo::Chinchiro::ECS::GameStateComponent>();
        gameState.phase                                             = CeeLo::Chinchiro::ECS::GamePhase::Ready;
        gameState.outcome                                           = CeeLo::Chinchiro::ECS::RoundOutcome::None;
        gameState.player                                            = playerEntity;
        gameState.cpu                                                = cpuEntity;

        //--------------------------------------------------------------
        // UIラベルエンティティの生成（画面は1700x1000を想定した暫定配置）
        //--------------------------------------------------------------
        {
            Tsukino::ECS::Entity cpuLabelEntity = CreateLabel(context, registry, hlslpp::float3(1350.0f, 40.0f, 0.0f));
            registry.AddComponent<CeeLo::Chinchiro::ECS::CpuHandLabelTag>(cpuLabelEntity);

            Tsukino::ECS::Entity playerLabelEntity = CreateLabel(context, registry, hlslpp::float3(40.0f, 40.0f, 0.0f));
            registry.AddComponent<CeeLo::Chinchiro::ECS::PlayerHandLabelTag>(playerLabelEntity);

            Tsukino::ECS::Entity messageLabelEntity = CreateLabel(context, registry, hlslpp::float3(550.0f, 900.0f, 0.0f));
            registry.AddComponent<CeeLo::Chinchiro::ECS::MessageLabelTag>(messageLabelEntity);
        }

        //--------------------------------------------------------------
        // 2Dカメラエンティティの生成
        //--------------------------------------------------------------
        {
            const std::string prefabPath  = "CeeLo/Assets/Prefabs/Camera2D/Prefab.json";
            entt::entity      cameraEntity2D = context->prefabFactory->Instantiate(prefabPath, registry);
        }

        //--------------------------------------------------------------
        // 3Dカメラエンティティの生成（左右両方のお椀が画角に収まる引き位置は
        // Assets/Prefabs/3DCamera 側で設定済み）
        //--------------------------------------------------------------
        {
            const std::string prefabPath = "CeeLo/Assets/Prefabs/3DCamera/Prefab.json";

            entt::entity testEntity = context->prefabFactory->Instantiate(prefabPath, registry);
        }

        //--------------------------------------------------------------
        // デバッグカメラエンティティの生成 (デバッグビルドのみ)
        //--------------------------------------------------------------
#ifdef _DEBUG
        {
            const std::string prefabPath    = "CeeLo/Assets/Prefabs/DebugCamera/Prefab.json";
            entt::entity      debugCamEntity = context->prefabFactory->Instantiate(prefabPath, registry);
        }
#endif

        //--------------------------------------------------------------
        // ディレクショナルライトエンティティの生成
        //--------------------------------------------------------------
        {
            const std::string prefabPath  = "CeeLo/Assets/Prefabs/DirectionalLight/Prefab.json";
            entt::entity      lightEntity = context->prefabFactory->Instantiate(prefabPath, registry);
        }

        //--------------------------------------------------------------
        // スカイアトモスフィアエンティティの生成
        //--------------------------------------------------------------
        {
            const std::string prefabPath = "CeeLo/Assets/Prefabs/Sky/Prefab.json";
            entt::entity      skyEntity  = context->prefabFactory->Instantiate(prefabPath, registry);
        }
    }

    //-------------------------------------------------------------
    //! @brief  シーンの更新
    //-------------------------------------------------------------
    void ChinchiroScene::OnUpdate(Tsukino::EngineIntegration::EngineAPI& api, float deltaTime) {
        m_scene.Update(deltaTime);
    }

    //-------------------------------------------------------------
    //! @brief  シーンの終了処理
    //-------------------------------------------------------------
    void ChinchiroScene::OnExit() {
        // シーン終了時の解放処理などが必要な場合はここに記述します
    }

}    // namespace CeeLo
