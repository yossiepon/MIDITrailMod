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
// ポート原点Y座標取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginY(
		unsigned char portNo
	)
{
	return 0;
}

//******************************************************************************
// ポート原点Z座標取得
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginZ(
		unsigned char portNo
	)
{

	return 0;
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
		unsigned char portNo,
		unsigned char chNo,
		float scale
	)
{
	float ox, oy, oz = 0.0f;

	//ポート単位の原点座標
	ox = GetPortOriginX(portNo);
	oy = GetPortOriginY(portNo);
	oz = GetPortOriginZ(portNo);

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


