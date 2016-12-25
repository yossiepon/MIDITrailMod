//******************************************************************************
//
// MIDITrail / MTNoteRippleMod
//
// ノート波紋描画Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteRipple.h"
#include "MTNoteDesignMod.h"


//******************************************************************************
// パラメータ定義
//******************************************************************************
//最大ポート数
#define MTNOTERIPPLE_MAX_PORT_NUM  (8)


//******************************************************************************
// ノート波紋描画Modクラス
//******************************************************************************
class MTNoteRippleMod : public MTNoteRipple
{
public:

	//コンストラクタ／デストラクタ
	MTNoteRippleMod(void);
	virtual ~MTNoteRippleMod(void);

	//生成
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//解放
	virtual void Release();

	//演奏時間設定
	void SetPlayTimeMSec(unsigned long playTimeMsec);

	//リセット
	virtual void Reset();

protected:

	virtual int _CreateNoteStatus();
	virtual int _CreateVertex(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual void _MakeMaterial(D3DMATERIAL9* pMaterial);
	virtual int _TransformRipple(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual int _UpdateVertexOfRipple(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	//ノート発音状態構造体
	//キー状態
	enum KeyStatus {
		BeforeNoteON,
		NoteON,
		AfterNoteOFF
	};

	//発音ノート情報構造体
	struct NoteStatusMod {
		bool isActive;
		KeyStatus keyStatus;
		unsigned long index;
		float keyDownRate;
	};

private:

	//ノートデザイン
	MTNoteDesignMod m_NoteDesignMod;

	//ノートリスト
	SMNoteList m_NoteListRT;

	//発音中ノート管理
	unsigned long m_PlayTimeMSec;
	unsigned long m_CurNoteIndex;
	float m_KeyDownRate[MTNOTERIPPLE_MAX_PORT_NUM][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	//ノート発音状態情報
	NoteStatusMod* m_pNoteStatusMod;

	int _SetVertexPosition(
				MTNOTERIPPLE_VERTEX* pVertex,
				SMNote note,
				NoteStatusMod* pNoteStatus,
				unsigned long rippleNo
			);
	int _UpdateStatusOfRipple(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateNoteStatus(
				unsigned long playTimeMSec,
				unsigned long decayDuration,
				unsigned long releaseDuration,
				SMNote note,
				NoteStatusMod* pNoteStatus
			);
};


