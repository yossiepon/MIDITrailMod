//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesignMod
//
// ピアノキーボードデザインModクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "DXColorUtil.h"
#include "MTConfFile.h"
#include "MTPianoKeyboardDesignMod.h"
#include "MTNoteRippleMod.h"
#include "MTNoteLyrics.h"


//******************************************************************************
// パラメータ定義
//******************************************************************************
//テクスチャ座標算出：ビットマップサイズ = 562 x 562
#define TEXTURE_POINT(x, y)  (D3DXVECTOR2((float)x/561.0f, (float)y/561.0f))

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTPianoKeyboardDesignMod::MTPianoKeyboardDesignMod(void)
{
	//_Initialize()内で基底クラスの初期化処理も呼ばれるため、基底クラスのコンストラクタは呼び出さない
	_Initialize();
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTPianoKeyboardDesignMod::~MTPianoKeyboardDesignMod(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int MTPianoKeyboardDesignMod::Initialize(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	//基底クラスの初期化処理を呼び出す
	MTPianoKeyboardDesign::Initialize(pSceneName, pSeqData);

	m_KeyboardDispNum = m_PortList.GetSize();

	//設定ファイル読み込み
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// シングルキーボードフラグ設定
//******************************************************************************
void MTPianoKeyboardDesignMod::SetKeyboardSingle()
{
	m_isKeyboardSingle = true;
	m_KeyboardDispNum = 1;

	for (int i = 0; i < SM_MAX_PORT_NUM; i++) {
		m_PortIndex[i] = 0;
	}
}

//******************************************************************************
// シングルキーボードフラグ取得
//******************************************************************************
bool MTPianoKeyboardDesignMod::IsKeyboardSingle()
{
	return m_isKeyboardSingle;
}
//******************************************************************************
// キーボード表示数取得
//******************************************************************************
int MTPianoKeyboardDesignMod::GetKeyboardDispNum()
{
	return m_KeyboardDispNum;
}

//******************************************************************************
// ノートからキーボードインデックス取得
//******************************************************************************
int MTPianoKeyboardDesignMod::GetKeyboardIndex(const SMNote &note)
{
	return m_PortIndex[note.portNo];
}

//******************************************************************************
// ポートリストサイズ取得
//******************************************************************************
int MTPianoKeyboardDesignMod::GetPortListSize()
{
	return m_PortList.GetSize();
}

//******************************************************************************
// ノートからポート番号取得
//******************************************************************************
unsigned char MTPianoKeyboardDesignMod::GetPortNo(const SMNote &note)
{
	if (m_isKeyboardSingle) {
		return 0;
	}
	else {
		return note.portNo;
	}
}

//******************************************************************************
// キーボードインデックスからポート番号取得
//******************************************************************************
unsigned char MTPianoKeyboardDesignMod::GetPortNoFromKeyboardIndex(int index)
{
	unsigned char portNo;

	m_PortList.GetPort(index, &portNo);

	return portNo;
}

//******************************************************************************
// 初期化
//******************************************************************************
void MTPianoKeyboardDesignMod::_Initialize()
{
	//基底クラスの初期化処理を呼び出す
	MTPianoKeyboardDesign::_Initialize();

	m_isKeyboardSingle = false;
	m_KeyboardDispNum = 0;

	for (int i = 0; i < 16; i++) {
		m_ActiveKeyColorList[i] = DXColorUtil::MakeColorFromHexRGBA(_T("FF0000FF")); //設定ファイル
	}

	return;
}

//******************************************************************************
// キーボード基準座標取得
//******************************************************************************
D3DXVECTOR3 MTPianoKeyboardDesignMod::GetKeyboardBasePos(
		int keyboardIndex,
		float angle
	)
{
	float ox, oy, oz = 0.0f;

	//ロール角度によって描画方法を切り替える
	angle += angle < 0.0f ? 360.0f : 0.0f;
	bool flip = !((angle > 120.0f) && (angle < 300.0f));

	//ポート単位の原点座標
	ox = GetPortOriginX();
	oy = GetPortOriginY(keyboardIndex, flip);
	oz = GetPortOriginZ(keyboardIndex, flip);

	return D3DXVECTOR3(ox, oy, oz);
}

//******************************************************************************
// ポート原点X座標取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginX()
{
	// angle: 120°～300°(rotateX: 90°, rotateZ: 90°)
	//             +z
	//              |
	// -x<----------0---------->+x
	//              | -RippleMargin
	//         +----+----+
	//         |    |    |  @:OriginX
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//         @----+----+
	//    Note #0   |  #127
	//             -z

	// angle: 0°～120°or 300°～360°(rotateX: -90°, rotateZ: 90°)
	//             +z
	//              |
	//    Note #0   |  #127
	//         +----+----+
	//         |    |    |  @:OriginX
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//         @----+----+
	//              | +RippleMargin
	// -x<----------0---------->+x
	//              |
	//             -z

	float originX = -GetPlaybackSectionHeight() / 2.0f;

	//鍵盤の1/2の幅だけ高音側に移動
	originX += GetWhiteKeyStep() * GetKeyboardResizeRatio() / 2.0f;

	return originX;
}

//******************************************************************************
// ポート原点Y座標取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginY(
		int keyboardIndex,
		bool flip
	)
{
	// angle: 120°～300°(rotateX: 90°, rotateZ: 90°)
	//      +y                                    
	//       |                                    
	//       |                             Ch.15  
	//       |                                    
	//       |            +--------------+        
	//       |      portC +--------------@ Ch.0   
	//       |                             Ch.15  
	//       |                                    
	// +z<---0---------------------------------------------->-z
	//       |            +--------------+        
	//       |      portB +--------------@ Ch.0   
	//       |                             Ch.15  
	//       |                                    
	//       |            +--------------+          @:OriginY(for portA,B,C)
	//       |      portA +--------------@ Ch.0   
	//       |                                    
	//      -y                                    

	// angle: 0°～120°or 300°～360°(rotateX: -90°, rotateZ: 90°)
	//                                           +y
	//                                     Ch.15  |
	//                                            |
	//                    +--------------+        |
	//              portA +--------------@ Ch.0   |
	//                                     Ch.15  |
	//                                            |
	// +z<----------------------------------------0-------------->-z
	//                    +--------------+        |
	//              portB +--------------@ Ch.0   |
	//                                     Ch.15  |
	//                                            |
	//                    +--------------+        |
	//              portC +--------------@ Ch.0   |
	//                                            |
	//                                            | @:OriginY(for portA,B,C)
	//                                           -y

	float portWidth = GetPortWidth();

	int keyboardDispNum = GetKeyboardDispNum();

	float originY;

	if (!flip) {
		originY = -portWidth * (float)(keyboardDispNum - keyboardIndex * 2) / 2.0f;

		//チャネル間隔の62.5%の高さだけ下に
		originY -= GetChStep() * 0.625f;
	}
	else {
		originY = portWidth * (float)(keyboardDispNum - (keyboardIndex + 1) * 2) / 2.0f;

		//チャネル間隔の37.5%の高さだけ上に
		originY += GetChStep() * 0.375f;
	}

	return originY;
}

//******************************************************************************
// ポート原点Z座標取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginZ(
		int keyboardIndex,
		bool flip
	)
{
	// angle: 120°～300°(rotateX: 90°, rotateZ: 90°)
	//             +z
	//              |
	// -x<----------0---------->+x
	//              | -RippleMargin
	//         +----+----+
	//         |    |    |  @:OriginZ
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//         @----+----+
	//    Note #0   |  #127
	//             -z

	// angle: 0°～120°or 300°～360°(rotateX: -90°, rotateZ: 90°)
	//             +z
	//              |
	//    Note #0   |  #127
	//         +----+----+
	//         |    |    |  @:OriginZ
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//         @----+----+
	//              | +RippleMargin
	// -x<----------0---------->+x
	//              |
	//             -z

	float originZ;

	if (!flip) {
		originZ = -(GetWhiteKeyLen() * GetKeyboardResizeRatio() + GetRippleMargin());
	}
	else {
		originZ = GetRippleMargin();
	}

	return originZ;
}

//******************************************************************************
// ノートボックス高さ取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetNoteBoxHeight()
{
	return m_NoteBoxHeight;
}

//******************************************************************************
// ノートボックス幅取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetNoteBoxWidth()
{
	return m_NoteBoxWidth;
}

//******************************************************************************
// ノート間隔取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetNoteStep()
{
	return m_NoteStep;
}

//******************************************************************************
// チャンネル間隔取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetChStep()
{
	return m_ChStep;
}

//******************************************************************************
// キーボード高さ取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetKeyboardHeight()
{
	return GetBlackKeyHeight();
}

//******************************************************************************
// キーボード幅取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetKeyboardWidth()
{
	return GetWhiteKeyStep() * (float)(SM_MAX_NOTE_NUM - 53);
}

//******************************************************************************
// グリッド高さ取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetGridHeight()
{
	return GetNoteStep() * 127.0f;
}

//******************************************************************************
// グリッド幅取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetGridWidth()
{
	return GetChStep() * 15.0f;
}

//******************************************************************************
// ポート高さ取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortHeight()
{
	return GetGridHeight();
}

//******************************************************************************
// ポート幅取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortWidth()
{
	return GetChStep() * 16.0f;
}

//******************************************************************************
// 再生面高さ取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPlaybackSectionHeight()
{
	return GetGridHeight() + GetNoteBoxHeight();
}

//******************************************************************************
// 再生面幅取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPlaybackSectionWidth()
{
	return GetGridWidth() + GetNoteBoxWidth();
}

//******************************************************************************
// 波紋描画間隔取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetRippleSpacing()
{
	return m_RippleSpacing;
}

//******************************************************************************
// 波紋描画マージン取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetRippleMargin()
{
	return GetRippleSpacing() * (MTNOTELYRICS_MAX_LYRICS_NUM + MTNOTERIPPLE_MAX_RIPPLE_NUM);
}

//******************************************************************************
// キーボードリサイズ比取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetKeyboardResizeRatio()
{
	//キーボード基準の相対座標に適用するリサイズ比
	return GetPlaybackSectionHeight() / GetKeyboardWidth();
}

//******************************************************************************
// 発音中キーカラー取得
//******************************************************************************
D3DXCOLOR MTPianoKeyboardDesignMod::GetActiveKeyColor(
		unsigned char chNo,
		unsigned char noteNo,
		unsigned long elapsedTime,
		D3DXCOLOR* pNoteColor
	)
{
	D3DXCOLOR color;
	float r,g,b,a = 0.0f;
	float rate = 0.0f;
	unsigned long duration = 0;

	//          on     off
	//   白 |---+......+---- ←offになったら白鍵の色に戻す
	//      |   :      :
	//      |   :  +---+     ←offになるまで中間色のまま
	//      |   : /:   :
	//      |   :/ :   :
	//   赤 |   +  :   :     ←キー押下直後の色（赤）
	//      |   :\ :   :
	//      |   : \:   :
	//      |   :  +---+     ←offになるまで中間色のまま
	//      |   :  :   :
	//   黒 |---+  :   +---- ←offになったら黒鍵の色に戻す
	//   ---+---*------*-------> +t
	//      |   on :   off
	//          <-->duration

	if ((pNoteColor != NULL) && (m_ActiveKeyColorType == NoteColor)) {
		//ノート色が指定されている場合
		color = *pNoteColor;
	}
	else {
		//それ以外はデフォルト色とする
		color = m_ActiveKeyColorList[chNo];
	}

	duration = (unsigned long)m_ActiveKeyColorDuration;
	rate     = m_ActiveKeyColorTailRate;

	if (elapsedTime < duration) {
		rate = ((float)elapsedTime / (float)duration) * m_ActiveKeyColorTailRate;
	}

	if (GetKeyType(noteNo) == KeyBlack) {
		r = color.r - ((color.r) * rate);
		g = color.g - ((color.g) * rate);
		b = color.b - ((color.b) * rate);
		a = color.a;
	}
	else {
		r = color.r + ((1.0f - color.r) * rate);
		g = color.g + ((1.0f - color.g) * rate);
		b = color.b + ((1.0f - color.b) * rate);
		a = color.a;
	}
	color = D3DXCOLOR(r, g, b, a);

	return color;
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTPianoKeyboardDesignMod::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	TCHAR key[21] = {_T('\0')};
	TCHAR hexColor[16] = {_T('\0')};
	unsigned long i = 0;
	MTConfFile confFile;

	//基底クラスの読み込み処理を呼び出す
	result = MTPianoKeyboardDesign::_LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//設定ファイルを開く
	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//----------------------------------
	//スケール情報
	//----------------------------------
	result = confFile.SetCurSection(_T("Scale"));
	if (result != 0) goto EXIT;

	result = confFile.GetFloat(_T("NoteBoxHeight"), &m_NoteBoxHeight, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("NoteBoxWidth"), &m_NoteBoxWidth, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("NoteStep"), &m_NoteStep, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("ChStep"), &m_ChStep, 0.5f);
	if (result != 0) goto EXIT;

	//----------------------------------
	//色情報
	//----------------------------------
	result = confFile.SetCurSection(_T("PianoKeyboard"));
	if (result != 0) goto EXIT;

	//発音中のキー色情報を取得
	for (i = 0; i < 16; i++) {
		_stprintf_s(key, 21, _T("Ch-%02d-ActiveKeyColor"), i+1);
		result = confFile.GetStr(key, hexColor, 16, _T("FF0000FF"));
		if (result != 0) goto EXIT;

		m_ActiveKeyColorList[i] = DXColorUtil::MakeColorFromHexRGBA(hexColor);
	}

	//----------------------------------
	//波紋情報
	//----------------------------------
	result = confFile.SetCurSection(_T("Ripple"));
	if (result != 0) goto EXIT;

	//波紋描画間隔
	result = confFile.GetFloat(_T("Spacing"), &m_RippleSpacing, 0.002f);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


