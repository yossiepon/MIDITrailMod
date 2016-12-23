//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3DMod
//
// ピアノロール3Dシーン描画Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTScenePianoRoll3D.h"
#include "MTGridBoxMod.h"
#include "MTNoteBoxMod.h"
#include "MTNoteRippleMod.h"
#include "MTNoteLyrics.h"
#include "MTPianoKeyboardCtrlMod.h"

//******************************************************************************
// ピアノロール3Dシーン描画Modクラス
//******************************************************************************
class MTScenePianoRoll3DMod : public MTScenePianoRoll3D
{
public:

	//コンストラクタ／デストラクタl
	MTScenePianoRoll3DMod();
	virtual ~MTScenePianoRoll3DMod();

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

	virtual void _Reset();

	//シングルキーボードフラグ
	bool m_IsSingleKeyboard;

private:

	// ライト2
	DXDirLight m_DirLightBack;

	//描画オブジェクト
	MTGridBoxMod m_GridBoxMod;
	MTNoteBoxMod m_NoteBoxMod;
	MTNoteRippleMod m_NoteRippleMod;
	MTNoteLyrics m_NoteLyrics;
	MTPianoKeyboardCtrlMod m_PianoKeyboardCtrl;

};

