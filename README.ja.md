[🇬🇧 English](README.md)

# MIDITrail Mod

MIDITrailは、MIDIデータを三次元可視化するMIDIプレーヤーです。
標準MIDIファイルFormat0/1、および複数ポート出力に対応しています。

本リポジトリは[和田雅志氏のMIDITrail](https://www.yknk.org/miditrail/)を
ベースにした **yossiepon Mod** です。波紋／鍵盤演出向上・鍵盤方向自動切替・
多ポート鍵盤表示・歌詞対応等の機能を追加しています。

## 動作環境

- OS: Windows 10 / 11
- VIDEO: DirectX 11 に対応したグラフィックチップが必要です

スムーズな画面表示のため、高性能グラフィックチップが搭載されたPCの使用を推奨します。
MIDIデータに含まれるノート数が多くなるほど、より高い性能が必要になります。

## クイックスタート

1. 標準MIDIファイル(\*.mid)をウィンドウにドラッグ＆ドロップしてください。
2. スペースキーで演奏開始／一時停止、ESCキーで演奏停止します。
3. W/A/S/Dキーとマウスで三次元空間内の視点を移動できます。

詳しい操作方法は「Help」→「Manual...」を参照してください。

## MIDI出力

MIDITrail Mod は以下の MIDI 出力バックエンドに対応しています:

| バックエンド | 説明 |
|---|---|
| WinMM | Windows 標準 MIDI 出力。すべての MIDI デバイスで動作します。 |
| KDMAPI | [OmniMIDI](https://github.com/KeppySoftware/OmniMIDI) ダイレクト API。Windows MIDI サブシステムをバイパスする低遅延出力。 |

いずれも libremidi 経由で管理され、MIDI OUT 設定ダイアログから選択できます。

### マルチポート 128ch 対応（OmniMIDI Mod）

[OmniMIDI Mod](https://github.com/yossiepon/OmniMIDIMod) を導入すると、MIDITrail Mod が自動検出し、KDMAPI 経由のマルチポート 128 チャンネル出力（8ポート × 16ch）が有効になります。

**セットアップ:**

1. [Releases](https://github.com/yossiepon/OmniMIDIMod/releases) から `OmniMIDI Mod` をダウンロード
2. `OmniMIDI.dll` を `MIDITrail.exe` と同じディレクトリに配置
3. MIDI OUT 設定に仮想 KDMAPI ポート（"OmniMIDI Mod (KDMAPI Port A)" 〜 "Port H"）が表示される
4. ポートを割り当てて、マルチポート MIDI ファイルを完全なチャンネル分離で再生

**フォールバック:** Mod DLL がない場合、MIDITrail は libremidi 経由の標準 KDMAPI バックエンド（シングルポート、16ch）を使用します。設定変更は不要です。

## ビルド方法

### 必要環境

- Visual Studio 2026 (v18) 以降
- Windows SDK 10.0
- vcpkg（マニフェストモード — 依存パッケージは `vcpkg.json` で管理）

### ビルド手順

```
MSBuild MIDITrail.sln /p:Configuration=Release /p:Platform=x64
```

または Visual Studio で `MIDITrail.sln` を開いてビルド（Release / x64）。

vcpkg マニフェストモードにより、初回ビルド時に必要なパッケージ（DirectXTK）が
自動インストールされます。

## フォルダ構成

| フォルダ | 説明 |
|---|---|
| /MIDITrail | アプリケーション本体(MIDITrail.exe)のプロジェクト。DirectX 11を用いた描画処理を実装。SMIDILib.dll と YNBaseLib.dll を利用。 |
| /SMIDILib | シンプルMIDIライブラリ(SMIDILib.dll)のプロジェクト。MIDIデータの再生とノート情報参照に特化。YNBaseLib.dll を利用。 |
| /YNBaseLib | 基本ライブラリ(YNBaseLib.dll)のプロジェクト。エラー制御やユーティリティ関数を含む。 |
| /Resources | アプリケーションが参照するリソースファイル（バイナリ配布に含まれる）。 |
| /x64 | x64用ビルドモジュールの出力先（Debug / Release）。 |

## ライセンス

修正BSDライセンスを適用して公開しています。
詳細は [COPYRIGHT.TXT](COPYRIGHT.TXT) および [LICENSE.TXT](LICENSE.TXT) を参照してください。

## 変更履歴

[CHANGELOG.md](CHANGELOG.md) を参照してください。
