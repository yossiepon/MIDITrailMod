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
	int Transform(
			LPDIRECT3DDEVICE9 pD3DDevice,
			D3DXVECTOR3 camVector,
			D3DXVECTOR3 lookVector,
			float rollAngle
		);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//リセット
	void Reset();

protected:

	int _CreateKeyboards(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

private:

	//ノートデザイン
	MTNoteDesignMod m_NoteDesignMod;

	//キーボードデザイン
	MTPianoKeyboardDesignMod m_KeyboardDesignMod;

	//アクティブポートフラグ
	bool m_isActivePort[SM_MAX_PORT_NUM];

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

	float _GetMaxPitchBendShift(int keyboardIndex);
	float _GetMaxPitchBendShift(int keyboardIndex, float max);
};


