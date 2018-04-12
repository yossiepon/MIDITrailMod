//******************************************************************************
//
// MIDITrail / MTTimeIndicator
//
// タイムインジケータ描画クラス
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// 「今再生しているところ」を指し示す再生面を描画する。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXPrimitive.h"
#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
// タイムインジケータ描画クラス
//******************************************************************************
class MTTimeIndicator
{
public:

	//コンストラクタ／デストラクタ
	MTTimeIndicator(void);
	virtual ~MTTimeIndicator(void);

	//生成
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector, float rollAngle);

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

	//移動ベクトル取得
	D3DXVECTOR3 GetMoveVector();

// >>> add 20180404 yossiepon begin
	//表示設定
	void SetEnable(bool isEnable);
// <<< add 20180404 yossiepon end

private:

	DXPrimitive m_Primitive;
	DXPrimitive m_PrimitiveLine;
	float m_CurPos;
	MTNoteDesign m_NoteDesign;
	bool m_isEnableLine;

// >>> add 20180404 yossiepon begin
	//表示可否
	bool m_isEnable;
// <<< add 20180404 yossiepon end

	unsigned long m_CurTickTime;

	//頂点バッファ構造体
	struct MTTIMEINDICATOR_VERTEX {
		D3DXVECTOR3 p;	//頂点座標
		D3DXVECTOR3 n;	//法線
		DWORD		c;	//ディフューズ色
	};

	//頂点バッファFVFフォーマット
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }

	int _CreatePrimitive(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreatePrimitiveLine(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateVertexOfIndicator(MTTIMEINDICATOR_VERTEX* pVertex, unsigned long* pIbIndex);
	int _CreateVertexOfIndicatorLine(MTTIMEINDICATOR_VERTEX* pVertex);

};


