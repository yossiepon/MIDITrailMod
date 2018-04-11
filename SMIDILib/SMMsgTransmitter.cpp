//******************************************************************************
//
// Simple MIDI Library / SMMsgTransmitter
//
// イベント転送クラス
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMMsgTransmitter.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMMsgTransmitter::SMMsgTransmitter(void)
{
	m_pMsgQueue = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMMsgTransmitter::~SMMsgTransmitter(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int SMMsgTransmitter::Initialize(
		SMMsgQueue* pMsgQueue
	)
{
	m_pMsgQueue = pMsgQueue;
	return 0;
}

//******************************************************************************
// 演奏状態通知
//******************************************************************************
int SMMsgTransmitter::PostPlayStatus(
		unsigned long playStatus
	)
{
	int result = 0;

	result = _Post(SM_MSG_PLAY_STATUS, 0, playStatus);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 演奏位置通知
//******************************************************************************
int SMMsgTransmitter::PostPlayTime(
		unsigned long playTimeMSec,
		unsigned long tickTime
	)
{
	int result = 0;

	//ポストできるデータサイズの制限があるため演奏時間(msec)は3byteまでとする
	//  0x00FFFFFF = 16777215 msec = 16777 sec = 279 min = 4.6 hour
	//この時間を越える場合はクリップする
	if (playTimeMSec > 0x00FFFFFF) {
		playTimeMSec = 0x00FFFFFF;
	}

	result = _Post(SM_MSG_TIME, playTimeMSec, tickTime);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// テンポ通知
//******************************************************************************
int SMMsgTransmitter::PostTempo(
		unsigned long tempo
	)
{
	int result = 0;

	result = _Post(SM_MSG_TEMPO, 0, tempo);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 小節番号通知
//******************************************************************************
int SMMsgTransmitter::PostBar(
		unsigned long barNo
	)
{
	int result = 0;

	result = _Post(SM_MSG_BAR, 0, barNo);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 拍子記号通知
//******************************************************************************
int SMMsgTransmitter::PostBeat(
		unsigned short numerator,
		unsigned short denominator
	)
{
	int result = 0;
	unsigned long param = 0;

	param = ((unsigned long)numerator << 16) | denominator;

	result = _Post(SM_MSG_BEAT, 0, param);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ノートOFF通知
//******************************************************************************
int SMMsgTransmitter::PostNoteOff(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	int result = 0;
	unsigned long param = 0;

	param = (portNo << 24) | (chNo << 16) | (noteNo << 8) | 0;

	result = _Post(SM_MSG_NOTE_OFF, 0, param);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ノートON通知
//******************************************************************************
int SMMsgTransmitter::PostNoteOn(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		unsigned char velocity
	)
{
	int result = 0;
	unsigned long param = 0;

	param = (portNo << 24) | (chNo << 16) | (noteNo << 8) | velocity;

	result = _Post(SM_MSG_NOTE_ON, 0, param);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ピッチベンド通知
//******************************************************************************
int SMMsgTransmitter::PostPitchBend(
		unsigned char portNo,
		unsigned char chNo,
		short pitchBendValue,
		unsigned char pitchBendSensitivity
	)
{
	int result = 0;
	unsigned long param = 0;

	param = (portNo << 24) | (chNo << 16) | ((unsigned short)pitchBendValue);

	result = _Post(SM_MSG_PITCHBEND, pitchBendSensitivity, param);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// スキップ開始通知
//******************************************************************************
int SMMsgTransmitter::PostSkipStart(
		unsigned long skipDirection
	)
{
	int result = 0;
	
	result = _Post(SM_MSG_SKIP_START, 0, skipDirection);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// スキップ開始通知
//******************************************************************************
int SMMsgTransmitter::PostSkipEnd(
		unsigned long notesCount
	)
{
	int result = 0;
	
	result = _Post(SM_MSG_SKIP_END, 0, notesCount);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// オールノートOFF通知
//******************************************************************************
int SMMsgTransmitter::PostAllNoteOff(
		unsigned char portNo,
		unsigned char chNo
	)
{
	int result = 0;
	unsigned char noteNo = 0;
	unsigned long param = 0;
	
	param = (portNo << 24) | (chNo << 16) | (noteNo << 8) | 0;
	
	result = _Post(SM_MSG_ALL_NOTE_OFF, 0, param);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// メッセージ通知
//******************************************************************************
int SMMsgTransmitter::_Post(
		unsigned char event,
		unsigned long param1, //3byteまで
		unsigned long param2  //4byteまで
	)
{
	int result = 0;
	unsigned long param1e = 0;
	BOOL bresult = false;

	param1e = ((unsigned long)event << 24) | (param1 & 0x00FFFFFF);

	if (m_pMsgQueue == NULL) goto EXIT;

	result = m_pMsgQueue->PostMessage(param1e, param2);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

} // end of namespace

