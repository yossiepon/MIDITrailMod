//******************************************************************************
//
// MIDITrail / MTNoteDesignMod
//
// ノートデザインModクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTConfFile.h"
#include "MTNoteDesignMod.h"


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteDesignMod::MTNoteDesignMod(void)
{
	// 基底クラスの_Clear()も呼ばれるため、基底クラスのコンストラクタは呼ばない
	_Clear();
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteDesignMod::~MTNoteDesignMod(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int MTNoteDesignMod::Initialize(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	//基底クラスの初期化処理を呼び出す
	MTNoteDesign::Initialize(pSceneName, pSeqData);

	//パラメータ設定ファイル読み込み
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 波紋ディケイ時間取得(msec)
//******************************************************************************
unsigned long MTNoteDesignMod::GetRippleDecayDuration()
{
	return (unsigned long)m_RippleDecayDuration;
}

//******************************************************************************
// 波紋リリース時間取得(msec)
//******************************************************************************
unsigned long MTNoteDesignMod::GetRippleReleaseDuration()
{
	return (unsigned long)m_RippleReleaseDuration;
}

//******************************************************************************
// 波紋上書き回数
//******************************************************************************
unsigned long MTNoteDesignMod::GetRippleOverwriteTimes()
{
	return (unsigned long)m_RippleOverwriteTimes;
}

//******************************************************************************
// 波紋描画間隔
//******************************************************************************
float MTNoteDesignMod::GetRippleSpacing()
{
	return m_RippleSpacing;
}

//******************************************************************************
// 波紋縦サイズ取得
//******************************************************************************
float MTNoteDesignMod::GetRippleHeight(
		float rate	//サイズ比率
	)
{
	return m_RippleHeight * GetDecayCoefficient(rate);
}

//******************************************************************************
// 波紋横サイズ取得
//******************************************************************************
float MTNoteDesignMod::GetRippleWidth(
		float rate	//サイズ比率
	)
{
	return m_RippleWidth * GetDecayCoefficient(rate);
}

//******************************************************************************
// 波紋透明度取得
//******************************************************************************
float MTNoteDesignMod::GetRippleAlpha(
		float rate	//サイズ比率
	)
{
	return GetDecayCoefficient(rate);
}

//******************************************************************************
// 減衰係数取得
//******************************************************************************
float MTNoteDesignMod::GetDecayCoefficient(
		float rate,			//サイズ比率
		float saturation	//飽和レベル
	)
{
	float coeff = 1.0f;

	if(rate < 0.5f) {
		coeff = (pow(2.0f, (0.5f - rate) * 8.0f) + 14.0f) / saturation;
	} else {
		coeff = (16.0f - pow(2.0f, (rate - 0.5f) * 8.0f)) / saturation;
	}

	coeff = coeff > 1.0f ? 1.0f : coeff;

	return coeff;
}

//******************************************************************************
// 発音中ノートボックス頂点座標取得
//******************************************************************************
void MTNoteDesignMod::GetActiveNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
		D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
		D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
		D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
		short pitchBendValue,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity,	//省略可：ピッチベンド感度
		float rate							//省略可：サイズ比率
	)
{
	D3DXVECTOR3 center;
	float bh, bw = 0.0f;
	float curSizeRatio = 1.0f;
	
	center = GetNoteBoxCenterPosX(curTickTime, portNo, chNo, noteNo, pitchBendValue, pitchBendSensitivity);
	
	if (rate > 0.0f) {
		curSizeRatio = 1.0f + (m_ActiveNoteBoxSizeRatio - 1.0f) * GetDecayCoefficient(rate, 30.0f);
	}
	
	bh = GetNoteBoxHeight() * curSizeRatio;
	bw = GetNoteBoxWidht() * curSizeRatio;
	
	*pVector0 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z+(bw/2.0f));
	*pVector1 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z-(bw/2.0f));
	*pVector2 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z+(bw/2.0f));
	*pVector3 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z-(bw/2.0f));
}

//******************************************************************************
// 発音中ノートボックスカラー取得
//******************************************************************************
D3DXCOLOR MTNoteDesignMod::GetActiveNoteBoxColor(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		float rate
	)
{
	float alpha = GetDecayCoefficient(rate, 30.0f);

	D3DXCOLOR color;
	float r,g,b,a = 0.0f;

	color = GetNoteBoxColor(portNo, chNo, noteNo);

	//m_ActiveNoteDuration リリースタイム
	//  発音開始時点からノート色を元に戻すまでの時間
	//  ただし m_ActiveNoteEmissive によってリリース後もノートOFFまで発光する

	//m_ActiveNoteWhiteRate 最大白色率
	//  0.0 → ノート色変化なし
	//  0.5 → ノート色と白の中間色
	//  1.0 → 白

	r = color.r + ((1.0f - color.r) * alpha * m_ActiveNoteWhiteRate);
	g = color.g + ((1.0f - color.g) * alpha * m_ActiveNoteWhiteRate);
	b = color.b + ((1.0f - color.b) * alpha * m_ActiveNoteWhiteRate);
	a = color.a;
	color = D3DXCOLOR(r, g, b, a);

	return color;
}

//******************************************************************************
// クリア
//******************************************************************************
void MTNoteDesignMod::_Clear(void)
{
	MTNoteDesign::_Clear();

	m_RippleDecayDuration = 100;
	m_RippleReleaseDuration = 250;
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTNoteDesignMod::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	MTConfFile confFile;

	//基底クラスの読み込み処理を呼び出す
	result = MTNoteDesign::_LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//設定ファイルを開く
	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//----------------------------------
	//波紋情報
	//----------------------------------
	result = confFile.SetCurSection(_T("Ripple"));
	if (result != 0) goto EXIT;

	//波紋ディケイ時間(msec)
	result = confFile.GetInt(_T("DecayDuration"), &m_RippleDecayDuration, 100);
	if (result != 0) goto EXIT;

	//波紋リリース時間(msec)
	result = confFile.GetInt(_T("ReleaseDuration"), &m_RippleReleaseDuration, 250);
	if (result != 0) goto EXIT;

	//波紋上書き回数
	result = confFile.GetInt(_T("OverwriteTimes"), &m_RippleOverwriteTimes, 3);
	if (result != 0) goto EXIT;

	//波紋描画間隔
	result = confFile.GetFloat(_T("Spacing"), &m_RippleSpacing, 0.002f);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

