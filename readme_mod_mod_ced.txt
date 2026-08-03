MIDITrail DirectX 11 移植・ライブモニタ全シーン対応・360度動画出力 ほか

MIDITrail 1.4.1 Mod Mod ced_20260627
GitHub: https://github.com/Zel9278/MIDITrailModMod
（yossiepon 版 mod をベースに改造）

────────────────────────────────────────────────────────
ビルド方法 / Build

ImGui は同梱せずパッケージから取得します（どちらの方法でも自動）。

■ xmake  ( https://xmake.io )
    xmake f -p windows -a x64 -m release
    xmake
  → 出力: build/windows/x64/release/MIDITrail.exe
    ImGui は xrepo から自動取得します。

■ MSBuild + vcpkg  ( https://github.com/microsoft/vcpkg )
    1. vcpkg を用意し、一度だけ:  vcpkg integrate install
    2. MIDITrail.sln を Visual Studio で開いてビルド（構成: Release / x64）
       またはコマンドライン:
       MSBuild MIDITrail.sln -t:MIDITrail -p:Configuration=Release -p:Platform=x64
  → 出力: x64/Release/MIDITrail.exe
    ルートの vcpkg.json により ImGui を自動インストールします（静的リンク）。

In short: install xmake ( https://xmake.io ) and run `xmake`, OR install vcpkg
( https://github.com/microsoft/vcpkg ), run `vcpkg integrate install` once, and
build MIDITrail.sln. ImGui is fetched from the package manager automatically.

────────────────────────────────────────────────────────
改造点 20260713：歌詞表示を View メニューから ON/OFF できるようにした

・[NEW][独自] View メニューに「Lyrics」を追加。歌詞イベント（メタイベント 0x05）の表示を
　単独で有効／無効にできる。
　→従来は歌詞の表示が「Ripple」トグルに相乗りしており（DX9 の EffectRipple が両方を駆動していた
　　名残）、波紋を消さないと歌詞を消せなかった。独立したトグルに分離した。
　状態は他の表示設定と同じく View 設定に保存される（キー名: EnableLyrics、既定 ON）。
　「Auto save view settings」が有効なら終了時に保存され、次回起動時に復元される。
　3D / 2D / Ring の各シーンに効く（Rain 系はもともと歌詞非対応）。

────────────────────────────────────────────────────────
改造点 20260713：3D シーンのノートの陰影（ライティング）を DX9 版と同じに戻した

・[FIX] PianoRoll 3D / 3D Live：ノートボックスが陰影なし（＝2D シーンと同じ“のっぺり”）で
　描かれていた不具合を修正。DX9 版と同じ方向性ライトを実装。
　→原因：DX9 は固定機能パイプラインのライティング(D3DRS_LIGHTING)をシーン単位で切り替えており、
　　MTScenePianoRoll3D(Live) だけが方向性ライト2灯（向き (1,-1,2) と (-1,1,-2)、拡散 1.2、
　　環境光 0.2/0.0、ノート材質の環境光 0.5）を有効にしている。一方 MTScenePianoRoll2D(Live) は
　　m_IsEnableLight=FALSE、リングも無効。DX11 移植ではこのライティングが丸ごと抜けており、
　　・再生時(DXNoteBox11)：シェーダにライト計算が無く、さらに箱が 8 頂点（角を共有）だったため
　　　面ごとの法線を持てず、そもそも陰影が付けられない構造だった
　　・ライブ時(MTNoteBoxLive11)：環境光 1.0 を入れて全シーンを強制的に無灯化していた
　　→対策：DX9 の MTNoteBox と同じ 24 頂点（面ごとに法線を持つ 6 面）のテンプレートに変更し、
　　　シェーダに DX9 相当のライティングを実装。2灯が正反対向きなので拡散項は |dot(n, L)| に
　　　まとまる＝ライト軸に沿った面は明るく、直交する面は環境光まで落ちる（＝立体感）。
　　　シーン名で 3D のときだけ点灯（2D/リングは DX9 同様そのまま無灯＝フラット）。
　　　ワールド行列のロール回転は、法線ではなくライト側を逆回転させて等価に処理。
・[FIX] 環境光を「灰色の定数加算」から「ノート色への乗算」に修正（＝DX9 と同じ）。
　→当初 lit = 色×拡散 + 灰色環境光(0.1) としていたが、これだと明るい面で色が白へ寄り、
　　ノートが淡いパステルに退色していた。DX9 の固定機能ライティングは環境光も拡散光も
　　「頂点色（ノート色）の反射率」なので、色 ×(環境光0.2 + 拡散1.2×ndl) となり、明るい面でも
　　チャンネルごとにクランプされるだけで白飛びしない。単ノートで DX9 の (238,128,128) に対し
　　DX11 (238,125,125)＝誤差 3/255 まで一致。
・[FIX] 発音中ノートを「通常ノート側では非表示」にした（＝DX9 と同じ）。DX9 は発音中、全ノート
　バッファ側の元ノートを非表示にし(MTNoteBox::_HideNoteBox)、別バッファで膨らませて描き直している。
　移植版は pass 0（全ノート）でも発音中ノートを描いたまま pass 1 で重ねていたため、同じ位置に
　箱が 2 個ある二重描画になっていた。1 つのノートは必ずどちらか一方のパスだけで描くようにした。
・[FIX] 発音中の白フラッシュ／膨らみの減衰を「発音開始からの固定時間(ActiveNoteDuration)」に
　修正（＝DX9 と同じ）。従来はフラッシュ量 emph を「ノート長で正規化」していたため、長いノートは
　発音中ずっと半分白いまま＝2D/3D とも面が白っぽく退色していた。DX9 は GetActiveNoteColor が
　発音開始からの経過 ms で rate = 1 - 経過/Duration と減衰させるので、長いノートでも冒頭で一瞬
　光ってすぐ元の色に戻る。現在テンポ(TimeDivision×BPM/60000)からフラッシュ減衰距離をワールド X に
　換算してシェーダへ渡し、DX9 と同じ挙動にした。
・[FIX] グリッド（格子線）が深度を書かないようにした。グリッドはノートより先に描く背景の目安
　なのに深度を書いていたため、透明なグリッド（conf の GridLineRGBA アルファ 00＝不可視）でも
　深度だけ残り、グリッド線と交差するノートに「見えない切り欠き（破線状の隙間）」を作っていた。
　DXPrimitive11 に深度書き込みトグル(SetDepthWrite)を追加し、グリッドは深度テストのみ・書き込み
　なしにした。背景として最初に描かれ、鍵盤・ボード・ノートが後からその上に描かれるので前後は不変。
　（実機 A/B：グリッド深度書き込み ON では横方向のノートが切り欠かれ、OFF では連続して描かれる。）
・[FIX] チャンネルごとに極小の深度オフセットを与え、ノート同士の z-fighting を解消した。
　→ノート箱は奥行き 0.1 に対しチャンネル間隔は ChStep（約 0.01）しかないため、別チャンネルの箱同士が
　　めり込む。面が拮抗深度で交差する所は画素ごとに前後が反転する＝黒 MIDI の斑。投影後の深度に
　　pbIdx(port×16+ch)×5e-6×w を加え、チャンネルを深度バッファ上だけで確実に順序付けした。
　　オフセットは D24 で数百レベル＝1 画素にも満たず見た目は不変、鍵盤・ボードとの前後も不変。
　　（実測：この対策の前でも DX11 は DX9 の約 1/12 しか縞が出ていない。DX11 は深度 D24、DX9 は D16
　　なので元々 DX11 の方が精度が高い。本対策でさらに減る。）
・[FIX] ノートの深度書き込みを有効に戻した（＝DX9 と同じ）。カリングも DX9 と同じ両面描画
　（D3DCULL_NONE）のまま。
　→移植版はノートの深度書き込みを切っており、重なったノートを深度ではなく描画順で解決していた。
　　これはノートが無灯＝全ての面が同じ色だった頃は無害だったが、conf のノート色はアルファ FF
　　＝不透明なので、深度を書かないと「後から描いた奥のノートが、先に描いた手前のノートを
　　塗りつぶす」ことになり、ノート面が裏返って描かれる。黒 MIDI のような密な譜面では、面が
　　破線・網目状に崩れて見える。これを z-fighting と誤診し、深度書き込みを切ったままにしていたのが
　　このバグの正体だった（斑の原因は z-fighting ではなく描画順の破綻）。
　　DX9 は MTScenePianoRoll3D で D3DRS_ZWRITEENABLE に一切触れていない＝D3D9 の既定 TRUE のまま。
　　深度を書けば箱の裏面は z バッファに落ちるので、両面描画でもノートは「陰影の付いた面 1 枚」に
　　見える。DX9 と同じ挙動に戻した。
　面の送信順は DX9 の頂点バッファと同じ 上→下→右→左→前→後 のまま。
・[FIX] DXPrimitive11：DX9 の D3DRS_LIGHTING 相当のシーン単位ライティング ON/OFF を追加
　(SetLightEnable)。既定は ON なので鍵盤等の見た目は不変。2D Live のノートだけ DX9 と同じく無灯に。
・[参考] Rain 系は DX9 でもノートが板ポリ（法線 +Y の 1 枚）で、ライト計算の結果が 1.0 に飽和する
　（拡散 0.87 + 環境光 0.25）ため実質フラット。よって DX11 の現状（無灯）と一致しており変更不要。
・[検証] DX9 版(1.4.1)を実際にビルドし、同じ MIDI・同じ視点で描画して画素値を突き合わせた。
　修正後は 4 ch すべての明面・暗面が DX9 と完全一致（例：赤 暗面(133,76,76) 明面(238,128,128)）。

────────────────────────────────────────────────────────
Mod 20260713: lyrics can be switched on and off from the View menu

* [NEW][original] Added "Lyrics" to the View menu, which enables/disables the display of the
  lyrics meta events (0x05) on its own. It used to ride on the "Ripple" toggle (a leftover from
  DX9, where EffectRipple drove both), so you could not turn the lyrics off without also losing
  the ripples. The setting is stored with the other view settings (key: EnableLyrics, default on)
  and, with "Auto save view settings" enabled, is restored on the next run. It applies to the
  3D / 2D / Ring scenes (the rain scenes never had lyrics).

────────────────────────────────────────────────────────
Mod 20260713: 3D scene note shading restored to match DX9

* [FIX] PianoRoll 3D / 3D Live: the note boxes were drawn flat - the same look as the 2D scene -
  because the DX11 port dropped DX9's scene lighting entirely. DX9 toggles fixed-function
  lighting per scene: MTScenePianoRoll3D(Live) enables two opposing directional lights
  (direction (1,-1,2) and (-1,1,-2), diffuse 1.2, ambient 0.2 / 0.0, against the note material's
  ambient 0.5), while MTScenePianoRoll2D(Live) clears m_IsEnableLight and the ring scene never
  lights at all. In the port, the playback renderer (DXNoteBox11) had no lighting in its shader
  and its box template shared 8 corner vertices, so there was nowhere to put a per-face normal;
  the live renderer (MTNoteBoxLive11) forced ambient 1.0 to keep every scene unlit.
  The box template is now 24 vertices (4 per face, each with its own normal), like DX9's
  MTNoteBox vertex buffer, and the shader reproduces DX9's lighting. Because the two lights are
  exact opposites, their diffuse term collapses to |dot(n, L)|: faces along the light axis stay
  bright, faces across it fall to the ambient floor - the 3D relief is back. Lighting is enabled
  by scene name, so the 2D and ring scenes stay flat exactly as in DX9. The world matrix's roll
  is handled by rotating the light the other way instead of the normals.
* [FIX] Ambient light MULTIPLIES the note colour instead of being added as a grey constant (as in
  DX9). The first cut computed lit = colour*diffuse + grey_ambient(0.1), which bleaches a bright
  face toward white and left the notes a washed-out pastel. DX9's fixed-function lighting treats
  both ambient and diffuse as reflectances of the vertex (note) colour, so lit = colour*(ambient
  0.2 + diffuse 1.2*ndl): a bright face just clamps per channel and keeps its hue, never washing
  out. A single note now reads DX11 (238,125,125) against DX9 (238,128,128) - within 3/255.
* [FIX] A sounding note is now hidden from the normal-note pass, as in DX9 (MTNoteBox::_HideNoteBox
  blanks the original note in the all-notes buffer for as long as it sounds, and the swollen copy
  is drawn from a separate buffer). The port drew the note in BOTH passes, leaving two coincident
  boxes. Each note is now drawn by exactly one of the two passes.
* [FIX] The active-note white flash / swell decays over a FIXED time (ActiveNoteDuration ms from
  onset), as in DX9 - not over the note's length. The flash amount emph used to be normalised by
  the note's span, so a long sustained note stayed half-white for its whole duration, bleaching
  the note field in both 2D and 3D. DX9's GetActiveNoteColor decays rate = 1 - elapsed/Duration
  by elapsed ms since onset, so even a long note flashes only at its start and then returns to its
  base colour. The flash decay distance is converted to world X from the current tempo
  (TimeDivision*BPM/60000) and passed to the shader.
* [FIX] The grid no longer writes depth. The grid is a backdrop reference drawn before the notes,
  but it was writing depth, so a fully transparent grid (conf GridLineRGBA alpha 00 = invisible)
  still left depth behind and punched invisible notches - a dashed-gap look - into the notes that
  crossed it. DXPrimitive11 got a depth-write toggle (SetDepthWrite); the grid now tests depth but
  does not write it. It still draws first as a backdrop, with the keyboard / board / notes over it,
  so front-to-back ordering is unchanged. (On-device A/B: with the grid writing depth the horizontal
  notes are notched; with it off they draw continuously.)
* [FIX] Each channel gets a tiny depth offset to kill note-vs-note z-fighting. A note box is 0.1
  deep while channels are only ChStep (~0.01) apart, so different channels' boxes interpenetrate;
  where their faces cross at near-equal depth the test flips pixel to pixel (the speckle on dense /
  black MIDI). The clip-space depth gets pbIdx(port*16+ch)*5e-6*w added, so the depth buffer orders
  channels deterministically. The offset is a few hundred D24 levels - under one pixel of motion,
  invisible, and far below the depth gap to the keyboard / board, so real ordering is untouched.
  (Measured: even before this, DX11 showed ~12x less speckle than DX9 - DX11's depth buffer is D24
  vs DX9's D16, so it was already more precise; this reduces it further.)
* [FIX] Notes WRITE depth again, as in DX9. Culling stays as in DX9 too: both sides of the box are
  drawn (D3DCULL_NONE).
  The port had turned depth writing off, so overlapping notes resolved by draw order instead of by
  depth. That is harmless only while the notes are unlit and every face is the same flat colour.
  The note colours in the confs are opaque (alpha FF), so with no depth write a note drawn later
  paints straight over a nearer note drawn earlier, and the note field renders inside out - on dense
  (black MIDI) material the note faces break up into dashes and hatching. That artifact was
  misdiagnosed as Z-fighting, and leaving depth writing off to "avoid the Z-fighting" is what kept
  it alive: the speckle was never depth precision, it was the draw order collapsing.
  DX9 never touches D3DRS_ZWRITEENABLE in MTScenePianoRoll3D, so it keeps D3D9's TRUE default. With
  the depth write back on, the z-buffer rejects the far faces of each box, so even with both sides
  drawn a note reads as one lit face - exactly as in DX9.
  The six faces are still submitted in DX9's order (top, bottom, right, left, front, back).
* [FIX] DXPrimitive11: added a per-scene lighting switch (SetLightEnable), the equivalent of
  DX9's D3DRS_LIGHTING. It defaults to on, so the keyboard and the other lit objects are
  unchanged; only the 2D live notes go unlit, as they are in DX9.
* [VERIFIED] The DX9 build (1.4.1) was compiled and run on the same MIDI from the same viewpoint,
  and the rendered pixels were compared. After the fix every channel's lit and shaded faces match
  DX9 exactly (e.g. red: shaded (133,76,76), lit (238,128,128)).
* [NOTE] The rain scenes need no change: in DX9 their notes are flat quads with a single +Y
  normal, and the light saturates them to full brightness (diffuse 0.87 + ambient 0.25 > 1.0),
  so DX9's rain notes are effectively unlit - which is what DX11 already draws.

────────────────────────────────────────────────────────
改造点 20260703：無限鍵盤を「通常鍵盤と全く同じ描画」に統一 ＋ 停止時視点リセットの戻し先修正

・[FIX][独自][実験的] 無限鍵盤：境界の黒鍵（note126 F#9 など）の位置ズレを修正。
　→原因：ジオメトリ生成の黒鍵位置補正に「最後の黒鍵(note126)は中央寄せ」「表示範囲の
　　先頭/末尾で取り残される黒鍵は中央寄せ」という端専用の特例があり、無限鍵盤では端が
　　本当の端ではない（両方向に拡張が続く）のに F#9 等が中央寄せされ、内側の同じ音の黒鍵
　　（本来の F# シフト）とズレていた。無限鍵盤時はこれら端の中央寄せを行わないようゲート。
　　→ F#9 の中心オフセットが内側 F#8 と完全一致（実測 -0.034 = -0.034）し、拡張と揃う。
・[FIX][独自] 手動停止(Stop/ESC)時の視点リセットの戻し先を「保存済み視点」に変更。
　→従来はハードコードの既定視点(SetDefaultViewpoint)に戻していたため、Auto save viewpoint で
　　作った視点や、OFF 時のユーザー視点まで毎回“初期位置”に飛ばされていた。
　　ロード時と同じ手順で conf の "Viewpoint-<scene>"（保存が無ければ既定）へ戻すように統一
　　(_ResetViewpointToSaved)。Auto save viewpoint ON なら直前保存で現在位置＝保存視点となり
　　実質そのまま維持、OFF でもユーザーが保存した視点へ戻る。
・[FIX][独自][実験的] 無限鍵盤：拡張オクターブを別バッファ／別 Draw で描くのをやめ、
　各キーボード自身の頂点・インデックスバッファの「末尾に連結」して、通常の 0-127 と
　“同じ 1 回の Draw・同じワールド行列”でまとめて描くようにした。
　→拡張部は通常鍵盤と完全に同一の描画経路を通るため、別 Draw 同士の深度競合が原理的に
　　なくなり、環境（GPU）によって拡張部の黒鍵が白鍵に覆われる不具合が出なくなる。
・[FIX][独自][実験的] 無限鍵盤：インデックスの送信順を「真のノート順」に並べ替えた
　＝［note0未満の拡張］→［通常 0-127］→［note128以降の拡張］。
　→原因：鍵盤キーは conf で半透明(WhiteKeyColor/BlackKeyColor の alpha<255)にでき、
　　鍵盤はブレンド有効＋深度書き込みONで描かれる。拡張部をバッファ末尾に足すと、
　　note0 未満の拡張が通常鍵盤より“後”に描かれ（＝順序が逆）、半透明＋同一深度の境界で
　　「後に描いた面が勝つ(LESS_EQUAL)」ため、勝者が環境依存でブレの出る＝GPUによって
　　境界の黒鍵が欠けて見える不具合になっていた。送信順をノート順に統一することで、
　　境界も内側と全く同じ順序で解決され、どのGPUでも通常鍵盤と同じ見た目になる。
　　（＝深度/半透明の“描画順”問題であり、透明度の有無そのものではない。）
・[FIX][独自][実験的] 無限鍵盤：note127(G9) の「右の切り欠き」を塞いでいた端キー処理を無効化。
　→原因：ジオメトリ生成に「表示範囲の端キー(GetKeyDispRangeEnd==noteNo)は隣に黒鍵が無いので
　　切り欠きを埋めて全幅の白鍵にする」処理があり(MTPianoKeyboard White1/2/3)、無限鍵盤で端が
　　note127 になると note127(G) の右切り欠きが塞がれる。しかし拡張の note128(G#) はまさにその
　　切り欠き位置に来るため、塞いだ白鍵の天面と G# が同一面で重なり＝129鍵目だけ z-fighting。
　　→ 無限鍵盤時は端キーの切り欠き埋め(開始/終了とも)を行わないようにゲート。切り欠きが残るので
　　　拡張の黒鍵が内側の黒鍵と全く同じように収まり、z-fighting が消える。
・バッファ内はノート順（各オクターブ C,C#,D,…,B）で焼き込む＝通常鍵盤の生成順と同一。
　拡張部は通常鍵盤と同じ頂点色（alpha 含む）を複製するので透明度も一致。静的（押下アニメは 0-127 のみ）。

────────────────────────────────────────────────────────
Mod 20260703: infinite keyboard unified to render exactly like the normal keyboard

* [FIX][original][experimental] Infinite keyboard: the extension octaves are no longer a
  separate buffer / separate Draw. They are APPENDED to the tail of each keyboard's own
  vertex/index buffer, so the same single Draw covers both the normal 0-127 keys and the
  extension, with the same world matrix and the same primitive submission order. The extension
  therefore goes through the identical render path as the main keyboard -> no cross-draw depth
  fight, so the GPU-dependent bug where white keys covered the extension's black keys is gone.
  Keys are baked in note order (per octave C,C#,D,...,B) = the main keyboard's build order; the
  raised black keys resolve in front just like the originals (the old white-then-black reorder
  is dropped). The extension is static (only notes 0-127 animate on key press).

────────────────────────────────────────────────────────
改造点 20260630：無限鍵盤の調整 ＋ 停止時の視点リセット

・[FIX][独自][実験的] 無限鍵盤：拡張部を「全オクターブのオフセットを頂点に焼き込んだ
　1つの静的バッファ」にして、メイン鍵盤と“全く同じワールド行列”で1回描画する方式に変更
　（±24オクターブ、実質無限）。さらにバッファ内は「白鍵を全部先→黒鍵を全部後」の順で
　焼き込む。深度が同値になる境界でも、後に描く黒鍵が必ず勝つ(LESS_EQUAL)ため、どのGPU
　でも黒鍵が手前に出る。オクターブごとに別行列で描いていた旧方式は深度がGPU依存で揺れ、
　環境によって白鍵が拡張部の黒鍵を覆う不具合が出ていた。
・[FIX][独自][実験的] 無限鍵盤の高音側の継ぎ目を修正（両方向に延長・隙間なし）。
　→note127 は本来 G9 だが「閉じた端キャップ(B 形状)」で描かれ、拡張部の G#9(=note128) が
　　背面にめり込んでいた。ON 時は note127 を本来の G 形状（右に G# 用の切り欠き）にし、
　　note128 の位置を外挿してその切り欠きに G#9 を収める。→ 休符時は継ぎ目がクリーン。
　　（既知の軽微：最高音 G9=note127 を「押した」瞬間だけ、押下アニメで動く note127 と
　　　静止する拡張 G#9 がずれて見える。最高音のため実使用ではほぼ発生しない。）
・[FIX][独自] 無限鍵盤のカリング距離をピッチ軸に垂直な距離 sqrt(Y^2+Z^2) に変更。
　向きによって延長が出ない問題を修正（上限160）。
・[NEW][独自] DX9 互換：手動停止(Stop)時にマウスで動かした視点を既定へ戻す。
　（曲切替/リサイズ/AA変更などのシーン更新では従来どおり視点を保持）

────────────────────────────────────────────────────────
Mod 20260630: infinite keyboard tweaks + viewpoint reset on Stop

* [FIX][original][experimental] Infinite keyboard high-end seam fixed (extends both ways, no
  gap). note 127 is really G9 but was drawn as a closed cap (B shape), so the extension's G#9
  (note 128) intersected its back. When enabled, note 127 is now drawn as a real G (with a
  right notch for G#) and note 128's position is extrapolated to place G#9 in that notch -> the
  seam is clean at rest. (Minor known issue: pressing the very top key G9 = note 127 briefly
  desyncs its key-press animation from the static extension G#9; rare since it's the top note.)
* [FIX][original][experimental] Infinite keyboard: the extension now re-draws one interior
  octave (notes 12-23) of EACH keyboard's own prim at octave-shifted world matrices, instead
  of a separate static buffer. Identical vertices / render state / depth as the normal
  keyboard, so the previous misalignment and black-key flicker (z-fighting from the separate
  buffer) are gone. It also excludes the note 0/127 keyboard end-caps.
* [FIX][original] Infinite-keyboard culling now uses the distance perpendicular to the pitch
  axis sqrt(Y^2+Z^2), so the extension grows in every scene orientation (cap 160).
* [NEW][original] DX9 parity: a manual Stop resets the (mouse-moved) viewpoint to the default.
  Scene updates (song switch / resize / AA change) still keep the viewpoint as before.

────────────────────────────────────────────────────────
改造点 20260629：AMD動画エンコーダ ＋ 鍵盤テクスチャ関連

・[FIX][独自] Config Manager(ImGui) を開いて閉じた後にマウスで見回せなくなる問題を修正
　→従来は開いた時にマウスカメラを強制 OFF するだけで、閉じても戻していなかった。
　　開いた瞬間に状態を退避（カーソルを使えるよう一時 OFF）、閉じた瞬間に元へ復元する。
・[FIX][独自] ロード後（特にドラッグ&ドロップ）にカメラ操作が効かなくなる問題を修正
　→根本原因：DirectInput は DISCL_FOREGROUND のため、ウィンドウが非アクティブのまま
　　MIDI をロードすると DI を取得できず、キーボード/マウス(カメラ)が操作不能になっていた
　　（メニュー等をクリックしてウィンドウがアクティブになると直る、の理由）。
　　対策(1) ロード完了時に SetForegroundWindow / SetFocus でウィンドウを必ずアクティブ化。
　　対策(2) DI の GetDeviceState 失敗時は再取得して1回リトライ、それでも駄目なら状態を
　　　　　　ゼロクリア（前回値での凍結＝キー押しっぱ/無反応を防ぐ）。失敗判定は特定エラー
　　　　　　コードだけでなく FAILED 全般に拡大（0x8007000c 等も復帰対象）。キー/マウス両方。

・[NEW][独自] 動画出力に AMD GPU ハードウェアエンコーダを追加
　→コーデック選択に「H.264 (AMF, AMD GPU)」「H.265/HEVC (AMF, AMD GPU)」を追加。
　　ffmpeg の h264_amf / hevc_amf を使用（-quality quality -rc cqp -qp_i/-qp_p）。
　　NVIDIA(NVENC) / Intel(QSV) / CPU(x264/x265) に続く第4の HW エンコーダ。
・[CHG][独自] 鍵盤画像を ini で指定できるよう [Bitmap] に Keyboard 項目を明示追加
　→PianoRoll 2D/3D とその Live の conf に `Keyboard=data\Keyboard.png` を追加
　　（Rain 系は元から記載あり）。既定は従来と同じ data\Keyboard.png。
　　Release はビルド時に conf がコピーされるので、ここを差し替えれば鍵盤画像を変更できる。
・[NEW][独自][実験的] 無限鍵盤（背景の fake 鍵盤を不要にする）
　→NotLive の PianoRoll 2D/3D で、0-127 の外にもオクターブパターンを延長して鍵盤が
　　無限に続くように見せる。カメラの可視範囲外のオクターブは描画しない（負荷対策）。
　　・既存の 1 オクターブ分(0-11鍵)の未押下ジオメトリを静的ブロックとしてコピーし、
　　　オクターブ幅で下方向(note<0)と上方向(note>127、128-131 は部分タイルで隙間なく)に
　　　タイル描画。拡張部は装飾（音が無いので光らない）。
　　・conf `[PianoKeyboard] InfiniteKeyboard=1` で有効化（既定 0=OFF、実験的）。
　　・ON のときは KeyDispRangeStart/End を無視して全 128 鍵表示（クリップ範囲と
　　　拡張タイルの隙間＝違和感を防ぐため）。
　　・128鍵固定配列（押鍵アニメ等）は不変＝低リスク。

────────────────────────────────────────────────────────
Mod 20260629: AMD video encoder + keyboard texture

* [NEW][original] AMD GPU hardware video encoder for export: "H.264 (AMF, AMD GPU)"
  and "H.265/HEVC (AMF, AMD GPU)" (ffmpeg h264_amf / hevc_amf). Joins NVENC / QSV / CPU.
* [CHG][original] Keyboard image is now listed in conf [Bitmap] (Keyboard=data\Keyboard.png)
  for PianoRoll 2D/3D + Live (Rain already had it), so it can be swapped per scene.
* [NEW][original][experimental] Infinite keyboard (so people no longer hand-build a fake
  background keyboard). On non-live PianoRoll 2D/3D, the octave pattern is extended beyond
  note 0-127 in both directions and camera-culled (off-screen octaves are not drawn). A
  static, unpressed one-octave block (notes 0-11) is tiled by octave width below note 0 and
  above note 127 (with a partial tile for 128-131 so there is no gap). Extension keys are
  decorative (no light-up). Enable via conf [PianoKeyboard] InfiniteKeyboard=1 (default 0).
  When enabled, KeyDispRangeStart/End are ignored (full 0-127 keyboard) so the clipped
  range and the extension tiles do not leave a gap.
  The fixed 128-key arrays / key-press animation are untouched (low risk).

────────────────────────────────────────────────────────
改造点 20260627-1：リリース後の不具合修正（20260628）

・[FIX][独自] Config Manager の色項目判定を改善
　→以前は「値がちょうど 8桁16進」というだけで色とみなしていたため、
　　NumberOfStars=10000000 のような 8桁の数値までカラーピッカーになっていた不具合を修正。
　　判定を「キー名が color / rgb / rgba で“終わる” かつ 値が 6桁(RGB) または
　　8桁(RGBA) の16進」に変更した。これにより：
　　・ActiveKeyColorDuration / ActiveKeyColorTailRate / NoteColorType のように
　　　Color を含むが色でないキーを誤検出しない（末尾一致のため）。
　　・BackGroundRGB=000000 のような 6桁RGB（アルファ無し）もカラーピッカーで編集可能に。
　　・8桁(NoteRGBA / ActiveKeyColor 等)はアルファ付き、6桁はアルファ無しで編集し、
　　　元の桁形式（6/8）のまま書き戻す。

・[NEW][独自] 色設定ダイアログ（Options → Color → パレット編集）でアルファ（透明度）を
　編集できるようにした
　→Windows 標準の色選択ダイアログ(ChooseColor)は OS 仕様でアルファ非対応。そこで：
　　(1) スウォッチを押すと自作の RGBA ピッカー（MTColorPickerDlg）が開く。
　　    ・HSV の2Dフィールド（彩度×明度）＋色相バー＋透明度バーを**ドラッグ**して選べる
　　    ・R/G/B/A のスライダー＋数値入力欄＋hex 欄でも調整できる
　　    ・プレビューは左半分＝不透明 / 右半分＝透明度あり(市松)で確認できる（ImGui風）
　　    オーナー描画は DIB 一括転送にして**ちらつきを解消**（当初プレビューがちらついた）。
　　(2) 各色の hex 欄を RGB(6桁)化し、その右に専用アルファ欄(0–255)を追加（手入力用）。
　　OK 時に RGB＋アルファを合成してパレットへ反映。Ch.1-16 / Background / Grid Line /
　　Counter / グラデーション Start・End すべてに対応。

・[NEW][独自] アンチエイリアシングを強化（Options → Graphic）
　(1) MSAA 16x 対応：レンダラ側が 8x で頭打ちだったのを 16x まで要求できるよう拡張
　　　（設定ダイアログは元から 16x まで対応。GPU が 16x MSAA をサポートしていれば有効、
　　　 非対応なら自動で 8x 以下にフォールバック）。
　(2) SSAA（スーパーサンプリング）追加：Graphic 設定に「SSAA」コンボ（OFF/2x/3x/4x）を
　　　新設。有効時は 3D シーンを内部的に N倍解像度のオフスクリーンへ描画し、
　　　factor×factor の平均（box フィルタ）でバックバッファへ縮小合成する。MSAA が苦手な
　　　テクスチャ内部やシェーディングのジャギーにも効き、どの GPU でも使える。
　　　（当初 bilinear 1タップで 3x/4x がジャギー残りだったため box フィルタに修正）
　　　設定キーは Graphic.ini [Anti-aliasing] SuperSample（1=OFF, 2..4）。
　　　※高負荷（4x = 面積16倍）。Black MIDI 等では重くなるので注意。
　　　※ダッシュボード(ファイル名/カウンタ)と ImGui は SS 縮小の“後”にバックバッファ
　　　　等倍で描画する。固定ピクセル配置の文字を SS 解像度で描くと NDC がずれて消える
　　　　不具合があったため修正（テキストはネイティブ解像度の方が綺麗）。
　[NOTE] BOM 無しだった DXRenderer11 / MTNoteLyrics11 に UTF-8 BOM を付与
　　　（日本語コメント追加に伴う CP932 誤認警告 C4819 を解消）。

・[CHG][独自] NoteColorType=CHANNELTRACK の配色方式を変更
　→従来は (track,channel) を黄金角で連続色相にするだけでカラーパレット未使用だった。
　　新方式：各チャンネルで「最初に現れたトラック」はそのチャンネル色（パレット[ch]）、
　　同じチャンネルを共有する2本目以降のトラックは (track,channel) の決定論ハッシュ
　　（Knuth 乗算）でパレット16色へ分散。1ch に複数トラックが乗る曲でもトラック毎に
　　色が分かれ、かつ設定したカラーパレットがそのまま反映される。MTNoteDesign に
　　m_FirstTrackForChannel[16] を持たせ、ノート順に on-the-fly で主トラックを判定
　　（Initialize で曲ごとにリセット）。Box / Rain / Ring の各ノートに適用。

* [FIX][original] Config Manager color-field detection improved.
  Previously any value that was exactly 8 hex digits was treated as a color, so a
  plain number such as NumberOfStars=10000000 wrongly became a color picker. A field
  is now treated as a color only if the KEY name ends with color/rgb/rgba AND the
  value is 6 (RGB) or 8 (RGBA) hex digits:
  - keys that merely contain "Color" but are not colors (ActiveKeyColorDuration,
    ActiveKeyColorTailRate, NoteColorType) are excluded (suffix match);
  - 6-digit RGB (e.g. BackGroundRGB=000000) is now supported (no-alpha picker);
  - 8-digit values keep the alpha bar; the original digit width is preserved on save.

* [NEW][original] Alpha (transparency) editing in the Color palette dialog
  (Options -> Color -> edit a palette). The Windows ChooseColor dialog cannot edit
  alpha (OS limitation), so: (1) clicking a swatch now opens a custom RGBA picker
  (MTColorPickerDlg): a draggable HSV 2D field (saturation x value) + a hue bar, plus
  R/G/B/A sliders, numeric boxes and a hex box, and a checkerboard transparency preview.
  Owner-draw uses one-shot DIB blits so it does not flicker. (2) each color's hex box is
  RGB (6 digits) with a dedicated alpha field (0-255) next to it for manual entry. On OK
  the RGB and alpha are combined into the palette. Applies to Ch.1-16, Background, Grid
  Line, Counter, and the gradation Start/End colors.

* [NEW][original] Stronger anti-aliasing (Options -> Graphic).
  (1) 16x MSAA: the renderer was capped at 8x; it now requests up to 16x (the dialog
      already listed 16x). Works if the GPU supports 16x MSAA, else falls back to <=8x.
  (2) SSAA (supersampling): a new "SSAA" combo (OFF/2x/3x/4x). When on, the 3D scene is
      rendered to an offscreen target at NxN resolution and downscaled to the backbuffer
      with an f x f box average (a single bilinear tap only blends 2x2, so 3x/4x would
      still alias). It antialiases texture interiors and shading that MSAA cannot, and
      works on any GPU. Config key: Graphic.ini [Anti-aliasing] SuperSample (1=off, 2..4).
      Note: 4x = 16x the pixels, heavy on black-MIDI scenes.

* [CHG][original] NoteColorType=CHANNELTRACK recolouring method changed.
  It used to just spread (track,channel) around the hue wheel (golden angle), ignoring
  the palette. Now: the first track seen on each channel gets that channel's palette
  colour; additional tracks sharing the channel get a deterministic hash (Knuth) into
  the 16-colour palette. Multi-track-per-channel songs get distinct per-track colours
  while still honouring the configured palette. MTNoteDesign tracks the first track per
  channel on the fly (reset each Initialize); applied to box / rain / ring notes.

────────────────────────────────────────────────────────
改造点 20260627：本家 1.4.1 をマージ ＋ 独自機能追加

・[NEW][独自] Config Manager を追加（Options → Config Manager...）
　→conf/ の PianoRoll 系シーン設定 .ini（3D/2D/Rain/Ring とその Live 版）を
　　GUI(ImGui) で直接編集できる。Player.ini / Video.ini 等は編集対象外。
　　・ファイルをコンボで選択、[section] は折りたたみ、key=value は入力欄で編集。
　　　コメント(;)・行順は保持して保存する。
　　・色の項目はカラーピッカー（スウォッチ＋パレット＋アルファ）で編集できる。
　　　（色判定の条件と不具合修正の詳細は「改造点 20260627-1」を参照）
　　・選択肢の項目はコンボ（ドロップダウン）で選択できる：
　　　NoteColorType（CHANNEL/SCALE/CHANNELTRACK）、ActiveKeyColorType（STANDARD/NOTE）、
　　　SrcBlend/DestBlend（ZERO/ONE/SRCALPHA/INVSRCALPHA/DESTALPHA/INVDESTALPHA）。
　　・Save すると現在のシーンを自動で再構築して変更を即反映。
　　・ウィンドウ表示中はカメラ操作が裏で効かないようガード。カメラは DirectInput で
　　　マウス/キー/パッドを直接読むため、表示中はカメラのユーザ入力自体を停止し
　　　（自動スクロール/ロールは継続）、マウスカメラ掴み(カーソル非表示)も解除する。


・[UPDATE] 本家 yossiepon 版 1.4.1 (mod. 20251101) をマージ
　→共有エンジン SMIDILib の 1.4.1 修正を全面取り込み：
　　・[FIX] 歌詞読込時のバッファサイズ誤りによるメモリ破壊の修正を反映
　　　（DX11 は歌詞を char で扱うため、安全な strncpy_s+_TRUNCATE 経路を維持）
　　・[FIX] MIDI 追加読込時のチャンネル番号上書きが効かない不具合の修正
　　　（GetDataSet 読み出し時に遅延適用する 1.4.1 方式を採用）
　　・[ADD] RIFF(RMID) ヘッダのスキップ対応／トラック終端の寛容なスキップ処理
　　・[ADD] アクティブノートのベロシティ追跡と、シーク時の Note Off/On 再送
　　　（All Notes Off 非対応音源での音残り対策）／全ポート Sound Off
・[KEEP] Mod Mod の軽量パーサー／再生エンジンはそのまま維持（1.4.1 へ退行させない）：
　→SMFileReader のメモリマップ読み込み・読込進捗コールバック・32bit 上限ガード
　→SMSeqData のトラックマージを min-heap による k-way マージ（O(N log T)）に維持
　→SMTrack のノートオン/オフ対応付けを O(1) フラット配列（黒MIDI 連打対応）で維持
　→SMSequencer の黒MIDI catch-up ループ（1tick で滞留イベントを一括処理）を維持
　→シークの DX9 スライド追従（GetCurrentTickTime ポーリング）を維持
・[NEW] 1.4.1 の新規アプリ機能を DX11/MBCS アプリへ移植：
　　・[File] フォルダを開く（フォルダ選択ダイアログ／IFileOpenDialog）
　　・[File] 前ファイル／次ファイル（フォルダ内の MIDI を順送り）
　　・[File] フォルダ演奏（曲終了で次ファイルへ自動送り。最後で停止）
　　・[View] My Viewpoint 1〜3（視点の保存／呼び出し。シーン別に設定ファイルへ保存）
　　　※当初 m_pScene(常にNULL)経由で何も動かなかったため、DX11 カメラ(m_FpCam11)を
　　　　直接読み書きする実装に修正（自動Viewpointと同じ X/Y/Z/Phi/Theta/Roll 形式）
・[FIX] 同じ表示モードのまま曲を切り替える/ウィンドウをリサイズ/AA変更すると、視点が
　　毎回デフォルト(または最後に保存した位置)へ戻ってしまう不具合を修正
　→_SetupDX11Scene で「同一シーンの再セットアップ」を検知し、その場合は現在の視点を
　　now-line相対で退避→再適用して、ユーザが合わせたカメラ位置を保持する
　　（表示モードを切り替えた時・初回ロード時は従来どおりデフォルト＋保存値を採用）
　　・[View] メニューバー表示切替（非表示時はウィンドウ上端にマウスを乗せると
　　　一時的にメニューが出る＝本家1.4.1と同じ挙動。DX11 で漏れていた分を追加）
　　・[View] Auto save view settings（Mod Mod 独自）：本家の Auto save viewpoint の
　　　直下に追加したトグル。ON にすると View の表示設定（Piano Keyboard/Ripple/
　　　Pitch Bend/Pitch Bend Whole Channel/Stars/Counter/Background Image/
　　　Time Indicator/Grid Box/Single Keyboard）を再起動後も保持する
　　　（[Scene] セクションへ保存。終了時とトグル切替時に書き出し、起動時に復元）
　　・[Ring] PianoRollRing の歌詞表示（1.4.1 の目玉機能）を DX11 へ移植。
　　　MTNoteLyrics11 に ring モードを追加し、位置決めと world 移動を MTNoteDesignRing
　　　から取得（タイミング/色は既存の平面用設計を流用＝ini 値共通）。リングのノートと
　　　同じ world フレーム（RotX(roll)×Trans(worldMove)）で完全整合。再生時のみ表示し
　　　Ripple トグルに連動。
　　※フォルダ内ファイル列挙は Unicode(WCHAR)、本アプリは MBCS のため
　　　m_LoadFilePathW 経由で Unicode パスを保持しつつ char 経路へ橋渡しして読み込む。
・[NEW] 1.4.1 のカラーパレット設定 UI を統合（Option → Color...）。
　　・パレット0(デフォルト)＝現行シーン ini の色そのまま（退行なし）。
　　　ユーザパレット 1〜6 を作成・編集し、選択でノート色を切替。
　　・ノート色（16ch）とグリッド線色を選択パレットから取得するよう MTNoteDesign を配線。
　　・Mod 独自のカラー処理（ActiveKeyColor・emissive・CaptionRGBA）は別系統で ini のまま温存。
　　・パレットのインポート/エクスポートダイアログも同梱。
　　・[NEW] 透明度（アルファ）を編集可能化。色テキスト欄を編集可(EDITTEXT)にし、
　　　8桁 RGBA 16進（末尾2桁＝アルファ）を直接入力して透明度を設定できる。
　　　（Windows の色選択は RGB のみのため、アルファはテキスト欄で指定）
　　・[NEW] DX11 の 3D/2D ピアノロールのノート描画にアルファブレンドを追加し、
　　　ノートの透明度が実際に反映されるようにした（従来は不透明固定）。
　　　深度書込は維持（ソートなし）のため、重なり順は描画順依存。
　　　alpha=FF のノートは従来同様の見た目（黒MIDI等ではブレンド分の負荷あり）。
　　※背景色・カウンタ色のパレット適用、および Ring/Rain ビューのノート透明度は
　　　今回未対応（要望あれば対応）。
・[NB] AMD Radeon 向け ripple 修正（D3D9 固定機能のブレンド変更）は未移植。
　　DX11 の描画経路と非互換／upstream でも未検証のため。要望があれば個別に対応。

Mod 20260627: merged upstream 1.4.1 + original features

*[NEW][original] Config Manager (Options -> Config Manager...): a built-in ImGui GUI
  editor for the PianoRoll scene conf files (3D/2D/Rain/Ring and their Live variants);
  Player.ini / Video.ini and others are intentionally not editable here. Pick a file
  from the combo; [sections] are collapsible, key=value rows are edit fields; comments
  (;) and line order are preserved on save. Colour fields (8-digit RRGGBBAA hex values
  - NoteRGBA / ActiveKeyColor / GridLine etc.) are edited with a colour picker
  (swatch + palette + alpha). Enumerated keys use a combo (dropdown): NoteColorType
  (CHANNEL/SCALE/CHANNELTRACK), ActiveKeyColorType (STANDARD/NOTE), SrcBlend/DestBlend
  (ZERO/ONE/SRCALPHA/INVSRCALPHA/DESTALPHA/INVDESTALPHA). Saving rebuilds the current scene so the
  change applies immediately. While the window is open the camera is frozen against user
  input: the camera reads mouse/keys/pad via DirectInput (which bypasses Win32/ImGui),
  so its user input is disabled (auto-scroll/roll continue) and the mouse-cam grab
  (hidden cursor) is released so the cursor can drive the ImGui UI.


*[UPDATE] Merged upstream yossiepon 1.4.1 (mod. 20251101).
  Engine (SMIDILib) fixes from 1.4.1 fully integrated:
  - lyrics buffer-size heap-corruption fix (DX11 keeps the safe char strncpy_s path),
  - "channel overwrite on appended MIDI" fix (1.4.1 deferred-apply via GetDataSet),
  - RIFF (RMID) header skip + tolerant track-end skipping,
  - active-note velocity tracking + Note Off/On resend on seek (for synths without
    All Notes Off) + all-port Sound Off.
*[KEEP] Mod Mod's lightweight parser/playback engine preserved (no regression to 1.4.1):
  memory-mapped SMF read + load-progress callback + 32-bit cap guard; min-heap k-way
  track merge; O(1) flat-array note on/off pairing (Black MIDI); the Black-MIDI
  catch-up loop in the sequencer; and the smooth DX9-style seek slide.
*[NEW] Ported 1.4.1's new app features into the DX11/MBCS app:
  - Open Folder (IFileOpenDialog folder picker),
  - Previous / Next file (navigate MIDI files in the folder),
  - Folder Playback (auto-advance to the next file when a song ends; stops at the last),
  - My Viewpoint 1-3 (save/restore camera viewpoints per scene, in the config file),
  - Menu Bar toggle,
  - PianoRollRing lyrics (1.4.1's headline ring feature) ported to DX11: MTNoteLyrics11
    gained a ring mode that takes positions + world-move from MTNoteDesignRing (timing/
    colour stay on the planar design = shared ini values), so lyrics lay on the ring in
    the exact same world frame as the ring notes. Playback only, tied to the Ripple toggle.
  (Folder enumeration is Unicode/WCHAR; the app is MBCS, so Unicode paths are kept via
   m_LoadFilePathW and bridged into the existing char load path.)
*[NEW] Integrated 1.4.1's colour-palette config UI (Option -> Color...):
  - palette 0 (default) = the current scene's ini colours (no regression); users can
    create/edit palettes 1-6 and switch note colours by selecting one,
  - MTNoteDesign now sources the 16 channel colours and the grid-line colour from the
    selected palette; the Mod colour features (ActiveKeyColor / emissive / CaptionRGBA)
    stay ini-based and untouched,
  - palette import/export dialogs are included.
  - [NEW] alpha (transparency) is editable: the colour text fields are now editable
    (EDITTEXT); type an 8-digit RRGGBBAA hex (last 2 = alpha) to set transparency
    (the Windows colour picker is RGB-only, so alpha is set via the text field),
  - [NEW] the DX11 3D/2D piano-roll note renderer now alpha-blends, so note
    transparency is actually shown (notes used to be forced opaque). Depth write is
    kept (no sort) so overlap order is draw-order dependent; alpha=FF looks identical
    to before (with the blend cost on dense/Black MIDI).
  (Background/counter palette wiring and Ring/Rain note transparency are not done yet.)
*[NB] The D3D9 AMD-Radeon ripple fix was NOT ported (incompatible with the DX11 render
  path / unverified upstream). Can be done on request.

────────────────────────────────────────────────────────
改造点 20260623：

・[FIX] 鍵盤アニメーションを DX9 と同等に修正
　→押下を先読みし、音が鳴る瞬間にちょうど押し切る（KeyDownDuration）。
　　離鍵は KeyUpDuration でゆっくり戻す。テンポ・再生速度に追従
　→色は押し切った時だけ付与（音の開始でパッと付き、終了で消える）
・[FIX] 鍵盤の押下色を ini の [PianoKeyboard] ActiveKeyColor 設定に対応
　→専用パレット（Ch-NN-ActiveKeyColor）＋ActiveKeyColorType を反映（従来はノート色固定）
　→色はノートオンの瞬間に即フル色で付与（フェードなし）
　→CHANNELTRACK（トラック別色）モード時はトラック色を維持
・[FIX] アクティブノートに [ActiveNote] EmissiveRGBA を反映（DX9 の emissive 加算）
・[NEW] ロード画面を改善（進捗バーに％表示、全体を 0→100％で単調表示）
　→重いノートフィールド構築中も件数とバーが動くように進捗を配線
　→「Reading MIDI file」がトラック毎にリセットしていた問題を修正（通し表示）
・[FIX] 巨大ファイル（32bit 上限＝約42.9億イベント／約21億ノート 超）読み込み時の
　　データ破壊・誤カウントを防止
　→上限到達で安全に打ち切り、「読めた分を表示しますか？」を Yes/No で確認
・[FIX] 再生/モニタリング中でないのに左クリックでマウスがグラブされる不具合を修正
　→曲アンロード(タイトル/曲なし)時のみ左クリックでのグラブを抑止。停止・曲アンロード
　　時は曲停止に伴いグラブを自動解除（カーソルが戻る）
　→（20260628 修正）当初は停止中もトグルを禁止していたが、DX9 では停止中でもマウス
　　視点が可能だったため、曲ロード済みなら停止中でもマウスカメラ切替を許可するよう緩和
　→（20260628 修正2）さらに、停止時にマウスカメラ掴みを自動解除していたのを止め、
　　曲アンロード(NoData)時のみ解除に変更。これで停止中もそのままマウスで見回せる
　　（カーソルを戻したい時は左クリックでトグル off）。キーボード(WASD)は元々停止中も可。
・[FIX] シーク（1／2 キー）を DX9 と同等の滑らかなスクロールへ修正
　→移動先まで MovingTimeSpanInMsec かけてスライド（瞬間移動を解消）。
　　本家の _SlidePlaybackTime をそのまま使う形に
・[FIX] 再生終了後にビューが先頭へ戻ってしまう不具合を修正
　→自然終了は末尾に残す（DX9 と同じ）。停止ボタン／ファイル読み込み時のみ
　　先頭へ巻き戻す（自然終了は次回再生時に巻き戻る）
・[FIX] 再生途中で停止して別 MIDI を読み込むと変な位置で表示される不具合を修正
・[FIX] ダッシュボードの文字色を DX9 と同等に修正（[Color] CaptionRGBA を反映）
・[FIX] 再生速度変更（4／5 キー）時に「SPEED:NNN%」をダッシュボードへ表示

────────────────────────────────────────────────────────
Mod 20260623:

*[FIX] Keyboard key animation now matches DX9: a key presses down ahead of the
       note so it is fully pressed exactly when the note sounds (KeyDownDuration)
       and releases over KeyUpDuration after note-off (follows tempo / play
       speed). The note colour is applied only while fully pressed - it snaps on
       at the note onset and off at the note end.
*[FIX] Pressed-key colour now honours the [PianoKeyboard] ActiveKeyColor settings
       (the dedicated Ch-NN-ActiveKeyColor palette + ActiveKeyColorType) instead
       of reusing the note colour. The colour appears at full immediately on
       note-on (no fade-in). CHANNELTRACK (per-track colour) mode keeps the track
       colours on the keys.
*[FIX] Active notes now honour [ActiveNote] EmissiveRGBA (DX9's active-note
       emissive), added on top of the existing white-flash.
*[NEW] Reworked the loading screen: the progress bar shows a percentage and now
       climbs monotonically 0->100%. The (long) note-field build reports live
       progress so the bar/count keep moving for black MIDI, and "Reading MIDI
       file" no longer restarts on every track (whole-file progress).
*[FIX] Guard against the 32-bit item limit (~4.29 billion events / ~2.1 billion
       notes): loading a file past it used to silently wrap and corrupt the data
       (wrong note count). It now stops safely at the limit and asks whether to
       display the portion that was loaded (Yes/No).
*[FIX] The mouse was grabbed (cursor hidden + clipped) on a left click even when
       not playing/monitoring, because the DX11 scene is always NULL. Mouse-look
       now toggles only while playing/paused/monitoring, and the grab is released
       on stop / when the song is unloaded.
*[FIX] Seeking (keys 1/2) now scrolls smoothly to the target over
       MovingTimeSpanInMsec instead of teleporting - it reuses DX9's own
       _SlidePlaybackTime slide rather than an approximation.
*[FIX] After a song finishes the view now stays at the end (DX9 behaviour); it
       rewinds to the start only on the Stop button or when loading a file (a
       natural end rewinds on the next Play).
*[FIX] Loading another MIDI after stopping mid-playback no longer shows the new
       song at a wrong scroll position.
*[FIX] Dashboard text colour now matches DX9 (reads [Color] CaptionRGBA instead
       of forcing solid white).
*[FIX] Changing the playback speed (keys 4/5) now shows "SPEED:NNN%" on the
       dashboard.

────────────────────────────────────────────────────────
改造点 20260622-2：

・[NEW] メニュー「View > Auto save viewpoint」を復活（ON/OFF 切替・状態を保存）
　→旧バージョンにあった自動視点保存をメニューから切り替え可能に
・[FIX] DX11 のライティングを DX9 と同等の 2 灯モデルへ修正（鍵盤等が暗く／
　　灰色がかって見える問題を解消）
　→DX9 の 3D シーンは対向 2 灯（diffuse 1.2）。1 灯だと光と逆向きの面が
　　アンビエントのみで暗くなっていたため、対向フィルライトを追加

────────────────────────────────────────────────────────
Mod 20260622-2:

*[NEW] Restored the "View > Auto save viewpoint" menu item (toggle on/off, the
       state is saved). Brings back the old auto-save-viewpoint as a menu toggle.
*[FIX] Matched the DX11 lighting to DX9's two-light model (fixes the keyboard and
       other meshes looking darker / greyer than DX9). DX9's 3D scene used two
       opposing lights (diffuse 1.2); with a single light, faces turned away from
       it were lit by ambient only, so an opposing fill light was added.

────────────────────────────────────────────────────────
改造点 20260622：

・[FIX] 次の MIDI に切り替えた後、スキップするまで波紋が表示されない不具合を修正
・[FIX] MIDI 再生後にモニタリングへ移行するとライブノートが表示されない不具合を修正
・[NEW] モニタリング（ライブ）でも波紋を表示（リアルタイムのノートオン駆動）

────────────────────────────────────────────────────────
Mod 20260622:

*[FIX] Fixed ripples not loading after switching to the next MIDI (until you skip
       back/forward).
*[FIX] Fixed live notes not showing when entering monitoring after playing a MIDI.
*[NEW] Ripples now show in monitoring (live), driven by real-time note-ons.

────────────────────────────────────────────────────────
改造点 20260621：

・[NEW] 描画エンジンを Direct3D 9（固定機能＋d3dx9）から Direct3D 11 へ全面移植
　→全シーン（PianoRoll 2D/3D/Rain/Rain2D/Ring）をノート・鍵盤・波紋・歌詞・
　　グリッド・タイムインジケータ・ボード・背景画像・星・ダッシュボードまで再実装
　→ノートは GPU インスタンシング描画。Black MIDI（数百万ノート）でも高速
・[NEW] ImGui を Direct3D 11 バックエンドへ移行
・[FIX] DXSDK (June 2010) / d3dx9 への依存を排除し、最新 Windows SDK のみでビルド可能に

・[NEW] オフライン動画出力（ffmpeg へ直接パイプ）
　→コーデック選択、解像度・FPS・品質指定、透過(alpha)出力に対応
・[NEW] 360 度動画出力（エクイレクタングラー 2:1、YouTube 用）
　→現在の視点位置・向きを中心にキューブマップ 6 面を描画して変換
　→出力ダイアログの「360 equirectangular」チェックで有効化
　　※YouTube に 360 と認識させるには球面メタデータの注入が別途必要

・[NEW] ライブモニタ（リアルタイム MIDI 入力）を全シーンで動的描画
　→PianoRoll 2D/3D：流れるノートボックス＋鍵盤反応
　→Rain/Rain2D：落下ノート＋鍵盤反応
　→Ring：円形ノート＋テクスチャボード（Ring に鍵盤は無し）
・[NEW] ライブと再生で設定・視点を完全分離
　→ライブは各シーンの *Live.ini と Viewpoint-*Live セクションを参照
・[NEW] 歌詞（Lyrics メタイベント）表示を DX11 へ移植

・[NEW] ピッチベンド対応を拡充（鍵盤・ノートの音程シフト、チャンネル全体/発音中のみ）
・[NEW] チャンネル×トラック配色、波紋のアンチエイリアス 等の描画調整
・[CHG] ソースコードを Shift-JIS から UTF-8(BOM) へ変換（GitHub 上での文字化け解消）
・[CHG] 配布リポジトリから個人用 conf/data を除外し、動画設定を Video.ini に統一
・[NEW] xmake ビルドを追加（MSBuild ソリューションに加えて）
・[CHG] ImGui をパッケージ管理に移行し最新版(1.92)へ更新。in-tree のソースを廃止し、
　xmake は xrepo、MSBuild は vcpkg(vcpkg.json) から取得（共に静的リンク）

────────────────────────────────────────────────────────
Mod 20260621:

*[NEW] Full port of the renderer from Direct3D 9 (fixed-function + d3dx9) to
       Direct3D 11. Every scene (PianoRoll 2D/3D/Rain/Rain2D/Ring) is
       reimplemented: notes, keyboards, ripple, lyrics, grid, time indicator,
       board, background image, stars and dashboard. Notes use GPU instancing
       and stay fast even for Black MIDI (millions of notes).
*[NEW] ImGui moved to the Direct3D 11 backend.
*[FIX] Dropped the DXSDK (June 2010) / d3dx9 dependency; builds with the modern
       Windows SDK only.

*[NEW] Offline video export (piped straight to ffmpeg): codec selection,
       resolution / FPS / quality, and transparent (alpha) output.
*[NEW] 360-degree video export (equirectangular 2:1, for YouTube). Renders the
       scene into a 6-face cubemap from the current viewpoint, then remaps it so
       the panorama centre is the current view direction. Enable via the
       "360 equirectangular" checkbox in the export dialog. (Spherical metadata
       still has to be injected separately for YouTube to detect it as 360.)

*[NEW] Live monitor (real-time MIDI input) now draws in every scene:
       2D/3D = flowing note boxes + reacting keyboard; Rain/Rain2D = falling
       notes + reacting keyboard; Ring = circular notes + textured board
       (Ring has no piano keyboard).
*[NEW] Live and playback are fully separated: live reads each scene's *Live.ini
       and its own Viewpoint-*Live section.
*[NEW] Ported the lyrics (Lyrics meta event) display to DX11.

*[NEW] Expanded pitch-bend support (keyboard + note pitch shift, whole-channel
       or sounding-notes-only).
*[NEW] Channel x track colouring, ripple anti-aliasing and other rendering
       tweaks.
*[CHG] Converted the source from Shift-JIS to UTF-8 (BOM) so it is not garbled
       on GitHub.
*[CHG] Removed personal conf/data from the published repo and unified the video
       settings into Video.ini.
*[NEW] Added an xmake build (xmake.lua) alongside the MSBuild solution.
*[CHG] ImGui is now package-managed and upgraded to the latest (1.92): the
       in-tree sources are dropped and pulled from xrepo (xmake) / vcpkg
       (MSBuild, vcpkg.json); both link it statically.
