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
// >>> modify 20120728 yossiepon begin
	virtual int Create(
// <<< modify 20120728 yossiepon end
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//更新
// >>> modify 20120728 yossiepon begin
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);
// <<< modify 20120728 yossiepon end

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//解放
// >>> modify 20120728 yossiepon begin
	virtual void Release();
// <<< modify 20120728 yossiepon end

	//演奏チックタイム登録
	void SetCurTickTime(unsigned long curTickTime);

	//リセット
// >>> modify 20120728 yossiepon begin
	virtual void Reset();
// <<< modify 20120728 yossiepon end

	//スキップ状態
	void SetSkipStatus(bool isSkipping);

// >>> modify 20120728 yossiepon begin
protected:

	//頂点バッファ構造体
	struct MTNOTEBOX_VERTEX {
		D3DXVECTOR3 p;	//頂点座標
		D3DXVECTOR3 n;	//法線
		DWORD		c;	//ディフューズ色
	};

protected:

	//ノートデザイン
	MTNoteDesign m_NoteDesign;

	//ノートリスト
	SMNoteList m_NoteList;

	//全ノートボックス
	DXPrimitive m_PrimitiveAllNotes;

	//発音中ノートボックス
	DXPrimitive m_PrimitiveActiveNotes;
	unsigned long m_CurTickTime;
	unsigned long m_CurNoteIndex;
	unsigned long m_ActiveNoteNum;

	//ピッチベンド情報
	MTNotePitchBend* m_pNotePitchBend;

	virtual int _CreateNoteStatus();

	int _CreateVertexOfNote(
			SMNote note,
			MTNOTEBOX_VERTEX* pVertex,
			unsigned long vertexOffset,
			unsigned long* pIbIndex,
			unsigned long elapsedTime = 0xFFFFFFFF,
			bool isEnablePitchBend = false
		);

	int _TransformActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual int _UpdateStatusOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual int _UpdateVertexOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);

	int _HideNoteBox(unsigned long index);
	int _ShowNoteBox(unsigned long index);

// <<< modify 20120728 yossiepon end

private:

	//発音ノート情報構造体
	struct NoteStatus {
		bool isActive;
		bool isHide;
		unsigned long index;
		unsigned long startTime;
	};

// >>> modify 20120728 yossiepon begin
private:

	//発音中ノートボックス
	NoteStatus* m_pNoteStatus;

	//スキップ状態
	bool m_isSkipping;

	//頂点バッファFVFフォーマット
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreateAllNoteBox(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateActiveNoteBox(LPDIRECT3DDEVICE9 pD3DDevice);

	unsigned long _GetVertexIndexOfNote(unsigned long index);

	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	void _MakeMaterialForActiveNote(D3DMATERIAL9* pMaterial);

// <<< modify 20120728 yossiepon end

};


