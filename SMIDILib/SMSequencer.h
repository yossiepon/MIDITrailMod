//******************************************************************************
//
// Simple MIDI Library / SMSequencer
//
// MIDI sequencer class.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "mmsystem.h"
#include "SMEventMIDI.h"
#include "SMEventSysEx.h"
#include "SMEventMeta.h"
#include "SMSeqData.h"
#include "SMMsgTransmitter.h"
#include "SMMsgQueue.h"
#include "SMOutDevCtrl.h"
#include "SMCommon.h"
#include "SMFPUCtrl.h"
#include "SMEventWatcher.h"

namespace SMIDILib {


//******************************************************************************
// Sequencer class
//******************************************************************************
class SMIDILIB_API SMSequencer
{
public:

	//Playback state
	enum Status {
		StatusPlay,
		StatusPause,
		StatusStop
	};

	//User request
	enum UserRequest {
		RequestNone,
		RequestPause,
		RequestStop,
		RequestSkip
	};

	//Constructor / Destructor
	SMSequencer(void);
	virtual ~SMSequencer(void);

	//Initialize
	int Initialize(SMMsgQueue* pMsgQueue);

	//Register port-to-device mapping
	int SetPortDev(unsigned char portNo, const char* pProductName);

	//Register sequence data
	int SetSeqData(SMSeqData* pSeqData);

	//Start playback
	int Play();

	//Pause playback
	void Pause();

	//Resume playback
	int Resume();

	//Stop playback
	void Stop();

	//Playback speed setting
	void SetPlaybackSpeed(unsigned long nTimes); //n times speed
	void SetPlaySpeedRatio(unsigned long ratio); //Percentage

	//Rewind/skip move time span setting
	void SetMovingTimeSpanInMsec(unsigned long timeSpan);

	//Change playback position
	int Skip(int relativeTimeInMsec);

private:

	//Playback state
	Status m_Status;
	unsigned long m_PlayIndex;
	UserRequest m_UserRequest;
	SMMsgTransmitter m_MsgTrans;
	SMMsgQueue* m_pMsgQue;
	SMEventWatcher m_EventWatcher;

	//MIDI device related
	SMOutDevCtrl m_OutDevCtrl;
	unsigned char m_PortNo;
	char m_PortDevName[SM_MIDIOUT_PORT_NUM_MAX][MAXPNAMELEN];

	//MIDI data related
	SMSeqData* m_pSeqData;
	SMTrack m_Track;
	SMEvent m_Event;

	//Timer control related
	UINT m_TimerID;
	unsigned long m_TimerResolution;
	unsigned long m_TimeDivision;
	unsigned long m_Tempo;
	unsigned long long m_PrevTimerTime;
	unsigned long long m_CurPlayTime;
	unsigned long long m_PrevEventTime;
	double m_NextEventTime;
	unsigned long long m_NextNtcTime;
	unsigned long m_PrevDeltaTime;
	unsigned long m_TotalTickTime;
	unsigned long m_TotalTickTimeTemp;
	unsigned long m_PlaybackSpeed;
	double m_PlaySpeedRatio;

	//Floating-point arithmetic control
	SMFPUCtrl m_FPUCtrl;

	//Bar number control related
	unsigned long m_TickTimeOfBar;
	unsigned long m_CurBarNo;
	unsigned long m_PrevBarTickTime;

	//Time signature
	unsigned long m_BeatNumerator;
	unsigned long m_BeatDenominator;

	//Skip control
	bool m_isSkipping;
	volatile bool m_isInTimer;
	unsigned long long m_TotalPlayTimeNano;
	unsigned long long m_SkipTargetTime;
	unsigned long m_NotesCount;
	unsigned long m_MovingTimeSpanInMsec;
	unsigned char m_CachePitchBend[SM_MAX_PORT_NUM][SM_MAX_CH_NUM][2];
	unsigned char m_CacheCC001_Modulation[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_CacheCC007_Volume[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_CacheCC010_Panpot[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_CacheCC011_Expression[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];

	//Note velocity
	unsigned char m_NoteVelocity[SM_MIDIOUT_PORT_NUM_MAX][SM_MAX_CH_NUM][SM_MAX_NOTE_NUM];

	//Timer device processing
	int _InitializeTimerDev();
	int _ReleaseTimerDev();

	//Port control
	void _ClearPortInfo();
	int _OpenMIDIOutDev();
	int _CloseMIDIOutDev();

	//Playback control
	int _InitializeParamsOnPlayStart();

	//Playback thread interval processing
	int _IntervalProc(BOOL* pIsContinue);

	//Time control
	int _UpdatePlayPosition();
	double _ConvTick2TimeNanosec(unsigned long tickTime);
	unsigned long _ConvTimeNanosec2Tick(unsigned long long timeMsec);
	unsigned long long _GetCurTimeInNano();

	//MIDI output processing
	int _OutputMIDIEvent(unsigned char portNo, SMEvent* pEvent);
	int _SendMIDIEvent(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int _SendSysExEvent(unsigned char portNo, SMEventSysEx* pSysExEvent);
	int _SendMetaEvent(unsigned char portNo, SMEventMeta* pMetaEvent);
	int _SendNoteOffForActiveNotes();
	int _SendNoteOnForActiveNotes();
	int _AllTrackNoteOff();
	int _AllTrackSoundOff();

	//Other
	int _ProcUserRequest(BOOL* pIsContinue);

	//Skip control
	void _ClearMIDIEventCache();
	int _FilterMIDIEvent(unsigned char portNo, SMEventMIDI* pMIDIEvent, bool* pIsFiltered);
	int _SendMIDIEventCache();
	int _SendMIDIEventPitchBend(unsigned char portNo, unsigned char chNo, unsigned char* pPtichBend);
	int _SendMIDIEventCC(unsigned char portNo, unsigned char chNo, unsigned char ccNo, unsigned char ccValue);
	int _ProcSkip(unsigned long long targetTimeInNanoSec, BOOL* pIsContinue);
	void _SlidePlaybackTime(unsigned long long startPlayTime, unsigned long startTickTime, unsigned long endTickTime);

	//Note state control
	void _ClearNoteVelocity();

	//Timer processing
	static void CALLBACK _TimerCallBack(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

protected:

	int _OnTimer();

private:

	//Prohibit assignment and copy constructor
	void operator=(const SMSequencer&);
	SMSequencer(const SMSequencer&);

};

} // end of namespace

