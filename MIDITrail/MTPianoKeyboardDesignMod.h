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

	//キーボード基準座標取得
	D3DXVECTOR3 GetKeyboardBasePos(
			int keyboardIndex,
			float angle
		);

	//ポート原点座標取得
	float GetPortOriginX();
	float GetPortOriginY(int keyboardIndex, bool flip);
	float GetPortOriginZ(int keyboardIndex, bool flip);

	//ノートボックス高さ・幅取得
	float GetNoteBoxHeight();
	float GetNoteBoxWidth();

	//ノート間隔取得
	float GetNoteStep();

	//チャンネル間隔取得
	float GetChStep();

	//キーボード高さ・幅取得
	float GetKeyboardHeight();
	float GetKeyboardWidth();

	//グリッド高さ・幅取得
	float GetGridHeight();
	float GetGridWidth();

	//ポート高さ・幅取得
	float GetPortHeight();
	float GetPortWidth();

	//再生面高さ・幅取得
	float GetPlaybackSectionHeight();
	float GetPlaybackSectionWidth();

	//波紋描画間隔取得
	float GetRippleSpacing();

	//波紋描画マージン取得
	float GetRippleMargin();

	//キーボードリサイズ比取得
	float GetKeyboardResizeRatio();

	//発音中キーカラー取得
	D3DXCOLOR GetActiveKeyColor(
			unsigned char chNo,
			unsigned char noteNo,
			unsigned long elapsedTime,
			D3DXCOLOR* pNoteColor = NULL
		);

protected:

	virtual void _Initialize();
	virtual int _LoadConfFile(const TCHAR* pSceneName);

private:

	//ノートボックス高さ
	float m_NoteBoxHeight;
	//ノートボックス幅
	float m_NoteBoxWidth;
	//ノート間隔
	float m_NoteStep;
	//チャンネル間隔
	float m_ChStep;

	//波紋描画間隔
	float m_RippleSpacing;

	//発音中キー色情報
	D3DXCOLOR m_ActiveKeyColorList[16];

};


