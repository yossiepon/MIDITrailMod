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

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//更新
	int Transform(
			LPDIRECT3DDEVICE9 pD3DDevice,
			D3DXVECTOR3 basePosVector,
			D3DXVECTOR3 playbackPosVector,
			float rollAngle
		);

	//キー状態変更
	virtual int PushKey(
			unsigned char chNo,
			unsigned char noteNo,
			float keyDownRate,
			unsigned long elapsedTime,
			D3DXCOLOR* pActiveKeyColor = NULL
		);

private:

	//逆順インデックスの生成
	int _CreateRevIndex(LPDIRECT3DDEVICE9 pD3DDevice);

	//キー単位の逆順インデックスの生成
	int _CreateRevIndexOfKey(
			unsigned char noteNo,
			unsigned long* pIndex,
			unsigned long* pRevIndex
		);

	//逆順インデックスバッファの生成
	int _CreateRevIndexBuffer(LPDIRECT3DDEVICE9 pD3DDevice, unsigned long indexNum);

	//逆順インデックスバッファのロック制御
	int _LockRevIndex(unsigned long** pPtrIndex, unsigned long offset = 0, unsigned long size = 0);
	int _UnlockRevIndex();

	virtual int _CreateVertexOfKeyWhite1(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyWhite2(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyWhite3(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyBlack(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);

private:

	//キーボードデザイン
	MTPianoKeyboardDesignMod m_KeyboardDesignMod;

	//逆順インデックス情報
	LPDIRECT3DINDEXBUFFER9 m_pRevIndexBuffer;
	unsigned long m_RevIndexNum;
	bool m_IsRevIndexLocked;
};


