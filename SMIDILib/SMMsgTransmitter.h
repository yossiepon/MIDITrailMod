//******************************************************************************
//
// Simple MIDI Library / SMMsgTransmitter
//
// Message transmitter class.
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"
#include "SMMsgQueue.h"

namespace SMIDILib {


//******************************************************************************
// Parameter definitions
//******************************************************************************
//Message type
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

//Playback state
#define SM_PLAYSTATUS_STOP       (0x00)
#define SM_PLAYSTATUS_PLAY       (0x01)
#define SM_PLAYSTATUS_PAUSE      (0x02)

//Skip direction
#define SM_SKIP_BACK           (0x00)
#define SM_SKIP_FORWARD        (0x01)


//******************************************************************************
// Message transmitter class
//******************************************************************************
class SMIDILIB_API SMMsgTransmitter
{
public:

	//Constructor / Destructor
	SMMsgTransmitter(void);
	virtual ~SMMsgTransmitter(void);

	//Initialize
	int Initialize(SMMsgQueue* pMsgQueue);

	//Playback state
	int PostPlayStatus(unsigned long playStatus);

	//Playback time notification
	//  Real time (playTimeSec) is limited to 3 bytes (0x00FFFFFF)
	int PostPlayTime(unsigned long playTimeMSec, unsigned long tickTime);

	//Tempo notification
	int PostTempo(unsigned long bpm);

	//Bar number notification: starts from 1
	int PostBar(unsigned long barNo);

	//Time signature notification
	//  The denominator can be passed up to 65535, but
	//  the MIDI spec allows numerator up to 255 / denominator up to 2^255
	int PostBeat(unsigned short numerator, unsigned short denominator);

	//Note ON notification
	int PostNoteOn(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				unsigned char verocity
			);

	//Note OFF notification
	int PostNoteOff(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo
			);

	//Pitch bend notification
	int PostPitchBend(
				unsigned char portNo,
				unsigned char chNo,
				short pitchBendValue,
				unsigned char pitchBendSensitivity
			);

	//Skip start
	int PostSkipStart(unsigned long skipDirection);

	//Skip end
	int PostSkipEnd(unsigned long notesCount);

	//All note OFF
	int PostAllNoteOff(
				unsigned char portNo,
				unsigned char chNo
			);

private:

	SMMsgQueue* m_pMsgQueue;

	int _Post(
			unsigned char msg,
			unsigned long param1, //up to 3 bytes
			unsigned long param2  //up to 4 bytes
		);

	//Prohibit assignment and copy constructor
	void operator=(const SMMsgTransmitter&);
	SMMsgTransmitter(const SMMsgTransmitter&);

};

} // end of namespace

