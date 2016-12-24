//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrl
//
// ピアノキーボード制御クラス
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
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
// >>> modify 20120728 yossiepon begin
	virtual int Create(
// <<< modify 20120728 yossiepon end
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			MTNotePitchBend* pNotePitchBend,
			bool isSingleKeyboard
		);

	//更新
// >>> modify 20120728 yossiepon begin
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);
// <<< modify 20120728 yossiepon end

	//描画
// >>> modify 20120728 yossiepon begin
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
// <<< modify 20120728 yossiepon end

	//解放
	void Release();

	//演奏チックタイム登録
	void SetCurTickTime(unsigned long curTickTime);

	//演奏時間設定
	void SetPlayTimeMSec(unsigned long playTimeMsec);

	//リセット
// >>> modify 20120728 yossiepon begin
	virtual void Reset();
// <<< modify 20120728 yossiepon end

	//表示設定
	void SetEnable(bool isEnable);

	//スキップ状態
	void SetSkipStatus(bool isSkipping);

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

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

// >>> modify access level to protected 20161224 yossiepon begin
// <<< modify 20161224 yossiepon end

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

// >>> modify access level 20161224 yossiepon begin
private:
// <<< modify 20161224 yossiepon end

	//スキップ状態
	bool m_isSkipping;

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

	//ピッチベンド情報
	MTNotePitchBend* m_pNotePitchBend;

	//表示可否
	bool m_isEnable;

	//シングルキーボードフラグ
	bool m_isSingleKeyboard;

	int _CreateNoteStatus();
// >>> modify 20120728 yossiepon begin
	virtual int _CreateKeyboards(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData);
// <<< modify 20120728 yossiepon end

	int _TransformActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);

// >>> modify access level 20161224 yossiepon begin
private:
// <<< modify 20161224 yossiepon end

	int _UpdateStatusOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

// >>> modify 20120728 yossiepon begin
	virtual int _UpdateNoteStatus(
				unsigned long playTimeMSec,
				unsigned long keyDownDuration,
				unsigned long keyUpDuration,
				SMNote note,
				NoteStatus* pNoteStatus
			);
	virtual int _UpdateVertexOfActiveNotes(LPDIRECT3DDEVICE9 pD3DDevice);
// <<< modify 20120728 yossiepon end
	float _GetPichBendShiftPosX(unsigned char portNo, unsigned char chNo);

};


