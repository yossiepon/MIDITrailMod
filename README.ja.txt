******************************************************************************

  MIDITrail ソースコード Ver.1.5.0 for Windows

  Copyright (C) 2010-2026 WADA Masashi. All Rights Reserved.

  Web : https://www.yknk.org/miditrail/
  Mail: wada@yknk.org

******************************************************************************

(1) 概要

  MIDITrail for Windows の全ソースコードです。

(2) ビルド環境

  Microsoft DirectX SDK (June 2010)
  Microsoft Visual Studio Community 2017

(3) フォルダ構成

  /bin
    x86用ビルドモジュールの出力先です。
    binの下に Debug / Release フォルダが作成されます。

  /x64
    x64用ビルドモジュールの出力先です。
    x64の下に Debug / Release フォルダが作成されます。

  /MIDITrail
    アプリケーション本体(MIDITrail.exe)のプロジェクトです。
    DirectXを用いた描画処理を実装しています。
    SMIDILib.dll と YNBaseLib.dll を利用しています。

  /SMIDILib
    シンプルMIDIライブラリ(SMIDILib.dll)のプロジェクトです。
    MIDIデータの再生とノート情報参照に特化したライブラリです。
    YNBaseLib.dll を利用しています。

  /YNBaseLib
    基本ライブラリ(YNBaseLib.dll)のプロジェクトです。
    エラー制御やユーティリティ関数を含んでいます。

  /SoundLib/TinySoundFont
    TinySoundFontライブラリです。
    このライブラリは、SF2ファイルをロードして、波形をレンダリングします。

  /Resources
    アプリケーションが参照するリソースファイルです。

(4) ライセンス

  修正BSDライセンスを適用して公開しています。 
  詳細は LICENSE.txt を参照してください。

(5) 謝辞

  MIDITrailは以下のライブラリとサウンドフォントバンクを使用しています。
  それぞれのライセンスについては、同梱しているライセンスファイルをご確認ください。

  TinySoundFont : Copyright (C) 2017-2023 Bernhard Schelling (Based on SFZero: Copyright (C) 2012 Steve Folta)
    https://github.com/schellingb/TinySoundFont
    ライセンス詳細 : Sources/SoundLib/TinySoundFont/LICENSE

  GeneralUser GS : Copyright (C) 1997-2025 S. Christian Collins
    https://www.schristiancollins.com/
    ライセンス詳細 : Resources/wave/LICENSE.txt

