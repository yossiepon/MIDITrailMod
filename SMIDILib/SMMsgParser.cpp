//******************************************************************************
//
// Simple MIDI Library / SMMsgParser
//
// Message parser class.
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "SMMsgParser.h"
#include "SMMsgTransmitter.h"

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMMsgParser::SMMsgParser(void)
{
	m_Param1 = 0;
	m_Param2 = 0;
	m_Msg = MsgUnknown;
}

//******************************************************************************
// Destructor
//******************************************************************************
SMMsgParser::~SMMsgParser(void)
{
}

//******************************************************************************
// Message parsing
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
// Get message type
//******************************************************************************
SMMsgParser::Message SMMsgParser::GetMsg()
{
	return m_Msg;
}

//******************************************************************************
// Get playback status
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
// Get playback time (seconds)
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
// Get playback time (milliseconds)
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
// Get tick time
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
// Get tempo(BPM)
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
// Get bar number
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
// Get time signature: numerator
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
// Get time signature: denominator
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
// Get port number
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
// Note info: get channel number
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
// Note info: get note number
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
// Note info: get velocity
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
// Get pitch bend
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
// Get pitch bend sensitivity
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
// Get skip start info
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
// Get skip end info
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

