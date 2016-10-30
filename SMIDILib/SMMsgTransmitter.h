//******************************************************************************
//
// Simple MIDI Library / SMMsgTransmitter
//
// メッセージ転送クラス
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

#include "SMEvent.h"

namespace SMIDILib {


//******************************************************************************
// パラメータ定義
//******************************************************************************
//メッセージ種別
#define SM_MSG_PLAY_STATUS     (0x00)
#define SM_MSG_TIME            (0x01)
#define SM_MSG_TEMPO           (0x02)
#define SM_MSG_BAR             (0x03)
#define SM_MSG_BEAT            (0x04)
#define SM_MSG_NOTE_OFF        (0x10)
#define SM_MSG_NOTE_ON         (0x11)
#define SM_MSG_PITCHBEND       (0x12)
#define SM_MSG_SKIP_START      (0x13)
#define SM_MSG_SKIP_END        (0x14)
#define SM_MSG_ALL_NOTE_OFF    (0x15)

//演奏状態
#define SM_PLAYSTATUS_STOP       (0x00)
#define SM_PLAYSTATUS_PLAY       (0x01)
#define SM_PLAYSTATUS_PAUSE      (0x02)

//スキップ方向
#define SM_SKIP_BACK           (0x00)
#define SM_SKIP_FORWARD        (0x01)


//******************************************************************************
// メッセージ転送クラス
//******************************************************************************
class SMIDILIB_API SMMsgTransmitter
{
public:

	//コンストラクタ／デストラクタ
	SMMsgTransmitter(void);
	virtual ~SMMsgTransmitter(void);

	//初期化
	int Initialize(HWND hTargetWnd, unsigned long msgId);

	//演奏状態
	int PostPlayStatus(unsigned long playStatus);

	//演奏時間通知
	//  実時間(playTimeSec)は3byte(0x00FFFFFF)までの制限あり
	int PostPlayTime(unsigned long playTimeMSec, unsigned long tickTime);

	//テンポ通知
	int PostTempo(unsigned long bpm);

	//小節番号通知：1から開始
	int PostBar(unsigned long barNo);

	//拍子記号通知
	//  分母は最大65535まで渡せるが
	//  MIDIの仕様では分子255／分母2の255乗まで表現できる
	int PostBeat(unsigned short numerator, unsigned short denominator);

	//ノートON通知
	int PostNoteOn(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				unsigned char verocity
			);

	//ノートOFF通知
	int PostNoteOff(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo
			);

	//ピッチベンド通知
	int PostPitchBend(
				unsigned char portNo,
				unsigned char chNo,
				short pitchBendValue,
				unsigned char pitchBendSensitivity
			);

	//スキップ開始
	int PostSkipStart(unsigned long skipDirection);

	//スキップ終了
	int PostSkipEnd(unsigned long notesCount);

	//オールノートOFF
	int PostAllNoteOff(
				unsigned char portNo,
				unsigned char chNo
			);

private:

	HWND m_hTargetWnd;
	unsigned long m_MsgId;

	int _Post(
			unsigned char msg,
			unsigned long param1, //3byteまで
			unsigned long param2  //4byteまで
		);

	//代入とコピーコンストラクタの禁止
	void operator=(const SMMsgTransmitter&);
	SMMsgTransmitter(const SMMsgTransmitter&);

};

} // end of namespace

