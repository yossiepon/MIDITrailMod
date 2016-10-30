//******************************************************************************
//
// MIDITrail / MTNoteBoxLive
//
// ライブモニタ用ノートボックス描画クラス
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ライブモニタ用ノートボックスを描画する。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXPrimitive.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"


//******************************************************************************
// パラメータ定義
//******************************************************************************
//最大発音ノート描画数
#define MTNOTEBOX_MAX_LIVENOTE_NUM  (2048)

// TODO: 最大ノート描画数を可変にする

//******************************************************************************
// ライブモニタ用ノートボックス描画クラス
//******************************************************************************
class MTNoteBoxLive
{
public:
	
	//コンストラクタ／デストラクタ
	MTNoteBoxLive(void);
	virtual ~MTNoteBoxLive(void);
	
	//生成
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			MTNotePitchBend* pNotePitchBend
		);
	
	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);
	
	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//解放
	void Release();
	
	//リセット
	void Reset();
	
	//ノートON登録
	void SetNoteOn(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo,
			unsigned char velocity
		);
	
	//ノートOFF登録
	void SetNoteOff(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo
		);
	
	//全ノートOFF
	void AllNoteOff();
	void AllNoteOffOnCh(unsigned char portNo, unsigned char chNo);
	
private:
	
	//発音ノート情報構造体
	struct NoteStatus {
		bool isActive;
		unsigned char portNo;
		unsigned char chNo;
		unsigned char noteNo;
		unsigned long startTime;
		unsigned long endTime;
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
	
	//ノートボックス
	DXPrimitive m_PrimitiveNotes;
	unsigned long m_NoteNum;
	NoteStatus* m_pNoteStatus;
	
	//ピッチベンド情報
	MTNotePitchBend* m_pNotePitchBend;
	
	//ライブモニタ表示期間（ミリ秒）
	unsigned long m_LiveMonitorDisplayDuration;
	
	//頂点バッファFVFフォーマット
	unsigned long _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }
	
	int _CreateNoteBox(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateNoteStatus();
	
	int _CreateVertexOfNote(
			NoteStatus noteStatus,
			MTNOTEBOX_VERTEX* pVertex,
			unsigned long vertexOffset,
			unsigned long* pIbIndex,
			unsigned long curTime,
			bool isEnablePitchBend = false
		);
	unsigned long _GetVertexIndexOfNote(unsigned long index);
	
	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	
	int _TransformNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateStatusOfNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateVertexOfNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	
	void _ClearOldestNoteStatus(unsigned long* pCleardIndex);
	
};


