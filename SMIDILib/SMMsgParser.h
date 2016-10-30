#//******************************************************************************
//
// Simple MIDI Library / SMMsgParser
//
// メッセージ解析クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

namespace SMIDILib {

//******************************************************************************
// メッセージ解析クラス
//******************************************************************************
class SMIDILIB_API SMMsgParser
{
public:

	//シーケンサメッセージ種別
	enum Message {
		MsgUnknown,		//メッセージ不明
		MsgPlayStatus,	//演奏状態通知
		MsgPlayTime,	//演奏時間通知
		MsgTempo,		//テンポ変更通知
		MsgBar,			//小節番号通知
		MsgBeat,		//拍子記号変更通知
		MsgNoteOff,		//ノートOFF通知
		MsgNoteOn,		//ノートON通知
		MsgPitchBend,	//ピッチベンド通知
		MsgSkipStart,	//スキップ開始通知
		MsgSkipEnd,		//スキップ終了通知
		MsgAllNoteOff	//オールノートOFF通知
	};

	//演奏状態
	enum PlayStatus {
		StatusUnknown,	//メッセージ不明
		StatusStop,		//停止
		StatusPlay,		//演奏
		StatusPause		//一時停止
	};

	//スキップ方向
	enum SkipDirection {
		SkipBack,
		SkipForward
	};

public:

	//コンストラクタ／デストラクタ
	SMMsgParser(void);
	virtual ~SMMsgParser(void);

	//メッセージ解析
	void Parse(unsigned long wParam, unsigned long lParam);

	//メッセージ種別取得
	Message GetMsg();

	//演奏状態取得
	PlayStatus GetPlayStatus();

	//演奏時間取得
	unsigned long GetPlayTimeSec();
	unsigned long GetPlayTimeMSec();
	unsigned long GetPlayTickTime();

	//テンポ取得
	unsigned long GetTempoBPM();

	//小節番号取得
	unsigned long GetBarNo();

	//拍子記号取得
	unsigned long GetBeatNumerator();
	unsigned long GetBeatDenominator();

	//ノートON/OFF情報取得
	unsigned char GetPortNo();
	unsigned char GetChNo();
	unsigned char GetNoteNo();
	unsigned char GetVelocity();

	//ピッチベンド情報取得
	short GetPitchBendValue();
	unsigned char GetPitchBendSensitivity();

	//スキップ開始情報取得
	SkipDirection GetSkipStartDirection();

	//スキップ終了情報取得
	unsigned long GetSkipEndNotesCount();

private:

	unsigned long m_WParam;
	unsigned long m_LParam;
	Message m_Msg;

};

} // end of namespace

