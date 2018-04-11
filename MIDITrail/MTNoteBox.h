//******************************************************************************
//
// MIDITrail / MTNoteBox
//
// ノートボックス描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ノートボックスを描画する。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"

using namespace SMIDILib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
//最大ポート数
#define MT_NOTEBOX_MAX_PORT_NUM  (8)

//最大発音ノート描画数
#define MTNOTEBOX_MAX_ACTIVENOTE_NUM  (100)

// TODO: 最大発音ノート描画数を可変にする
//   事前にシーケンスデータの最大同時発音数を調査しておけば
//   確保するバッファサイズを変更できる
//   現状でもバッファサイズは初期化時点で動的に変更可能である


//******************************************************************************
// ノートボックス描画クラス
//******************************************************************************
class MTNoteBox
{
public:

	//コンストラクタ／デストラクタ
	MTNoteBox(void);
	virtual ~MTNoteBox(void);

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

	//演奏時間設定
	void SetPlayTimeMSec(unsigned long playTimeMsec);

	//リセット
	void Reset();

	//スキップ状態
	void SetSkipStatus(bool isSkipping);

private:

	//キー状態
	enum KeyStatus {
		BeforeNoteON,
		NoteON,
		AfterNoteOFF
	};

	//発音ノート情報構造体
	struct NoteStatus {
		bool isActive;
		bool isHide;
		KeyStatus keyStatus;
		unsigned long index;
		float keyDownRate;
		//unsigned long startTime;
	};

	//頂点バッファ構造体
	struct MTNOTEBOX_VERTEX {
		D3DXVECTOR3 p;	//頂点座標
		D3DXVECTOR3 n;	//法線
		DWORD		c;	//ディフューズ色
	};

private:

	//ノートデザイン
	MTNoteDesign m_NoteDesign;

	//ノートリスト
	SMNoteList m_NoteList;
	SMNoteList m_NoteListRT;

	//全ノートボックス
	DXPrimitive m_PrimitiveAllNotes;

	//発音中ノートボックス
	DXPrimitive m_PrimitiveActiveNotes;

	//発音中ノート管理
	unsigned long m_PlayTimeMSec;
	unsigned long m_CurTickTime;
	unsigned long m_CurNoteIndex;
	float m_KeyDownRate[MT_NOTEBOX_MAX_PORT_NUM][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	//ノート発音状態情報
	NoteStatus* m_pNoteStatus;
	unsigned long m_ActiveNoteNum;

	//スキップ状態
	bool m_isSkipping;

		//ピッチベンド情報
	MTNotePitchBend* m_pNotePitchBend;

	//頂点バッファFVFフォーマット
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreateAllNoteBox(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateActiveNoteBox(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateNoteStatus();

	int _CreateVertexOfNote(
			SMNote note,
			MTNOTEBOX_VERTEX* pVertex,
			unsigned long vertexOffset,
			unsigned long* pIbIndex,
			float keyDownRate = 0.0f,
			bool isEnablePitchBend = false
		);
	unsigned long _GetVertexIndexOfNote(unsigned long index);

	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	void _MakeMaterialForActiveNote(D3DMATERIAL9* pMaterial);

	int _TransformActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateStatusOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateNoteStatus(
				unsigned long playTimeMSec,
				unsigned long decayDuration,
				unsigned long releaseDuration,
				SMNote note,
				NoteStatus* pNoteStatus
			);
	int _UpdateVertexOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);

	int _HideNoteBox(unsigned long index);
	int _ShowNoteBox(unsigned long index);

};


