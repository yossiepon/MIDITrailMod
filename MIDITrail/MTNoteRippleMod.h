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

	//M3 DX11: device-free helpers (vertex layout XYZ|NORMAL|DIFFUSE|TEX1 == DXP11_VERTEX)
	int InitForDX11(const TCHAR* pSceneName, SMSeqData* pSeqData, MTNotePitchBend* pNotePitchBend);
	int UpdateCPU(D3DXVECTOR3 camVector, void* pVertexBuf, unsigned long* pActiveNum);
	unsigned long GetOverwriteTimes();

	//live monitor: feed real-time note-ons (timeGetTime-based ms) into the ripple
	int  AddLiveNoteOn(unsigned char portNo, unsigned char chNo, unsigned char noteNo, unsigned long startMsec);
	void RecycleLiveListIfIdle();

	//M4.13 (DX11): use the Ring note design for ripple positions (Ring scene).
	//Set before InitForDX11. The ripple parameters still come from m_NoteDesignMod.
	void SetRingMode(bool isRing) { m_RingMode = isRing; }

protected:

	virtual int _CreateNoteDesign();
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

	//M4.13 (DX11): ring-scene mode (use MTNoteDesignRing for positions)
	bool m_RingMode;

	//ノートリスト
	SMNoteList m_NoteListRT;

	//per-note source track aligned with m_NoteListRT (channel+track color mode);
	//owned copy, NULL when not in track color mode. Indexed by the note's list index.
	unsigned char* m_pTrackNo;

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


