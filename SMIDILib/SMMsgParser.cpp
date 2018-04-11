#//******************************************************************************
//
// Simple MIDI Library / SMMsgParser
//
// メッセージ解析クラス
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "SMMsgParser.h"
#include "SMMsgTransmitter.h"

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMMsgParser::SMMsgParser(void)
{
	m_Param1 = 0;
	m_Param2 = 0;
	m_Msg = MsgUnknown;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMMsgParser::~SMMsgParser(void)
{
}

//******************************************************************************
// メッセージ解析
//******************************************************************************
void SMMsgParser::Parse(
		unsigned long param1,
		unsigned long param2
	)
{
	m_Param1 = param1;
	m_Param2 = param2;

	switch (m_Param1 >> 24) {
		case SM_MSG_PLAY_STATUS:
			m_Msg = MsgPlayStatus;
			break;
		case SM_MSG_TIME:
			m_Msg = MsgPlayTime;
			break;
		case SM_MSG_TEMPO:
			m_Msg = MsgTempo;
			break;
		case SM_MSG_BAR:
			m_Msg = MsgBar;
			break;
		case SM_MSG_BEAT:
			m_Msg = MsgBeat;
			break;
		case SM_MSG_NOTE_OFF:
			m_Msg = MsgNoteOff;
			break;
		case SM_MSG_NOTE_ON:
			m_Msg = MsgNoteOn;
			break;
		case SM_MSG_PITCHBEND:
			m_Msg = MsgPitchBend;
			break;
		case SM_MSG_SKIP_START:
			m_Msg = MsgSkipStart;
			break;
		case SM_MSG_SKIP_END:
			m_Msg = MsgSkipEnd;
			break;
		case SM_MSG_ALL_NOTE_OFF:
			m_Msg = MsgAllNoteOff;
			break;
		default:
			m_Msg = MsgUnknown;
	}

	return;
}

//******************************************************************************
// メッセージ種別取得
//******************************************************************************
SMMsgParser::Message SMMsgParser::GetMsg()
{
	return m_Msg;
}

//******************************************************************************
// 演奏状態取得
//******************************************************************************
SMMsgParser::PlayStatus SMMsgParser::GetPlayStatus()
{
	PlayStatus status = StatusUnknown;

	if (m_Msg != MsgPlayStatus) {
		goto EXIT;
	}

	switch (m_Param2) {
		case SM_PLAYSTATUS_STOP:
			status = StatusStop;
			break;
		case SM_PLAYSTATUS_PLAY:
			status = StatusPlay;
			break;
		case SM_PLAYSTATUS_PAUSE:
			status = StatusPause;
			break;
		default:
			status = StatusUnknown;
			break;
	}

EXIT:;
	return status;
}

//******************************************************************************
// 演奏時間取得（秒）
//******************************************************************************
unsigned long SMMsgParser::GetPlayTimeSec()
{
	unsigned long timeSec = 0;

	if (m_Msg != MsgPlayTime) {
		goto EXIT;
	}

	timeSec = (m_Param1 & 0x00FFFFFF) / 1000;

EXIT:;
	return timeSec;
}

//******************************************************************************
// 演奏時間取得（ミリ秒）
//******************************************************************************
unsigned long SMMsgParser::GetPlayTimeMSec()
{
	unsigned long timeSec = 0;

	if (m_Msg != MsgPlayTime) {
		goto EXIT;
	}

	timeSec = m_Param1 & 0x00FFFFFF;

EXIT:;
	return timeSec;
}

//******************************************************************************
// チックタイム取得
//******************************************************************************
unsigned long SMMsgParser::GetPlayTickTime()
{
	unsigned long tickTime = 0;

	if (m_Msg != MsgPlayTime) {
		goto EXIT;
	}

	tickTime = m_Param2;

EXIT:;
	return tickTime;
}

//******************************************************************************
// テンポ取得(BPM)
//******************************************************************************
unsigned long SMMsgParser::GetTempoBPM()
{
	unsigned long tempo = 0;
	unsigned long tempoBPM = 0;

	if (m_Msg != MsgTempo) {
		goto EXIT;
	}

	tempo = m_Param2;
	tempoBPM = (60 * 1000 * 1000) / tempo;

EXIT:;
	return tempoBPM;
}

//******************************************************************************
// 小節番号取得
//******************************************************************************
unsigned long SMMsgParser::GetBarNo()
{
	unsigned long barNo = 0;

	if (m_Msg != MsgBar) {
		goto EXIT;
	}

	barNo = m_Param2;

EXIT:;
	return barNo;
}

//******************************************************************************
// 拍子記号取得：分子
//******************************************************************************
unsigned long SMMsgParser::GetBeatNumerator()
{
	unsigned long numerator = 0;

	if (m_Msg != MsgBeat) {
		goto EXIT;
	}

	numerator = m_Param2 >> 16;

EXIT:;
	return numerator;
}

//******************************************************************************
// 拍子記号取得：分母
//******************************************************************************
unsigned long SMMsgParser::GetBeatDenominator()
{
	unsigned long denominator = 0;

	if (m_Msg != MsgBeat) {
		goto EXIT;
	}

	denominator = m_Param2 & 0x0000FFFF;

EXIT:;
	return denominator;
}

//******************************************************************************
// ポート番号取得
//******************************************************************************
unsigned char SMMsgParser::GetPortNo()
{
	unsigned char portNo = 0;

	if ((m_Msg != MsgNoteOff) && (m_Msg != MsgNoteOn) && (m_Msg != MsgPitchBend)
		&& (m_Msg != MsgAllNoteOff)) {
		goto EXIT;
	}

	portNo = (m_Param2 & 0xFF000000) >> 24;

EXIT:;
	return portNo;
}

//******************************************************************************
// ノート情報：チャンネル番号取得
//******************************************************************************
unsigned char SMMsgParser::GetChNo()
{
	unsigned char chNo = 0;

	if ((m_Msg != MsgNoteOff) && (m_Msg != MsgNoteOn) && (m_Msg != MsgPitchBend)
		&& (m_Msg != MsgAllNoteOff)) {
		goto EXIT;
	}

	chNo = (unsigned char)((m_Param2 & 0x00FF0000) >> 16);

EXIT:;
	return chNo;
}

//******************************************************************************
// ノート情報：ノート番号取得
//******************************************************************************
unsigned char SMMsgParser::GetNoteNo()
{
	unsigned char noteNo = 0;

	if ((m_Msg != MsgNoteOff) && (m_Msg != MsgNoteOn)) {
		goto EXIT;
	}

	noteNo = (unsigned char)((m_Param2 & 0x0000FF00) >> 8);

EXIT:;
	return noteNo;
}

//******************************************************************************
// ノート情報：ベロシティ取得
//******************************************************************************
unsigned char SMMsgParser::GetVelocity()
{
	unsigned char velocity = 0;

	if ((m_Msg != MsgNoteOff) && (m_Msg != MsgNoteOn)) {
		goto EXIT;
	}

	velocity = (unsigned char)(m_Param2 & 0x000000FF);

EXIT:;
	return velocity;
}

//******************************************************************************
// ピッチベンド取得
//******************************************************************************
short SMMsgParser::GetPitchBendValue()
{
	short pitchBend = 0;

	if (m_Msg != MsgPitchBend) {
		goto EXIT;
	}

	pitchBend = (short)(m_Param2 & 0x0000FFFF);

EXIT:;
	return pitchBend;
}

//******************************************************************************
// ピッチベンド感度取得
//******************************************************************************
unsigned char SMMsgParser::GetPitchBendSensitivity()
{
	unsigned char sensitivity = 0;

	if (m_Msg != MsgPitchBend) {
		goto EXIT;
	}

	sensitivity = (unsigned char)(m_Param1 & 0x000000FF);

EXIT:;
	return sensitivity;
}

//******************************************************************************
// スキップ開始情報取得
//******************************************************************************
SMMsgParser::SkipDirection SMMsgParser::GetSkipStartDirection()
{
	SkipDirection direction = SkipBack;

	if (m_Msg != MsgSkipStart) {
		goto EXIT;
	}

	if (m_Param2 == SM_SKIP_BACK) {
		direction = SkipBack;
	}
	else if (m_Param2 == SM_SKIP_FORWARD) {
		direction = SkipForward;
	}

EXIT:;
	return direction;
}

//******************************************************************************
// スキップ終了情報取得
//******************************************************************************
unsigned long SMMsgParser::GetSkipEndNotesCount()
{
	unsigned long notesCount = 0;

	if (m_Msg != MsgSkipEnd) {
		goto EXIT;
	}

	notesCount = m_Param2;

EXIT:;
	return notesCount;
}

} // end of namespace

