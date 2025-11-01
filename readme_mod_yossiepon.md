# MIDITrail 1.4.1 mod. yossiepon_20251101

## ピカピカMIDITrail 波紋／鍵盤演出向上・鍵盤方向自動切替・多ポート・歌詞対応等

## Pikapika MIDITrail Mod (Ripple and Piano Keyboard Effect Enhancements / Automatically switch the piano keyboard drawing orientation / Support for multiple MIDI ports in the Piano Roll 3D / Suport for MIDI Lyric Meta Event, etc.)

### 改造点 20251101：
* [FIX] PianoRollRing：波紋Mod（20191224版）の不具合を修正 ※該当バージョンは未リリース
* [ADD] PianoRollRing：歌詞表示対応
* [FIX] 歌詞表示全般：デバッグ実行時にアサーションエラーが発生する問題を修正
* [FIX] MIDIファイル追加読込機能：チャネル番号の上書き指定が処理されない不具合を修正

### Mod 20251101：
* [FIX] PianoRollRing: Fixed Issues in Ripple Mod. (since Mod 20191224, unreleased)
* [ADD] PianoRollRing: Support for Lyrics display.
* [FIX] Lyrics Display: Resolved an assertion error that occurred when running in debug mode.
* [FIX] Add File Menu: Fixed a bug where channel number override was not processed.

### 改造点 20251030：
* [MERGE] 元ソースの 1.4.0～1.4.1をマージ

### Mod 20251030:
* [MERGE] original v1.4.0 - v1.4.1

### 改造点 20250616：
* [MERGE] 元ソースの 1.3.1～1.3.6をマージ

### Mod 20250616:
* [MERGE] original v1.3.1 - v1.3.6

### 改造点 20191224：
* [UPDATE] PianoRollRing：波紋をMod化

### Mod 20191224:
* [UPDATE] PianoRollRing: The ripple is modded.

### 改造点 20190828：
* [MERGE] 元ソースの 1.2.4～1.2.6をマージ
* [FIX] x64実行時にFPUの精度変更がサポートされていないため無効化

### Mod 20190828:
* [MERGE] original v1.2.4 - v1.2.6
* [FIX] Disable to change FPU precision on x64 binary.

### 改造点 20180412：
* [MERGE] 元ソースの 1.2.3をマージ
* [ADD] リップルと背景画像のブレンド方法を指定できる設定を追加
  * INIファイル中のRippleセクション SrcBlendおよびDestBlend
* [FIX] 多ポートのシーケンスでPianoRoll2Dのキーボードが1つに集約されない不具合を修正
* [FIX] PianoRoll3Dで多ポート時のキーボード基準位置がずれている不具合を修正

### 改造点 20170528：
* [FIX] タイトル（シーケンス名）が空の場合、ファイル名を代替表示するよう修正
* [FIX] シーケンス中のテキスト取得時にRTRIMをかけるように修正

### 改造点 20161226：
* [MERGE] 元ソースの 1.2.1 からの取り込みを保留していた機能を反映
  * #30547 機能追加：音階色指定
  * #32427 機能追加：押下状態のキーにノートの色を反映する
* [UPDATE] PianoRoll2D/3DMod : ActiveKeyColorType=NOTE に対応
* [ADD] 波紋上書き回数および波紋間の描画間隔を指定できる設定を追加
  * INIファイル中のRippleセクション OverwriteTimesおよびSpacing

### 改造点 20161223：
* [MERGE] 元ソースの 1.2.2 をベースに再マージ

### 改造点 20140920：
* [MERGE] 元ソースの 1.2.1b をベースに再マージ（とりあえず動く程度）
* [ADD] x64版のバイナリを追加

### 改造点 20121229：
* [ADD] PianoRoll3D: 逆方向にライトを追加しました

### 改造点 20120728-30：
・改造箇所をなるべく別ソースに出すように作り直した
* [FIX] 鍵盤を進行方向から見た際に描画がおかしい問題を修正
* [FIX] PianoRollRainが使えない不具合を修正
* [NEW] ピアノロールの高低の向きに、鍵盤の向きを追従させるようにした
  * 高音部が左・下にある場合は進行方向、右・上にある場合は逆方向を向きます
* [NEW] PianoRoll3Dで多ポートMIDIの場合にポート毎に鍵盤を表示するようにした
* [NEW] Lyricsメタイベント（歌詞）が含まれる場合、表示するようにした
  * ホイール回転に追従しないため、ピアノロールは縦向きにする必要があります
* [NEW] ファイルの追加読み込み機能を追加（メニューのFile＞Add）
  * 主にポート・チャンネル単位での追加読み込みを想定しているため、次の機能と併用します
  * ファイル名に「portX」が含まれる場合、Xをポート番号とみなす（a-Z：大小同一視）
  * ファイル名に「chXX」が含まれる場合、XXをチャンネル番号とみなす（00-99：二桁必須）
  * ※エラーチェックがありませんので、ご注意ください
  * ※ファイル名の例：nit_of_nit_2pin_ch01.mid
  * ※ファイル名の例：真っ黒フランドール・S 修正版_17_Other 1b_portB_ch01.mid

### 改造点 20120320-23：
* ピアノロールの高低を逆にし、鍵盤の向きを進行方向に変更
* 鍵盤を半透明にした際に他のオブジェクトが透けて見えるように修正
  * ※描画順を変更したため、鍵盤を反対から見た際に不具合有り
  * （波紋のαが適用されない、TimeIndicatorをまたぐと鍵盤が見えない等）
* 波紋が輝くように描画方法を変更（DSTにBLEND_ONEで3回上書きしてます）
* 波紋・発音色の減衰描画を、実時間ではなくシーケンサの演奏時間を使うように修正
* 波紋・発音色の減衰率を、リニアではなく指数関数的に行なうように修正
* 波紋・発音色の減衰時間を、固定長ではなく発音長を考慮するように修正

### 改造点 20120318：
* 2D/3D ビューの鍵盤を動くようにした
* 時間をミリ秒表示に修正
* ウィンドウ背景のブラシを黒固定に変更（背景が時々真っ白になるので）
