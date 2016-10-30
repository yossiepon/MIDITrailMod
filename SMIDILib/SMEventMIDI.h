//******************************************************************************
//
// Simple MIDI Library / SMEventMIDI
//
// MIDIイベントクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// イベントクラスから派生させる設計が理想だが、newの実施回数を激増させる
// ため、スタックで処理できるデータ解析ユーティリティクラスとして実装する。

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"


namespace SMIDILib {

//******************************************************************************
// MIDIイベントクラス
//******************************************************************************
class SMIDILIB_API SMEventMIDI
{
public:

	//チャンネルメッセージ種別
	enum ChMsg {
		None					= 0x00, // none
		NoteOff					= 0x80, // 8n kk vv
		NoteOn					= 0x90, // 9n kk vv
		PolyphonicKeyPressure	= 0xA0, // An kk vv
		ControlChange			= 0xB0, // Bn cc vv
		ProgramChange			= 0xC0, // Cn pp   
		ChannelPressure			= 0xD0, // Dn vv   
		PitchBend				= 0xE0  // En mm ll
	};

public:

	//コンストラクタ／デストラクタ
	SMEventMIDI();
	virtual ~SMEventMIDI(void);

	//イベントアタッチ
	void Attach(SMEvent* pEvent);

	//MIDI出力メッセージ取得
	int GetMIDIOutShortMsg(unsigned long* pMsg);

	//チャンネルメッセージ
	ChMsg GetChMsg();

	//チャンネル番号取得
	unsigned char GetChNo();

	//ノート番号取得
	unsigned char GetNoteNo();

	//ベロシティ取得
	unsigned char GetVelocity();

	//コントロールチェンジ番号取得
	unsigned char GetCCNo();

	//コントロールチェンジ値取得
	unsigned char GetCCValue();

	//プログラム番号取得
	unsigned char GetProgramNo();

	//チャンネルプレッシャー値取得
	unsigned char GetPressureValue();

	//ピッチベンド値取得
	short GetPitchBendValue();

private:

	SMEvent* m_pEvent;

	//代入とコピーコンストラクタの禁止
	void operator=(const SMEventMIDI&);
	SMEventMIDI(const SMEventMIDI&);

};

} // end of namespace

