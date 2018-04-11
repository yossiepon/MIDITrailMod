//******************************************************************************
//
// MIDITrail / MTNoteDesignMod
//
// ノートデザインModクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteDesign.h"
//******************************************************************************
// ノートデザインModクラス
//******************************************************************************
class MTNoteDesignMod : public MTNoteDesign
{
public:

	//コンストラクタ／デストラクタ
	MTNoteDesignMod(void);
	virtual ~MTNoteDesignMod(void);

	//初期化
	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	//波紋表示時間取得
	unsigned long GetRippleDecayDuration();
	unsigned long GetRippleReleaseDuration();

	//波紋描画情報取得
	unsigned long GetRippleOverwriteTimes();
	float GetRippleSpacing();

	//波紋サイズ取得
	float GetRippleHeight(float rate);
	float GetRippleWidth(float rate);
	float GetRippleAlpha(float rate);
	float GetDecayCoefficient(
				float rate,					//サイズ比率
				float saturation = 20.0f	//飽和レベル
			);

	//発音中ノートボックス頂点座標取得
	void GetActiveNoteBoxVirtexPos(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
				D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
				D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
				D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
				short pitchBendValue = 0,				//省略可：ピッチベンド
				unsigned char pitchBendSensitivity = 0,	//省略可：ピッチベンド感度
				float rate = 0.0f						//省略可：サイズ比率
			);

	//発音中ノートボックスカラー取得
	D3DXCOLOR GetActiveNoteBoxColor(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				float rate
			);

protected:

	virtual void _Clear();
	virtual int _LoadConfFile(const TCHAR* pSceneName);

private:

	//ディケイ時間
	int m_RippleDecayDuration;
	//リリース時間
	int m_RippleReleaseDuration;

	//上書き回数
	int m_RippleOverwriteTimes;
	//描画間隔
	float m_RippleSpacing;
};


