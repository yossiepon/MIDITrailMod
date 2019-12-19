******************************************************************************

  MIDITrail ソースコード Ver.1.3.1 for Windows

  Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.

  Web : https://osdn.jp/projects/miditrail/
  Mail: yknk@users.osdn.me

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

  /Resources
    アプリケーションが参照するリソースファイルです。

(4) ライセンス

  修正BSDライセンスを適用して公開しています。 
  詳細は LICENSE.txt を参照してください。


