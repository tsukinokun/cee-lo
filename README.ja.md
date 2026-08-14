# Cee-lo (チンチロ)

![ゲームプレイ画面](img/image.png)

🇬🇧 [English version here / 英語版はこちら](README.md)

自作のC++ / DirectX 11ゲームエンジン **TsukinoEngine** を使って作られたサイコロゲームです。

## 概要

Cee-lo（欧米での呼び名。日本では「チンチロ」）は、プレイヤー1人 vs CPU1人で対戦する2人用のサイコロゲームです。それぞれが自分の壺を持ち、サイコロ3個を同時に振ります。チップ（賭け金）制度はなく、1本勝負。<kbd>Space</kbd>キーでシーンをリロードして再戦できます。

本プロジェクトは、自作エンジン [TsukinoEngine](https://github.com/tsukinokun/TsukinoEngine) のショーケース/デモゲームも兼ねています。

## ゲームルール

役ができるまで最大3回まで振り直しが可能で、3回とも役が出せなければ自動的に負けとなります。役の強さ（強い順）は以下の通りです。

| 役 | 出目 | 説明 |
|---|---|---|
| ピンゾロ | 1-1-1 | 最強の役 |
| アラシ | ゾロ目（2-2-2〜6-6-6） | 1以外のゾロ目 |
| シゴロ | 4-5-6 | 決まった順目 |
| 目 | ペア＋残り1個 | 残り1個の出目の大小で強さが決まる |
| 目なし | 上記以外 | 負け／振り直し対象 |

## 技術スタック

- **言語:** C++20
- **ビルドシステム:** [Premake5](https://premake.github.io/)
- **グラフィックス:** DirectX 11
- **ECS:** [EnTT](https://github.com/skypjack/entt)
- **物理演算:** [Jolt Physics](https://github.com/jrouwe/JoltPhysics)
- **対応プラットフォーム:** Windows (x64) のみ
- NuGetパッケージ: `directxtk_desktop_win10`, `AssimpCpp`

## プロジェクト構成

```
CeeLo/                    ゲーム本体プロジェクト
├─ src/Scene/              メインゲームシーン (ChinchiroScene)
├─ src/Chinchiro/System/   ECSシステム群（サイコロ判定、役判定、CPU AI、ターン制御 など）
├─ Assets/Models/          壺・サイコロのモデル
└─ Assets/Prefabs/         JSON形式のプレハブ（カメラ、ライト、サイコロ、プレイヤー など）

External/TsukinoEngine/   自作ゲームエンジン (gitサブモジュール)
```

## セットアップ

### 前提条件

- Windows
- Visual Studio 2022
- DirectX 11対応GPU

### ビルド & 実行

1. サブモジュールを含めてクローンします。
   ```
   git clone --recurse-submodules https://github.com/tsukinokun/cee-lo.git
   ```
   （すでに`--recurse-submodules`なしでクローン済みの場合は `git submodule update --init --recursive` を実行してください。）
2. リポジトリルートで `open.bat` を実行します。Premake5がVisual Studio 2022用のソリューション（`.build/CeeLo.sln`）を生成し、自動的に開きます。
3. Visual Studio上で `CeeLo` をスタートアッププロジェクトとしてビルド・実行します（NuGetパッケージは自動で復元されます）。

## 作者

**つきの**（山﨑 愛）

- GitHub: [@tsukinokun](https://github.com/tsukinokun)
- Qiita: [@tsukino_](https://qiita.com/tsukino_)
