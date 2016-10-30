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

	//波紋表示時間取得
	unsigned long GetRippleDecayDuration();
	unsigned long GetRippleReleaseDuration();

	//波紋サイズ取得
	float GetRippleHeight(float rate);
	float GetRippleWidth(float rate);
	float GetRippleAlpha(float rate);
	float GetDecayCoefficient(float rate);

	//ポート原点座標取得
	virtual float GetPortOriginZ(unsigned char portNo);

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

	int m_RippleDecayDuration;
	int m_RippleReleaseDuration;

};


