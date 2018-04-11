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
//最大ポート数
#define MTNOTERIPPLE_MAX_PORT_NUM  (8)

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
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector, float rollAngle);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//解放
	void Release();

	////ノートOFF登録
	//void SetNoteOff(
	//		unsigned char portNo,
	//		unsigned char chNo,
	//		unsigned char noteNo
	//	);

	////ノートON登録
	//void SetNoteOn(
	//		unsigned char portNo,
	//		unsigned char chNo,
	//		unsigned char noteNo,
	//		unsigned char velocity
	//	);

	//演奏チックタイム登録
	void SetCurTickTime(unsigned long curTickTime);

	//演奏時間設定
	void SetPlayTimeMSec(unsigned long playTimeMsec);

	//リセット
	void Reset();

	//表示設定
	void SetEnable(bool isEnable);

	//スキップ状態
	void SetSkipStatus(bool isSkipping);

private:

	////ノート発音状態構造体
	//struct NoteStatus {
	//	bool isActive;
	//	unsigned char portNo;
	//	unsigned char chNo;
	//	unsigned char noteNo;
	//	unsigned char velocity;
	//	unsigned long regTime;
	//};

	//キー状態
	enum KeyStatus {
		BeforeNoteON,
		NoteON,
		AfterNoteOFF
	};

	//発音ノート情報構造体
	struct NoteStatus {
		bool isActive;
		KeyStatus keyStatus;
		unsigned long index;
		float keyDownRate;
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

private:

	//描画系
	DXPrimitive m_Primitive;
	LPDIRECT3DTEXTURE9 m_pTexture;
	D3DMATERIAL9 m_Material;

	//カメラ
	D3DXVECTOR3 m_CamVector;

	//ノートデザイン
	MTNoteDesign m_NoteDesign;

	//ピッチベンド情報
	MTNotePitchBend* m_pNotePitchBend;

	//ノートリスト
	SMNoteList m_NoteListRT;

	//発音中ノート管理
	unsigned long m_PlayTimeMSec;
	unsigned long m_CurTickTime;
	unsigned long m_CurNoteIndex;
	float m_KeyDownRate[MTNOTERIPPLE_MAX_PORT_NUM][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	//ノート発音状態情報
	NoteStatus* m_pNoteStatus;
	unsigned long m_ActiveNoteNum;

	//表示可否
	bool m_isEnable;

	//スキップ状態
	bool m_isSkipping;

	int _CreateTexture(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);
	int _CreateNoteStatus();
	int _CreateVertex(LPDIRECT3DDEVICE9 pD3DDevice);
	int _SetVertexPosition(
				MTNOTERIPPLE_VERTEX* pVertex,
				SMNote note,
				NoteStatus* pNoteStatus,
				unsigned long rippleNo
			);
	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	int _TransformRipple(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateStatusOfRipple(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateNoteStatus(
				unsigned long playTimeMSec,
				unsigned long decayDuration,
				unsigned long releaseDuration,
				SMNote note,
				NoteStatus* pNoteStatus
			);
	int _UpdateVertexOfRipple(LPDIRECT3DDEVICE9 pD3DDevice);

};


