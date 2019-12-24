//******************************************************************************
//
// MIDITrail / MTScenePianoRollRingMod
//
// ピアノロールリングシーン描画Modクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteRippleRingMod.h"
#include "MTGridRingMod.h"
#include "MTTimeIndicatorRingMod.h"
#include "MTScenePianoRollRing.h"

using namespace SMIDILib;


//******************************************************************************
// ピアノロールリングシーン描画Modクラス
//******************************************************************************
class MTScenePianoRollRingMod : public MTScenePianoRollRing
{
public:

	//コンストラクタ／デストラクタl
	MTScenePianoRollRingMod();
	virtual ~MTScenePianoRollRingMod();

	//名称取得
	const TCHAR* GetName();

	//生成
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);

	//変換
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice);

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//破棄
	virtual void Release();

	//シーケンサメッセージ受信
	virtual int OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	);

	//エフェクト設定
	virtual void SetEffect(MTScene::EffectType type, bool isEnable);

protected:

	//描画オブジェクト
	MTNoteRippleRingMod m_NoteRippleMod;
	MTGridRingMod m_GridRingMod;
	MTTimeIndicatorRingMod m_TimeIndicatorMod;

	//シーンリセット
	virtual void _Reset();

private:

};

