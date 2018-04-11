//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlMod
//
// ピアノキーボード制御Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardCtrl.h"
#include "MTPianoKeyboardDesignMod.h"
#include "MTNoteDesignMod.h"


//******************************************************************************
// ピアノキーボード制御Modクラス
//******************************************************************************
class MTPianoKeyboardCtrlMod : public MTPianoKeyboardCtrl
{
public:

	//コンストラクタ／デストラクタ
	MTPianoKeyboardCtrlMod(void);
	virtual ~MTPianoKeyboardCtrlMod(void);

	//生成
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend,
			bool isSingleKeyboard
		);

	//更新
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//リセット
	virtual void Reset();

protected:

	virtual int _CreateKeyboards(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

private:

	//ノートデザイン
	MTNoteDesignMod m_NoteDesignMod;

	//キーボードデザイン
	MTPianoKeyboardDesignMod m_KeyboardDesignMod;

	//ポートリスト
	SMPortList m_PortList;
	int m_PortIndex[SM_MAX_PORT_NUM];
	unsigned char m_MaxPortIndex;

	//キー押下率配列
	float m_KeyDownRateMod[SM_MAX_CH_NUM][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	virtual int _UpdateNoteStatus(
				unsigned long playTimeMSec,
				unsigned long keyDownDuration,
				unsigned long keyUpDuration,
				SMNote note,
				NoteStatus* pNoteStatus
			);
	virtual int _UpdateVertexOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);

};


