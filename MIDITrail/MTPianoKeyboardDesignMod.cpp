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

	//設定ファイル読み込み
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 初期化
//******************************************************************************
void MTPianoKeyboardDesignMod::_Initialize()
{
	unsigned long i = 0;

	//基底クラスの初期化処理を呼び出す
	MTPianoKeyboardDesign::_Initialize();

	for (i = 0; i < 16; i++) {
		m_ActiveKeyColorList[i] = DXColorUtil::MakeColorFromHexRGBA(_T("FF0000FF")); //設定ファイル
	}

	return;
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

	float originX = MTPianoKeyboardDesign::GetPortOriginX(0);

	//鍵盤の1/2の幅だけ高音側に移動
	return originX + GetWhiteKeyStep() / 2.0f;
}

//******************************************************************************
// ポート原点Y座標取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginY(
		int keyboardIndex,
		float antiResizeScale,
		bool flip
	)
{
	// angle: 120°～300°(rotateX: 90°, rotateZ: 90°)
	//      +y                                    
	//       |                                    
	// +z<---0-----------------------------Ch.15------------>-z
	//       |                                    
	//       |            +--------------+        
	//       |      portC +--------------@ Ch.0   
	//       |                             Ch.15  
	//       |                                    
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
	//                    +--------------+        |
	//              portB +--------------@ Ch.0   |
	//                                     Ch.15  |
	//                                            |
	//                    +--------------+        |
	// +z<----------portC +--------------@ Ch.0---0-------------->-z
	//                                            |
	//                                            | @:OriginY(for portA,B,C)
	//                                           -y

	float portWidth = GetChStep() * 16.0f;

	// TODO シングルキーボードの判定方法を再検討
	int keyboardDispNum = GetKeyboardMaxDispNum() > 1 ? m_PortList.GetSize() : 1;

	float originY = portWidth * (keyboardDispNum - keyboardIndex - 1);

	if (!flip) {
		originY = -(originY + GetChStep() * 15.0f);
	}

	//キーボードリサイズ後に正しいポート間隔となるよう逆比をかける
	originY *= antiResizeScale;

	//鍵盤の1/4の高さだけ下に
	originY -= GetWhiteKeyHeight() / 4.0f;

	return originY;
}

//******************************************************************************
// ポート原点Z座標取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginZ(
		int keyboardIndex,
		float rippleMargin,
		float antiResizeScale,
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

	//キーボードリサイズ後に正しいリップルマージンとなるよう逆比をかける
	rippleMargin *= antiResizeScale;

	if (!flip) {
		originZ = -(GetWhiteKeyLen() + rippleMargin);
	}
	else {
		originZ = rippleMargin;
	}

	return originZ;
}

//******************************************************************************
// チャンネル間隔取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetChStep()
{
	return m_ChStep;
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
// キーボード基準座標取得
//******************************************************************************
D3DXVECTOR3 MTPianoKeyboardDesignMod::GetKeyboardBasePos(
		int keyboardIndex,
		float rippleMargin,
		float boardHeight,
		float angle
	)
{
	float ox, oy, oz = 0.0f;

	//ロール角度によって描画方法を切り替える
	angle += angle < 0.0f ? 360.0f : 0.0f;
	bool flip = !((angle > 120.0f) && (angle < 300.0f));

	float keyboardWidth = MTPianoKeyboardDesign::GetPortOriginX(0) * -2.0f;

	//キーボード描画時にリサイズがかかってはならない相対座標用の逆リサイズ比
	//対象：ポート間隔、リップルマージン
	float antiResizeScale = keyboardWidth / boardHeight;

	//ポート単位の原点座標
	ox = GetPortOriginX();
	oy = GetPortOriginY(keyboardIndex, antiResizeScale, flip);
	oz = GetPortOriginZ(keyboardIndex, rippleMargin, antiResizeScale, flip);

	return D3DXVECTOR3(ox, oy, oz);
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

EXIT:;
	return result;
}


