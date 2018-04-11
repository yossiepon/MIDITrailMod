//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesignMod
//
// ピアノキーボードデザインModクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardDesign.h"


//******************************************************************************
// ピアノキーボードデザインModクラス
//******************************************************************************
class MTPianoKeyboardDesignMod : public MTPianoKeyboardDesign
{
public:

	//コンストラクタ／デストラクタ
	MTPianoKeyboardDesignMod(void);
	virtual ~MTPianoKeyboardDesignMod(void);

	//初期化
	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	//ポート原点座標取得
	virtual float GetPortOriginY(unsigned char portNo);
	virtual float GetPortOriginZ(unsigned char portNo);

	//チャンネル間隔取得
	float GetChStep();

	//発音中キーカラー取得
	D3DXCOLOR GetActiveKeyColor(unsigned char chNo, unsigned char noteNo, unsigned long elapsedTime);

	//キーボード基準座標取得
	virtual D3DXVECTOR3 GetKeyboardBasePos(unsigned char portNo, unsigned char chNo, float scale);

protected:

	virtual void _Initialize();
	virtual int _LoadConfFile(const TCHAR* pSceneName);

private:

	//チャンネル間隔
	float m_ChStep;

	//発音中キー色情報
	D3DXCOLOR m_ActiveKeyColorList[16];

};


