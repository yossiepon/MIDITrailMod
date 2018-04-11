//******************************************************************************
//
// MIDITrail / MTPianoKeyboardMod
//
// ピアノキーボード描画Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboard.h"
#include "MTPianoKeyboardDesignMod.h"


//******************************************************************************
// ピアノキーボード描画Modクラス
//******************************************************************************
class MTPianoKeyboardMod : public MTPianoKeyboard
{
public:

	//コンストラクタ／デストラクタ
	MTPianoKeyboardMod(void);
	virtual ~MTPianoKeyboardMod(void);

	//生成
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			LPDIRECT3DTEXTURE9 pTexture = NULL
		);

	//更新
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 moveVector1, D3DXVECTOR3 moveVector2, float scale, float z, float rollAngle);

	//キー状態変更
	virtual int PushKey(unsigned char chNo, unsigned char noteNo, float keyDownRate, unsigned long elapsedTime);

private:

	//キーボードデザイン
	MTPianoKeyboardDesignMod m_KeyboardDesignMod;

};


