//******************************************************************************
//
// MIDITrail / MTNoteRain
//
// ノートレイン描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ノートレインを描画する。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"
#include "MTPianoKeyboardDesign.h"

using namespace SMIDILib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
//最大発音ノート描画数
#define MTNOTERAIN_MAX_ACTIVENOTE_NUM  (100)

// TODO: 最大発音ノート描画数を可変にする
//   事前にシーケンスデータの最大同時発音数を調査しておけば
//   確保するバッファサイズを変更できる
//   現状でもバッファサイズは初期化時点で動的に変更可能である


//******************************************************************************
// ノートレイン描画クラス
//******************************************************************************
class MTNoteRain
{
public:

	//コンストラクタ／デストラクタ
	MTNoteRain(void);
	virtual ~MTNoteRain(void);

	//生成
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//解放
	void Release();

	//演奏チックタイム登録
	void SetCurTickTime(unsigned long curTickTime);

	//リセット
	void Reset();

	//現在位置取得
	float GetPos();

	//スキップ状態
	void SetSkipStatus(bool isSkipping);

private:

	//発音ノート情報構造体
	struct NoteStatus {
		bool isActive;
		unsigned long index;
		unsigned long startTime;
	};

	//頂点バッファ構造体
	struct MTNOTERAIN_VERTEX {
		D3DXVECTOR3 p;	//頂点座標
		D3DXVECTOR3 n;	//法線
		DWORD       c;	//ディフューズ色
	};

private:

	//ノートデザイン
	MTNoteDesign m_NoteDesign;

	//キーボードデザイン
	MTPianoKeyboardDesign m_KeyboardDesign;

	//ノートリスト
	SMNoteList m_NoteList;

	//全ノートレイン
	DXPrimitive m_PrimitiveAllNotes;

	//発音中ノートボックス
	unsigned long m_CurTickTime;
	unsigned long m_CurNoteIndex;
	NoteStatus* m_pNoteStatus;
	float m_CurPos;

	//スキップ状態
	bool m_isSkipping;

	//ピッチベンド情報
	MTNotePitchBend* m_pNotePitchBend;

	//頂点バッファFVFフォーマット
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreateAllNoteRain(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateVertexOfNote(
				SMNote note,
				MTNOTERAIN_VERTEX* pVertex,
				unsigned long vertexOffset,
				unsigned long* pIndex
			);
	int _CreateNoteStatus();
	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	int _TransformActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateStatusOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateVertexOfNote(
				unsigned long index,
				bool isEnablePitchBendShift = false
			);

};


