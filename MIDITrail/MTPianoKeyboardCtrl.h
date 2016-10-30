//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrl
//
// ピアノキーボード制御クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// 複数のピアノキーボードを管理するクラス。
// 各キーボードの配置とキーの押下状態を制御する。
// 現状は1ポート(16ch)の描画のみに対応している。
// 2ポート目以降の描画には対応していない。

#pragma once

#include "SMIDILib.h"
#include "MTPianoKeyboard.h"
#include "MTPianoKeyboardDesign.h"
#include "MTNotePitchBend.h"
#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
// ピアノキーボード制御クラス
//******************************************************************************
class MTPianoKeyboardCtrl
{
public:

	//コンストラクタ／デストラクタ
	MTPianoKeyboardCtrl(void);
	virtual ~MTPianoKeyboardCtrl(void);

	//生成
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend
		);

	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//解放
	void Release();

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

private:

	//ノートデザイン
	MTNoteDesign m_NoteDesign;

	//キーボード描画オブジェクト：ポインタ配列
	MTPianoKeyboard* m_pPianoKeyboard[SM_MAX_CH_NUM];

	//キーボードデザイン
	MTPianoKeyboardDesign m_KeyboardDesign;

	//ノートリスト
	SMNoteList m_NoteListRT;

	//発音中ノート管理
	unsigned long m_PlayTimeMSec;
	unsigned long m_CurTickTime;
	unsigned long m_CurNoteIndex;
	NoteStatus* m_pNoteStatus;
	float m_KeyDownRate[SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	//スキップ状態
	bool m_isSkipping;

	//ピッチベンド情報
	MTNotePitchBend* m_pNotePitchBend;

	//表示可否
	bool m_isEnable;

	int _CreateNoteStatus();
	int _CreateKeyboards(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);

	int _TransformActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateStatusOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateNoteStatus(
				unsigned long playTimeMSec,
				unsigned long keyDownDuration,
				unsigned long keyUpDuration,
				SMNote note,
				NoteStatus* pNoteStatus
			);
	int _UpdateVertexOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	float _GetPichBendShiftPosX(unsigned char portNo, unsigned char chNo);

};


