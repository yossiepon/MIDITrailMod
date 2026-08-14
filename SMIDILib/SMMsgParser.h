//******************************************************************************
//
// Simple MIDI Library / SMMsgParser
//
// Message parser class.
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
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
// Message parser class
//******************************************************************************
class SMIDILIB_API SMMsgParser
{
public:

	//Sequencer message type
	enum Message {
		MsgUnknown,		//Unknown message
		MsgPlayStatus,	//Playback state notification
		MsgPlayTime,	//Playback time notification
		MsgTempo,		//Tempo change notification
		MsgBar,			//Bar number notification
		MsgBeat,		//Time signature change notification
		MsgNoteOff,		//Note OFF notification
		MsgNoteOn,		//Note ON notification
		MsgPitchBend,	//Pitch bend notification
		MsgSkipStart,	//Skip start notification
		MsgSkipEnd,		//Skip end notification
		MsgAllNoteOff	//All note OFF notification
	};

	//Playback state
	enum PlayStatus {
		StatusUnknown,	//Unknown message
		StatusStop,		//Stopped
		StatusPlay,		//Playing
		StatusPause		//Paused
	};

	//Skip direction
	enum SkipDirection {
		SkipBack,
		SkipForward
	};

public:

	//Constructor / Destructor
	SMMsgParser(void);
	virtual ~SMMsgParser(void);

	//Message parsing
	void Parse(unsigned long param1, unsigned long param2);

	//Get message type
	Message GetMsg();

	//Get playback state
	PlayStatus GetPlayStatus();

	//Get playback time
	unsigned long GetPlayTimeSec();
	unsigned long GetPlayTimeMSec();
	unsigned long GetPlayTickTime();

	//Get tempo
	unsigned long GetTempoBPM();

	//Get bar number
	unsigned long GetBarNo();

	//Get time signature
	unsigned long GetBeatNumerator();
	unsigned long GetBeatDenominator();

	//Get note ON/OFF info
	unsigned char GetPortNo();
	unsigned char GetChNo();
	unsigned char GetNoteNo();
	unsigned char GetVelocity();

	//Get pitch bend info
	short GetPitchBendValue();
	unsigned char GetPitchBendSensitivity();

	//Get skip start info
	SkipDirection GetSkipStartDirection();

	//Get skip end info
	unsigned long GetSkipEndNotesCount();

private:

	unsigned long m_Param1;
	unsigned long m_Param2;
	Message m_Msg;

};

} // end of namespace

