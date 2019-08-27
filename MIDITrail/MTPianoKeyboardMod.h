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
// パラメータ定義
//******************************************************************************
#define MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX (12)

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
			D3DXVECTOR3 camVector,
			D3DXVECTOR3 lookVector,
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

	//描画情報の作成
	int _MakeRenderingInfo(
			D3DXVECTOR3 basePosVector,
			D3DXVECTOR3 playbackPosVector,
			D3DXVECTOR3 camVector,
			D3DXVECTOR3 lookVector,
			float rollAngle
		);

	//描画インデックスの生成
	int _CreateRenderingIndex(LPDIRECT3DDEVICE9 pD3DDevice);

	//キー単位の描画インデックスの生成
	int _CreateRenderingIndexOfKey(
			unsigned char noteNo,
			int bufferIdx,
			unsigned long* pIndex,
			unsigned long* pRenderingIndex
		);

	//描画インデックスバッファの生成
	int _CreateRenderingIndexBuffer(
				LPDIRECT3DDEVICE9 pD3DDevice,
				int bufferIdx,
				unsigned long indexNum
			);

	//描画インデックスバッファのロック制御
	int _LockRenderingIndex(
				unsigned long** pPtrIndex,
				int bufferIdx,
				unsigned long offset = 0,
				unsigned long size = 0
			);
	int _UnlockRenderingIndex(
				int bufferIdx
			);

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

	//描画インデックス情報
	LPDIRECT3DINDEXBUFFER9 m_pRenderingIndexBuffer[ MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX ];
	unsigned long m_RenderingIndexNum[ MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX ];
	bool m_IsRenderingIndexLocked[ MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX ];

	//キーボード描画情報
	int m_noteNoLow;
	int m_noteNoHigh;
	int m_camDirLR;
	int m_camPosIdx;
};


