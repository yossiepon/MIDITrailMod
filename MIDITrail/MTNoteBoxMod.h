//******************************************************************************
//
// MIDITrail / MTNoteBoxMod
//
// ノートボックス描画Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteBox.h"
#include "MTNoteDesignMod.h"


//******************************************************************************
// パラメータ定義
//******************************************************************************
//最大ポート数
#define MT_NOTEBOX_MAX_PORT_NUM  (8)


//******************************************************************************
// ノートボックス描画Modクラス
//******************************************************************************
class MTNoteBoxMod : public MTNoteBox
{
public:

	//コンストラクタ／デストラクタ
	MTNoteBoxMod(void);
	virtual ~MTNoteBoxMod(void);

	//生成
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//更新
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

	//演奏時間設定
	void SetPlayTimeMSec(unsigned long playTimeMsec);

	//解放
	virtual void Release();

	//リセット
	virtual void Reset();

private:

	//キー状態
	enum KeyStatus {
		BeforeNoteON,
		NoteON,
		AfterNoteOFF
	};

	//発音ノート情報構造体
	struct NoteStatusMod {
		bool isActive;
		bool isHide;
		unsigned long index;
		KeyStatus keyStatus;
		float keyDownRate;
	};

protected:

	virtual int _UpdateStatusOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	virtual int _UpdateVertexOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	//ノートデザイン
	MTNoteDesignMod m_NoteDesignMod;

	//ノートリスト
	SMNoteList m_NoteListRT;

	//発音中ノート管理
	unsigned long m_PlayTimeMSec;
	float m_KeyDownRate[MT_NOTEBOX_MAX_PORT_NUM][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	//ノート発音状態情報
	NoteStatusMod* m_pNoteStatusMod;

	virtual int _CreateNoteStatus();

	int _CreateVertexOfNote(
			SMNote note,
			MTNOTEBOX_VERTEX* pVertex,
			unsigned long vertexOffset,
			unsigned long* pIbIndex,
			float keyDownRate = 0.0f,
			bool isEnablePitchBend = false
		);

	int _UpdateNoteStatus(
			unsigned long playTimeMSec,
			unsigned long decayDuration,
			unsigned long releaseDuration,
			SMNote note,
			NoteStatusMod* pNoteStatus
		);
};


