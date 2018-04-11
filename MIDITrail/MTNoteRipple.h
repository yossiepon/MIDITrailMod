//******************************************************************************
//
// MIDITrail / MTNoteRipple
//
// ノート波紋描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

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
//最大波紋描画数
#define MTNOTERIPPLE_MAX_RIPPLE_NUM  (100)

// TODO: 最大波紋描画数を可変にする
//   事前にシーケンスデータの最大同時発音数を調査しておけば
//   確保するバッファサイズを変更できる
//   現状でもバッファサイズは初期化時点で動的に変更可能である


//******************************************************************************
// ノート波紋描画クラス
//******************************************************************************
class MTNoteRipple
{
public:

	//コンストラクタ／デストラクタ
	MTNoteRipple(void);
	virtual ~MTNoteRipple(void);

	//生成
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//更新
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector, float rollAngle);

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//解放
	virtual void Release();

	//ノートOFF登録
	void SetNoteOff(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo
		);

	//ノートON登録
	void SetNoteOn(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo,
			unsigned char velocity
		);

	//演奏チックタイム登録
	void SetCurTickTime(unsigned long curTickTime);

	//リセット
	virtual void Reset();

	//表示設定
	void SetEnable(bool isEnable);

	//スキップ状態
	void SetSkipStatus(bool isSkipping);

protected:

	//ノート発音状態構造体
	struct NoteStatus {
		bool isActive;
		unsigned char portNo;
		unsigned char chNo;
		unsigned char noteNo;
		unsigned char velocity;
		unsigned long regTime;
	};

	//頂点バッファ構造体
	struct MTNOTERIPPLE_VERTEX {
		D3DXVECTOR3 p;	//頂点座標
		D3DXVECTOR3 n;	//法線
		DWORD		c;	//ディフューズ色
		D3DXVECTOR2 t;	//テクスチャ画像位置
	};

	//頂点バッファFVFフォーマット
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

protected:

	//描画系
	DXPrimitive m_Primitive;
	LPDIRECT3DTEXTURE9 m_pTexture;

	//再生時刻
	unsigned long m_CurTickTime;

	//カメラ
	D3DXVECTOR3 m_CamVector;

	//ノートデザイン
	MTNoteDesign m_NoteDesign;

	//ピッチベンド情報
	MTNotePitchBend* m_pNotePitchBend;

	//ノート発音状態情報
	unsigned long m_ActiveNoteNum;

	//表示可否
	bool m_isEnable;

	//スキップ状態
	bool m_isSkipping;

	virtual int _CreateNoteStatus();
	virtual int _CreateVertex(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual void _MakeMaterial(D3DMATERIAL9* pMaterial);
	virtual int _TransformRipple(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual int _UpdateVertexOfRipple(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	//描画系
	D3DMATERIAL9 m_Material;

	//ノート発音状態情報
	NoteStatus* m_pNoteStatus;

	int _CreateTexture(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);
	int _SetVertexPosition(
				MTNOTERIPPLE_VERTEX* pVertex,
				NoteStatus* pNoteStatus,
				unsigned long rippleNo,
				unsigned long curTime,
				bool* pIsTimeout
			);
};


