----------------------------------------
-- CeeLo ワークスペース
----------------------------------------
workspace "CeeLo"
    architecture "x64"
    configurations { "Debug", "Release" }

    startproject "CeeLo"
    location ".build"
    multiprocessorcompile "On"
    exceptionhandling "On"

    -- TsukinoEngine本体のworkspace設定と同一にする
    -- (サブモジュール側のworkspace宣言はIS_SAME_AS_ROOTガードでスキップされるため、
    --  ここで肩代わりする必要がある)
    filter "configurations:*"
        defines { "JPH_DEBUG_RENDERER" } -- 値は1でなくても定義されていることが重要
    filter {}

    filter "configurations:Debug"
        optimize "Off"
        symbols "On"

    filter "action:vs*"
        buildoptions { "/utf-8" }
    filter {}

    filter "configurations:Release"
        optimize "Full"
        symbols "On"
    filter {}

    filter "configurations:*"
        linkoptions { "/IGNORE:4006" }
    filter {}

----------------------------------------
-- TsukinoEngine を「エンジン部分だけ」取り込む
-- (workspace宣言とTsukino.Sandboxはpremake5.lua側のガードでスキップされる)
----------------------------------------
include "External/TsukinoEngine/premake5.lua"

----------------------------------------
-- CeeLo 本体（実行ファイル）
----------------------------------------
project "CeeLo"
    location ".build/CeeLo"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"
    forceincludes { "pch.h" }               -- 強制インクルード

    filter "action:vs*"
        buildoptions { "/permissive-" }
    filter {}

    pchheader "pch.h"
    pchsource "CeeLo/pch.cpp"

    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.buildcfg}")

    -- Debug: workspaceルート(=cee-loルート)を作業ディレクトリにし、Assetsを直接参照する
    filter "configurations:Debug"
        debugdir "%{wks.location}/.."
    filter {}

    -- Release: exeの隣にAssetsをコピーして配布可能にする
    -- (Tsukino.Engine/Tsukino.BuiltInのRelease時postbuildは各ライブラリ自身のtargetdir
    --  (External/TsukinoEngine/bin/Release)へコピーするため、CeeLoのtargetdirとは別になる。
    --  CeeLo.exeが単体で動くようTools/Tsukino.BuiltIn/AssetsもここでCeeLo側へ複製する)
    filter "configurations:Release"
        debugdir "%{cfg.targetdir}"
        postbuildcommands {
            "{COPYDIR} %{wks.location}/../CeeLo/Assets %{cfg.targetdir}/CeeLo/Assets",
            "{COPYDIR} %{wks.location}/../External/TsukinoEngine/Tools %{cfg.targetdir}/Tools",
            "{COPYDIR} %{wks.location}/../External/TsukinoEngine/Tsukino.BuiltIn/Assets %{cfg.targetdir}/Tsukino.BuiltIn/Assets",
        }
    filter {}

    files {
        "CeeLo/src/**.cpp",
        "CeeLo/include/**.hpp",
        "CeeLo/pch.cpp",
    }

    includedirs {
        "CeeLo/include",
        "External/TsukinoEngine/Tsukino.Audio/include",
        "External/TsukinoEngine/Tsukino.GraphicsCommon/include",
        "External/TsukinoEngine/Tsukino.Engine/include",
        "External/TsukinoEngine/Tsukino.Renderer/include",
        "External/TsukinoEngine/Tsukino.BuiltIn/include",
        "External/TsukinoEngine/Tsukino.EngineIntegration/include",
        "External/TsukinoEngine/Tsukino.Core/include",
        "External/TsukinoEngine/External/cereal/include",
        "External/TsukinoEngine/External/hlslpp/include",
        "External/TsukinoEngine/External/entt/single_include",
        "External/TsukinoEngine/External/JoltPhysics",
        "External/TsukinoEngine/External/Effekseer/Dev/Cpp",
        "External/TsukinoEngine/External/Effekseer/Dev/Cpp/Effekseer",
        "External/TsukinoEngine/External/Effekseer/Dev/Cpp/EffekseerRendererDX11",
        "External/TsukinoEngine/External/Effekseer/Dev/Cpp/EffekseerRendererCommon",
        "External/TsukinoEngine/External/Effekseer/Dev/Cpp/3rdParty",
    }

    links {
        "Tsukino.Engine",
        "Tsukino.Renderer",
        "Tsukino.GraphicsCommon",
        "Tsukino.Audio",
        "Tsukino.BuiltIn",
        "Tsukino.EngineIntegration",
        "JoltPhysics",
        "Tsukino.Core",
        "EffekseerRendererDX11",
        "EffekseerRendererCommon",
        "Effekseer",
        "d3d11",
        "dxgi",
        "d3dcompiler",
    }

    nuget {
        "directxtk_desktop_win10:2026.4.1.1",
        "AssimpCpp:5.0.1.6",
    }
