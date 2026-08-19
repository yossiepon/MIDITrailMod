//******************************************************************************
//
// Simple MIDI Library / SMSequencer
//
// MIDI sequencer class.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 Yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

// MEMO:
// The timer thread handles playback processing, so it must focus solely
// on controlling the MIDI output device. Do not perform screen update
// processing etc. on this thread.
// Notify other threads via PostMessage etc.
// _TimerCallBack()->_OnTimer()->...

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMSequencer.h"
#include "SMEventMIDI.h"
#include "SMEventSysEx.h"
#include "SMEventMeta.h"
#include "SMFPUCtrl.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMSequencer::SMSequencer(void)
{
	//Playback status
	m_Status = StatusStop;
	m_PlayIndex = 0;
	m_UserRequest = RequestNone;

	//MIDI device related
	m_PortNo = 0;

	//MIDI data related
	m_pSeqData = NULL;

	//Timer control related
	m_TimerID = NULL;
	m_TimerResolution = 0;
	m_TimeDivision = 0;
	m_Tempo = SM_DEFAULT_TEMPO;
	m_PrevTimerTime = 0;
	m_CurPlayTime = 0;
	m_PrevEventTime = 0;
	m_NextEventTime = 0.0;
	m_NextNtcTime = 0;
	m_TotalTickTime = 0;
	m_TotalTickTimeTemp = 0;
	m_PlaybackSpeed = 1;
	m_PlaySpeedRatio = 1.0;

	//Skip control
	m_isSkipping = false;
	m_isInTimer = false;
	m_SkipTargetTime = 0;
	m_NotesCount = 0;
	m_TotalPlayTimeNano = 0;
	m_MovingTimeSpanInMsec = 0;

	//Bar number control related
	m_TickTimeOfBar = 0;
	m_CurBarNo = 1;
	m_PrevBarTickTime = 0;

	//Time signature
	m_BeatNumerator = 0;
	m_BeatDenominator = 0;

	//Clear port info
	_ClearPortInfo();

		//Clear note velocity
	_ClearNoteVelocity();
}

//******************************************************************************
// Destructor
//******************************************************************************
SMSequencer::~SMSequencer(void)
{
	//Clear port info
	_ClearPortInfo();

	//Close MIDI output device
	_CloseMIDIOutDev();

	//Release timer device
	_ReleaseTimerDev();
}

//******************************************************************************
// Initialize
//******************************************************************************
int SMSequencer::Initialize(
		SMMsgQueue* pMsgQueue
	)
{
	int result = 0;

	if (m_Status != StatusStop) {
		result = YN_SET_ERR("Program error.", m_Status, 0);
		goto EXIT;
	}

	//Initialize MIDI output device
	result = m_OutDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	//Clear port info
	_ClearPortInfo();

	//Initialize event transmission object
	result = m_MsgTrans.Initialize(pMsgQueue);
	if (result != 0) goto EXIT;

	//Initialize event watcher
	result = m_EventWatcher.Initialize(&m_MsgTrans);
	if (result != 0) goto EXIT;

	//Playback mode: disable MsgQueue posting of NoteOn/NoteOff/AllNoteOff
	//Not needed in DX11 since NoteTracker manages draw notes. Defaults to true for Live
	m_EventWatcher.SetNoteEventPostEnabled(false);

	//Initialize timer device
	result = _InitializeTimerDev();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Register port-to-device mapping
//******************************************************************************
int SMSequencer::SetPortDev(
		unsigned char portNo,
		const char* pProductName
	)
{
	int result = 0;
	errno_t eresult = 0;

	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}

	eresult = strcpy_s(m_PortDevName[portNo], MAXPNAMELEN, pProductName);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Register sequence data
//******************************************************************************
int SMSequencer::SetSeqData(
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long numerator = 0;
	unsigned long denominator = 0;

	if (m_Status != StatusStop) {
		result = YN_SET_ERR("Program error.", m_Status, 0);
		goto EXIT;
	}

	m_pSeqData = pSeqData;

	//Hold nominal song length (nanoseconds) (temporary: remove once load-time event clipping is introduced)
	//GetTotalPlayTime returns milliseconds
	m_TotalPlayTimeNano = (unsigned long long)m_pSeqData->GetTotalPlayTime() * 1000000ULL;

	//Get merged track
	result = m_pSeqData->GetMergedTrack(&m_Track);
	if (result != 0) goto EXIT;

	//Get resolution: value indicating the length of a quarter note (ex. 48, 480, ...)
	m_TimeDivision = m_pSeqData->GetTimeDivision();
	if (m_TimeDivision == 0) {
		//Invalid data: should have been checked when reading the SMF
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//Get tempo
	m_Tempo = m_pSeqData->GetTempo();
	if (m_Tempo == 0) {
		//Invalid data
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}

	//Calculate tick time per bar from the time signature
	numerator = m_pSeqData->GetBeatNumerator();
	denominator = m_pSeqData->GetBeatDenominator();
	if (denominator == 0) {
		//Invalid data
		result = YN_SET_ERR("Invalid data found.", numerator, denominator);
		goto EXIT;
	}
	m_TickTimeOfBar = (numerator * m_TimeDivision * 4) / denominator;

	m_BeatNumerator = numerator;
	m_BeatDenominator = denominator;

EXIT:;
	return result;
}

//******************************************************************************
// Start playback
//******************************************************************************
int SMSequencer::Play()
{
	int result = 0;
	SMFPUCtrl fpuCtrl;

	if (m_pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//Do nothing if already playing
	if (m_Status == StatusPlay) goto EXIT;

	//Set floating-point precision to double
	result = fpuCtrl.Start(SMFPUCtrl::FPUDouble);
	if (result != 0) goto EXIT;

	//Start playback from the beginning
	if (m_Status == StatusStop) {
		//Open the MIDI output device
		result = _OpenMIDIOutDev();
		if (result != 0) goto EXIT;

		//Initialize parameters on play start
		result = _InitializeParamsOnPlayStart();
		if (result != 0) goto EXIT;
	}
	//Resume playback from pause
	if (m_Status == StatusPause) {
		m_PrevTimerTime = _GetCurTimeInNano();

		//Send note-on for active notes
		result = _SendNoteOnForActiveNotes();
		if (result != 0) goto EXIT;
	}
	m_Status = StatusPlay;
	m_UserRequest = RequestNone;
	m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_PLAY);

	//Start the timer
	m_TimerID = timeSetEvent(
					m_TimerResolution, //Event delay (milliseconds)
					m_TimerResolution, //Event resolution (milliseconds)
					_TimerCallBack,    //Callback function
					(DWORD_PTR)this,   //User callback data
					TIME_PERIODIC      //Timer type: periodic invocation
				);
	if (m_TimerID == NULL) {
		result = YN_SET_ERR("Timer device error.", m_TimerResolution, 0);
		goto EXIT;
	}

	result = fpuCtrl.End();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Pause playback
//******************************************************************************
void SMSequencer::Pause()
{
	//Just accept the request (no queuing)
	//The actual processing is delegated to the timer thread
	m_UserRequest = RequestPause;
}

//******************************************************************************
// Resume playback
//******************************************************************************
int SMSequencer::Resume()
{
	int result = 0;

	//Currently Play() also serves as the resume handler
	result = Play();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Stop playback
//******************************************************************************
void SMSequencer::Stop()
{

	if (m_Status == StatusPause) {
		//While paused, the timer thread is stopped,
		//so notify termination from here
		m_Status = StatusStop;
		m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_STOP);
	}
	else {
		//While playing, just accept the request (no queuing)
		//The actual processing is delegated to the timer thread
		m_UserRequest = RequestStop;
	}
}

//******************************************************************************
// Set playback speed (n times)
//******************************************************************************
void SMSequencer::SetPlaybackSpeed(
		unsigned long nTimes
	)
{
	m_PlaybackSpeed =  nTimes;
}

//******************************************************************************
// Set playback speed (percent)
//******************************************************************************
void SMSequencer::SetPlaySpeedRatio(
		unsigned long ratio
	)
{
	m_PlaySpeedRatio =  (double)ratio / 100.0;
}

//******************************************************************************
// Set rewind/skip movement time span
//******************************************************************************
void SMSequencer::SetMovingTimeSpanInMsec(
		unsigned long timeSpan
	)
{
	m_MovingTimeSpanInMsec = timeSpan;
}

//******************************************************************************
//Skip playback position
//******************************************************************************
int SMSequencer::Skip(
		int relativeTimeInMsec
	)
{
	int result = 0;
	unsigned long long diffTime = 0;

	//Do nothing if not playing
	if (m_Status != StatusPlay) goto EXIT;

	//Playback position
	if (relativeTimeInMsec < 0) {
		diffTime = (unsigned long long)(-1 * relativeTimeInMsec) * 1000000;
		if (m_CurPlayTime < diffTime) {
			m_SkipTargetTime = 0;
		}
		else {
			m_SkipTargetTime = m_CurPlayTime - diffTime;
		}
	}
	else {
		diffTime = (unsigned long long)(relativeTimeInMsec) * 1000000;
		m_SkipTargetTime = m_CurPlayTime + diffTime;
		//May exceed the song's end time
	}

	//While playing, just accept the request (no queuing)
	//The actual processing is delegated to the timer thread
	m_UserRequest = RequestSkip;

EXIT:;
	return result;
}

//******************************************************************************
// Initialize timer device
//******************************************************************************
int SMSequencer::_InitializeTimerDev()
{
	int result = 0;
	UINT apiresult = 0;
	TIMECAPS tc;

	if (m_TimerResolution != 0) goto EXIT;

	//Get the minimum resolution of the timer device (typically 1ms)
	apiresult = timeGetDevCaps(&tc, sizeof(TIMECAPS));
	if (apiresult != TIMERR_NOERROR) {
		result = YN_SET_ERR("Timer device error.", apiresult, 0);
		goto EXIT;
	}
	m_TimerResolution = tc.wPeriodMin;

	//Set the minimum timer resolution
	timeBeginPeriod(m_TimerResolution);

EXIT:;
	return result;
}

//******************************************************************************
// Release timer device
//******************************************************************************
int SMSequencer::_ReleaseTimerDev()
{
	int result = 0;
	UINT apiresult = 0;

	if (m_TimerResolution != 0) {
		apiresult = timeEndPeriod(m_TimerResolution);
		if (apiresult != TIMERR_NOERROR) {
			result = YN_SET_ERR("Timer device error.", apiresult, 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Clear port info
//******************************************************************************
void SMSequencer::_ClearPortInfo()
{
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortDevName[portNo][0] = '\0';
	}
}

//******************************************************************************
// Open the MIDI output device
//******************************************************************************
int SMSequencer::_OpenMIDIOutDev()
{
	int result = 0;
	unsigned char portNo = 0;
	TCHAR warnMsg[1024] = {_T('\0')};
	int warnCount = 0;

	//Register port-to-device names with the MIDI output device control
	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		if (strlen(m_PortDevName[portNo]) > 0) {
			result = m_OutDevCtrl.SetPortDev(portNo, m_PortDevName[portNo]);
			if (result != 0) goto EXIT;

			//Consume TLS to detect device-not-found warning
			YNErrInfo* pWarn = YNErrCtrl::GetErr();
			if (pWarn != NULL) {
				TCHAR line[256];
				_sntprintf_s(line, 256, _TRUNCATE,
					_T("  Port %c: %s (not found)\n"), _T('A') + portNo, m_PortDevName[portNo]);
				_tcsncat_s(warnMsg, 1024, line, _TRUNCATE);
				warnCount++;
				delete pWarn;
			}
		}
	}

	//Open the devices for all ports
	result = m_OutDevCtrl.OpenPortDevAll();
	if (result != 0) goto EXIT;

	//Consume TLS to detect port-open warnings
	{
		YNErrInfo* pWarn = YNErrCtrl::GetErr();
		if (pWarn != NULL) {
			_tcsncat_s(warnMsg, 1024, _T("  Some ports failed to open.\n"), _TRUNCATE);
			warnCount++;
			delete pWarn;
		}
	}

	//Show aggregated warning dialog
	if (warnCount > 0) {
		TCHAR summary[1200];
		_sntprintf_s(summary, 1200, _TRUNCATE,
			_T("MIDI OUT device warning:\n%sPlayback will continue without these ports."), warnMsg);
		YN_SET_WARN(summary, warnCount, 0);
		YN_SHOW_ERR(NULL);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Close the MIDI output device
//******************************************************************************
int SMSequencer::_CloseMIDIOutDev()
{
	int result = 0;

	result = m_OutDevCtrl.ClosePortDevAll();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Playback interval processing
//******************************************************************************
int SMSequencer::_IntervalProc(
		BOOL* pIsContinue
	)
{
	int result = 0;
	unsigned long deltaTime = 0;

	*pIsContinue = true;

	//Update playback position
	result = _UpdatePlayPosition();
	if (result != 0) goto EXIT;

	//Stop playback if the nominal song length + margin is exceeded (safety net)
	//Normally, completion of all event processing (m_PlayIndex >= m_Track.GetSize()) fires first for MIDI
	if (m_TotalPlayTimeNano > 0 && m_CurPlayTime >= m_TotalPlayTimeNano + 100000000ULL) {
		if (!m_isSkipping) {
			_AllTrackNoteOff();
			m_MsgTrans.PostPlayTime(
				(unsigned long)(m_TotalPlayTimeNano / 1000000), m_TotalTickTime);
			m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_STOP);
			m_Status = StatusStop;
		}
		*pIsContinue = false;
		goto EXIT;
	}

	//If the event processing time has been reached, perform the send processing
	//The while loop ensures that even if delay accumulates, one callback fully catches up
	while (((unsigned long long)m_NextEventTime <= m_CurPlayTime) && *pIsContinue) {

		//Sum of tick time
		m_TotalTickTime += m_PrevDeltaTime;

		deltaTime = 0;
		while (deltaTime == 0) {
			//Event transmission
			result = _OutputMIDIEvent(m_PortNo, &m_Event);
			if (result != 0) goto EXIT;

			//Stop playback at end of data
			m_PlayIndex++;
			if (m_PlayIndex >= m_Track.GetSize()) {
				if (!m_isSkipping) {
					_AllTrackNoteOff();
					m_MsgTrans.PostPlayTime((unsigned long)(m_CurPlayTime/1000000), m_TotalTickTime);
					m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_STOP);
					m_Status = StatusStop;
				}
				*pIsContinue = false;
				break;
			}

			//Get next event
			m_Track.GetDataSet(m_PlayIndex, &deltaTime, &m_Event, &m_PortNo);
		}
		//Remember the event occurrence time for periodic notification
		//Periodic notification does not require strict precision, so anything under 1msec is ignored
		m_PrevEventTime = (unsigned long long)m_NextEventTime;

		//Calculate the next event transmission position
		m_NextEventTime += _ConvTick2TimeNanosec(deltaTime);
		m_PrevDeltaTime = deltaTime;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Update playback position
//******************************************************************************
int SMSequencer::_UpdatePlayPosition()
{
	int result = 0;
	unsigned long long curTime = 0;
	unsigned long long diffTime = 0;
	unsigned long diffTickTime = 0;
	unsigned long nextBarTickTime = 0;
	unsigned long long ntcSpan = 0;

	curTime = _GetCurTimeInNano();

	//Update playback time using the elapsed time since the previous timer tick
	//  This calculation is still valid even when crossing the 49-day mark since startup
	diffTime = curTime - m_PrevTimerTime;

	//Update playback time using the elapsed time since the previous timer tick
	if (m_isSkipping) {
		//While skipping, advance by a virtual 5msec.
		diffTime = 5 * 1000000;
	}
	else {
		//If not skipping, calculate the actual elapsed time
		diffTime = curTime - m_PrevTimerTime;
	}

	//Apply playback speed (n times)
	if (m_PlaybackSpeed == 1) {
		diffTime = (unsigned long long)((double)diffTime * m_PlaySpeedRatio);
	}
	else {
		diffTime = diffTime * m_PlaybackSpeed;
	}

	m_CurPlayTime += diffTime;
	m_PrevTimerTime = curTime;

	//Convert the elapsed time since the previous event into tick time
	//  A conversion error occurs but is harmless since it is not allowed to accumulate
	diffTickTime = _ConvTimeNanosec2Tick(m_CurPlayTime - m_PrevEventTime);

	//Total tick time from the start of the song
	//m_TotalTickTime is only updated on event occurrence, so it is not rewritten here
	m_TotalTickTimeTemp = m_TotalTickTime + diffTickTime;

	//Notify the playback time once the notification time is reached
	if ((m_NextNtcTime <= m_CurPlayTime) && (!m_isSkipping)) {
		m_MsgTrans.PostPlayTime((unsigned long)(m_CurPlayTime/1000000), m_TotalTickTimeTemp);
		//The notification interval is set to 1,000,000,000/120[nanosec] x playback speed, considering 60FPS display
		//TODO: allow the interval to be specified externally
		ntcSpan = (unsigned long long)(1000000000.0 * m_PlaySpeedRatio / 120.0);
		m_NextNtcTime = m_CurPlayTime - (m_CurPlayTime % ntcSpan) + ntcSpan;
	}

	//Check whether the bar number needs updating
	nextBarTickTime = m_PrevBarTickTime + m_TickTimeOfBar;
	if (nextBarTickTime <= m_TotalTickTimeTemp) {
		m_CurBarNo++;
		m_PrevBarTickTime = nextBarTickTime;
		if (!m_isSkipping) {
			m_MsgTrans.PostBar(m_CurBarNo);
		}
	}

//EXIT:;
	return result;
}

//******************************************************************************
// Convert tick time to real time (nanoseconds)
//******************************************************************************
double SMSequencer::_ConvTick2TimeNanosec(
		unsigned long tickTime
	)
{
	double timeNanosec = 0;
	
	//(1) Resolution per quarter note: division
	//    e.g. 48
	//(2) Delta time of track data: delta
	//    A time difference expressed using the resolution value
	//    If the resolution is 48 and the delta time is 24, that is the time difference of an eighth note
	//(3) Tempo setting (microseconds): tempo
	//    Real-time interval of a quarter note
	//
	// Real-time interval corresponding to the delta time (milliseconds)
	//  = (delta / division) * tempo / 1000
	//  = (delta * tempo) / (division * 1000)
	
	timeNanosec = ((double)tickTime * (double)m_Tempo) * 1000.0 / ((double)m_TimeDivision);
	
	return timeNanosec;
}

//******************************************************************************
// Convert real time (nanoseconds) to tick time
//******************************************************************************
unsigned long SMSequencer::_ConvTimeNanosec2Tick(
		unsigned long long timeNanosec
	)
{
	unsigned long tickTime = 0;
	unsigned long long a = 0;
	unsigned long long b = 0;
	
	a = timeNanosec * m_TimeDivision / 1000;
	b = a / m_Tempo;
	tickTime = (unsigned long)b;
	
	return tickTime;
}

//******************************************************************************
// Event transmission processing
//******************************************************************************
int SMSequencer::_OutputMIDIEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;

	//MIDIEvent transmission
	if (pEvent->GetType() == SMEvent::EventMIDI) {
		SMEventMIDI eventMIDI;
		eventMIDI.Attach(pEvent);
		result = _SendMIDIEvent(portNo, &eventMIDI);
		if (result != 0) goto EXIT;
	}
	//SysExEvent transmission
	else if (pEvent->GetType() == SMEvent::EventSysEx) {
		SMEventSysEx eventSysEx;
		eventSysEx.Attach(pEvent);
		result = _SendSysExEvent(portNo, &eventSysEx);
		if (result != 0) goto EXIT;
	}
	//Meta event transmission
	else if (pEvent->GetType() == SMEvent::EventMeta) {
		SMEventMeta eventMeta;
		eventMeta.Attach(pEvent);
		result = _SendMetaEvent(portNo, &eventMeta);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDIEvent transmission
//******************************************************************************
int SMSequencer::_SendMIDIEvent(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	unsigned long msg = 0;
	bool isFiltered = false;
	unsigned int chNo = 0;
	unsigned int noteNo = 0;

	//Get message
	result = pMIDIEvent->GetMIDIOutShortMsg(&msg);
	if (result != 0) goto EXIT;

	//MIDI event filter
	result = _FilterMIDIEvent(portNo, pMIDIEvent, &isFiltered);
	if (result != 0) goto EXIT;

	//MIDIEvent transmission
	if (!isFiltered) {
		//Message output: control does not return until output completes
		result = m_OutDevCtrl.SendShortMsg(portNo, msg);
		if (result != 0) goto EXIT;

		//Post the MIDI event message
		result =  m_EventWatcher.WatchEventMIDI(portNo, pMIDIEvent);
		if (result != 0) goto EXIT;
	}

	//Update note state
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOn) {
		chNo = pMIDIEvent->GetChNo();
		noteNo = pMIDIEvent->GetNoteNo();
		if ((portNo < SM_MIDIOUT_PORT_NUM_MAX) && (chNo < SM_MAX_CH_NUM) && (noteNo < SM_MAX_NOTE_NUM)) {
			m_NoteVelocity[portNo][chNo][noteNo] = pMIDIEvent->GetVelocity();
		}
	}
	else if (pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOff) {
		chNo = pMIDIEvent->GetChNo();
		noteNo = pMIDIEvent->GetNoteNo();
		if ((portNo < SM_MIDIOUT_PORT_NUM_MAX) && (chNo < SM_MAX_CH_NUM) && (noteNo < SM_MAX_NOTE_NUM)) {
			m_NoteVelocity[portNo][chNo][noteNo] = 0;
		}
	}

	//Count note-on
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOn) {
		m_NotesCount++;
	}

	//Control change monitoring
	//  Monitor RPN in order to pick up the pitch bend sensitivity
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::ControlChange) {
		result = m_EventWatcher.WatchEventControlChange(portNo, pMIDIEvent);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// SysExEvent transmission
//******************************************************************************
int SMSequencer::_SendSysExEvent(
		unsigned char portNo,
		SMEventSysEx* pSysExEvent
	)
{
	int result = 0;
	unsigned char* pVarMsg = NULL;
	unsigned long size = 0;

	//Get message
	pSysExEvent->GetMIDIOutLongMsg(&pVarMsg, &size);

	//Message output: control does not return until output completes
	result = m_OutDevCtrl.SendLongMsg(portNo, pVarMsg, size);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Meta event transmission
//******************************************************************************
int SMSequencer::_SendMetaEvent(
		unsigned char portNo,
		SMEventMeta* pMetaEvent
	)
{
	int result = 0;

	//Meta events are not sent to the MIDI device

	//Tempo info
	if (pMetaEvent->GetType() == 0x51) {
		//Reflect in the delta time calculation
		m_Tempo = pMetaEvent->GetTempo();
		if (m_Tempo == 0) {
			//Invalid data
			result = YN_SET_ERR("Invalid data found.", 0, 0);
			goto EXIT;
		}

		//Notify
		if (!m_isSkipping) {
			m_MsgTrans.PostTempo(m_Tempo);
		}
	}

	//Time signature
	if (pMetaEvent->GetType() == 0x58) {
		//Get numerator and denominator
		unsigned long numerator = 0;
		unsigned long denominator = 0;
		pMetaEvent->GetTimeSignature(&numerator, &denominator);
		if (denominator == 0) {
			//Invalid data
			result = YN_SET_ERR("Invalid data found.", numerator, denominator);
			goto EXIT;
		}
		m_BeatNumerator = numerator;
		m_BeatDenominator = denominator;

		//Notify
		if (!m_isSkipping) {
			m_MsgTrans.PostBeat((unsigned short)numerator, (unsigned short)denominator);
		}

		//Update the tick time per bar
		m_TickTimeOfBar = (numerator * m_TimeDivision * 4) / denominator;

		//Notify as the start of bar 1 since the time signature was updated
		if (m_PrevBarTickTime != m_TotalTickTime) {
			m_CurBarNo++;
			m_PrevBarTickTime = m_TotalTickTime;
			if (!m_isSkipping) {
				m_MsgTrans.PostBar(m_CurBarNo);
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// User request processing
//******************************************************************************
int SMSequencer::_ProcUserRequest(
		BOOL* pIsContinue
	)
{
	int result = 0;

	if (m_UserRequest == RequestNone) goto EXIT;

	//Send note-off for active notes, considering MIDI sound modules that do not support the All Notes Off message
	result = _SendNoteOffForActiveNotes();
	if (result != 0) goto EXIT;

	//All tracks note off
	result = _AllTrackNoteOff();
	if (result != 0) goto EXIT;

	//All tracks sound off
	result = _AllTrackSoundOff();
	if (result != 0) goto EXIT;

	*pIsContinue = false;

	//If a pause was requested
	if (m_UserRequest == RequestPause) {
		m_Status = StatusPause;
		m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_PAUSE);
	}

	//If a stop was requested
	if (m_UserRequest == RequestStop) {
		m_Status = StatusStop;
		m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_STOP);
	}

	//If a skip was requested
	if (m_UserRequest == RequestSkip) {
		*pIsContinue = true;
		//Clip at nominal song length + margin (safety net)
		unsigned long long skipTarget = m_SkipTargetTime;
		unsigned long long endTimeWithMargin = m_TotalPlayTimeNano + 100000000ULL;
		if (m_TotalPlayTimeNano > 0 && skipTarget > endTimeWithMargin) {
			skipTarget = endTimeWithMargin;
		}
		result = _ProcSkip(skipTarget, pIsContinue);
		if (result != 0) goto EXIT;
	}

	m_UserRequest = RequestNone;

EXIT:;
	return result;
}

//******************************************************************************
// Send note-off for active notes
//******************************************************************************
int SMSequencer::_SendNoteOffForActiveNotes()
{
	int result = 0;
	unsigned int portNo = 0;
	unsigned int chNo = 0;
	unsigned int noteNo = 0;
	unsigned int velocity = 0;
	unsigned long msg = 0;
	
	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
			for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
				velocity = m_NoteVelocity[portNo][chNo][noteNo];
				if (velocity > 0) {
					msg = (unsigned long)((0 << 16) | (noteNo << 8) | (0x80 | chNo));
					result = m_OutDevCtrl.SendShortMsg(portNo, msg);
					if (result != 0) goto EXIT;
				}
			}
		}
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// Send note-on for active notes
//******************************************************************************
int SMSequencer::_SendNoteOnForActiveNotes()
{
	int result = 0;
	unsigned int portNo = 0;
	unsigned int chNo = 0;
	unsigned int noteNo = 0;
	unsigned int velocity = 0;
	unsigned long msg = 0;
	
	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
			for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
				velocity = m_NoteVelocity[portNo][chNo][noteNo];
				if (velocity > 0) {
					msg = (unsigned long)((velocity << 16) | (noteNo << 8) | (0x90 | chNo));
					result = m_OutDevCtrl.SendShortMsg(portNo, msg);
					if (result != 0) goto EXIT;
				}
			}
		}
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// All tracks note off
//******************************************************************************
int SMSequencer::_AllTrackNoteOff()
{
	int result = 0;

	result = m_OutDevCtrl.NoteOffAll();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// All tracks sound off
//******************************************************************************
int SMSequencer::_AllTrackSoundOff()
{
	int result = 0;

	result = m_OutDevCtrl.SoundOffAll();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Get current time (nanoseconds)
//******************************************************************************
unsigned long long SMSequencer::_GetCurTimeInNano()
{
	return ((unsigned long long)(timeGetTime()) * 1000000);
}

//******************************************************************************
// Initialize parameters on play start
//******************************************************************************
int SMSequencer::_InitializeParamsOnPlayStart()
{
	int result = 0;
	unsigned long deltaTime = 0;

	//Reset the playback position to the start of the song
	m_PlayIndex = 0;
	result = m_Track.GetDataSet(m_PlayIndex, &deltaTime, &m_Event, &m_PortNo);
	if (result != 0) goto EXIT;

	m_PrevTimerTime = _GetCurTimeInNano();
	m_CurPlayTime = 0;
	m_PrevEventTime = 0;
	m_NextEventTime = _ConvTick2TimeNanosec(deltaTime);
	m_NextNtcTime = 0;
	m_PrevDeltaTime = deltaTime;
	m_TotalTickTime = 0;
	m_TotalTickTimeTemp = 0;
	m_CurBarNo = 1;
	m_PrevBarTickTime = 0;
	m_NotesCount = 0;

	//Initialize event watcher
	result = m_EventWatcher.Initialize(&m_MsgTrans);
	if (result != 0) goto EXIT;

	//Clear note velocity
	_ClearNoteVelocity();

EXIT:;
	return result;
}

//******************************************************************************
// Clear MIDI event cache
//******************************************************************************
void SMSequencer::_ClearMIDIEventCache()
{
	unsigned long portNo = 0;
	unsigned long chNo = 0;

	for (portNo = 0; portNo < SM_MAX_PORT_NUM; portNo++) {
		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
			m_CachePitchBend[portNo][chNo][0] = 0xFF;
			m_CachePitchBend[portNo][chNo][1] = 0xFF;
			m_CacheCC001_Modulation[portNo][chNo] = 0xFF;
			m_CacheCC007_Volume[portNo][chNo] = 0xFF;
			m_CacheCC010_Panpot[portNo][chNo] = 0xFF;
			m_CacheCC011_Expression[portNo][chNo] = 0xFF;
		}
	}

	return;
}

//******************************************************************************
// MIDI event filter
//******************************************************************************
int SMSequencer::_FilterMIDIEvent(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent,
		bool* pIsFiltered
	)
{
	int result = 0;
	unsigned char* pData = NULL;
	unsigned long shortMsg = 0;
	unsigned long size = 0;
	unsigned char chNo = 0;
	unsigned char ccNo = 0;
	unsigned char ccValue = 0;

	*pIsFiltered = false;

	//Only filter while skipping
	if (!m_isSkipping) goto EXIT;

	chNo = pMIDIEvent->GetChNo();

	//Do not send note on/off
	if ((pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOff) ||
		(pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOn)) {
		*pIsFiltered = true;
	}

	//Do not send pitch bend
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::PitchBend) {
		*pIsFiltered = true;
		result = pMIDIEvent->GetMIDIOutShortMsg(&shortMsg);
		if (result != 0) goto EXIT;

		//Remember the pitch bend value: refer to bytes 2 and 3 (En dl dm)
		pData = (unsigned char*)(&shortMsg);
		m_CachePitchBend[portNo][chNo][0] = pData[1];
		m_CachePitchBend[portNo][chNo][1] = pData[2];
	}

	//Do not send some control changes
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::ControlChange) {
		ccNo = pMIDIEvent->GetCCNo();
		ccValue = pMIDIEvent->GetCCValue();

		//CC#1 Modulation
		if (ccNo == 1) {
			*pIsFiltered = true;
			m_CacheCC001_Modulation[portNo][chNo] = ccValue;
		}
		//CC#7 Volume
		else if (ccNo == 7) {
			*pIsFiltered = true;
			m_CacheCC007_Volume[portNo][chNo] = ccValue;
		}
		//CC#10 Panpot
		else if (ccNo == 10) {
			*pIsFiltered = true;
			m_CacheCC010_Panpot[portNo][chNo] = ccValue;
		}
		//CC#11 Expression
		else if (ccNo == 11) {
			*pIsFiltered = true;
			m_CacheCC011_Expression[portNo][chNo] = ccValue;
		}
		//CC#121 Reset All Controllers
		else if (ccNo == 121) {
			//Discard the cache for the parameters to be cleared
			m_CachePitchBend[portNo][chNo][0] = 0xFF;
			m_CachePitchBend[portNo][chNo][1] = 0xFF;
			m_CacheCC001_Modulation[portNo][chNo] = 0xFF;
			//Excluded m_CacheCC007_Volume[portNo][chNo] = 0xFF;
			//Excluded m_CacheCC010_Panpot[portNo][chNo] = 0xFF;
			m_CacheCC011_Expression[portNo][chNo] = 0xFF;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Send MIDI event cache
//******************************************************************************
int SMSequencer::_SendMIDIEventCache()
{
	int result = 0;
	unsigned long index = 0;
	unsigned char portNo = 0;
	unsigned char chNo = 0;
	unsigned char pitchBend[2];
	unsigned char ccValue = 0;

	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		portNo = (unsigned char)index;
		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
			//Pitch bend
			pitchBend[0] = m_CachePitchBend[portNo][chNo][0];
			pitchBend[1] = m_CachePitchBend[portNo][chNo][1];
			if (pitchBend[0] < 0xFF) {
				result = _SendMIDIEventPitchBend(portNo, chNo, pitchBend);
				if (result != 0) goto EXIT;
			}
			//CC#1 Modulation
			ccValue = m_CacheCC001_Modulation[portNo][chNo];
			if (ccValue < 0x80) {
				result = _SendMIDIEventCC(portNo, chNo, 1, ccValue);
				if (result != 0) goto EXIT;
			}
			//CC#7 Volume
			ccValue = m_CacheCC007_Volume[portNo][chNo];
			if (ccValue < 0x80) {
				result = _SendMIDIEventCC(portNo, chNo, 7, ccValue);
				if (result != 0) goto EXIT;
			}
			//CC#10 Panpot
			ccValue = m_CacheCC010_Panpot[portNo][chNo];
			if (ccValue < 0x80) {
				result = _SendMIDIEventCC(portNo, chNo, 10, ccValue);
				if (result != 0) goto EXIT;
			}
			//CC#11 Expression
			ccValue = m_CacheCC011_Expression[portNo][chNo];
			if (ccValue < 0x80) {
				result = _SendMIDIEventCC(portNo, chNo, 11, ccValue);
				if (result != 0) goto EXIT;
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Send MIDI event cache: pitch bend
//******************************************************************************
int SMSequencer::_SendMIDIEventPitchBend(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char* pPtichBend
	)
{
	int result = 0;
	SMEvent event;
	SMEventMIDI eventMIDI;

	//Create MIDI event data
	event.SetMIDIData(0xE0 | chNo, pPtichBend, 2);
	eventMIDI.Attach(&event);

	//MIDIEvent transmission
	result = _SendMIDIEvent(portNo, &eventMIDI);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Send MIDI event cache: control change
//******************************************************************************
int SMSequencer::_SendMIDIEventCC(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char ccNo,
		unsigned char ccValue
	)
{
	int result = 0;
	unsigned char data[2];
	SMEvent event;
	SMEventMIDI eventMIDI;

	//Create MIDI event data
	data[0] = ccNo;
	data[1] = ccValue;
	event.SetMIDIData(0xB0 | chNo, data, 2);
	eventMIDI.Attach(&event);

	//MIDIEvent transmission
	result = _SendMIDIEvent(portNo, &eventMIDI);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Skip processing
//******************************************************************************
int SMSequencer::_ProcSkip(
		unsigned long long targetTimeInNanoSec,
		BOOL* pIsContinue
	)
{
	int result = 0;
	unsigned long long startPlayTime = 0;
	unsigned long startTickTime = 0;
	unsigned long endTickTime = 0;

	if (m_Status != StatusPlay) goto EXIT;

	startPlayTime = m_CurPlayTime;
	startTickTime = m_TotalTickTimeTemp;

	//If skipping backward
	if (targetTimeInNanoSec < m_CurPlayTime) {
		//Initialize parameters on play start
		result = _InitializeParamsOnPlayStart();
		if (result != 0) goto EXIT;

		m_MsgTrans.PostSkipStart(SM_SKIP_BACK);
	}
	//If skipping forward
	else {
		m_MsgTrans.PostSkipStart(SM_SKIP_FORWARD);
	}

	//Clear MIDI event cache
	_ClearMIDIEventCache();

	//Process MIDI events up to the specified time
	m_isSkipping = true;
	while (*pIsContinue) {
		//Thread interval processing
		result = _IntervalProc(pIsContinue);
		if (result != 0) goto EXIT;

		//End the skip once the specified time is reached
		if (targetTimeInNanoSec <= m_CurPlayTime) break;
	}
	m_isSkipping = false;

	//Send cache
	result = _SendMIDIEventCache();
	if (result != 0) goto EXIT;

	//Move the playback time
	endTickTime = m_TotalTickTimeTemp;
	_SlidePlaybackTime(startPlayTime, startTickTime, endTickTime);

	//Send note-on per active note
	result = _SendNoteOnForActiveNotes();
	if (result != 0) goto EXIT;

	//Notify the state at the skip destination
	m_MsgTrans.PostPlayTime((unsigned long)(m_CurPlayTime/1000000), endTickTime);
	m_MsgTrans.PostTempo(m_Tempo);
	m_MsgTrans.PostBeat((unsigned short)m_BeatNumerator, (unsigned short)m_BeatDenominator);
	m_MsgTrans.PostBar(m_CurBarNo);

	//Update the playback start time
	m_PrevTimerTime = _GetCurTimeInNano();

	//End of skip
	m_MsgTrans.PostSkipEnd(m_NotesCount);

	//End of playback due to a forward skip
	if (!(*pIsContinue)) {
		_AllTrackNoteOff();
		m_MsgTrans.PostPlayTime((unsigned long)(m_CurPlayTime/1000000), m_TotalTickTime);
		m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_STOP);
		m_Status = StatusStop;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Move the playback time
//******************************************************************************
void SMSequencer::_SlidePlaybackTime(
		unsigned long long startPlayTime,
		unsigned long startTickTime,
		unsigned long endTickTime
	)
{
	unsigned long i = 0;
	unsigned long tickTime = 0;
	unsigned long tickTimeStep = 0;
	unsigned long waitTimeInMsec = 10;  //Notify every 10msec.
	unsigned long stepNum = 0;
	bool isRewind = false;

	//Number of playback time notifications
	stepNum = m_MovingTimeSpanInMsec / waitTimeInMsec;

	//Tick time step value
	if (startTickTime > endTickTime) {
		isRewind = true;
		tickTimeStep = (startTickTime - endTickTime) / stepNum;
	}
	else {
		isRewind = false;
		tickTimeStep = (endTickTime - startTickTime) / stepNum;
	}

	//Move the playback time
	tickTime = startTickTime;
	for (i = 0; i < stepNum; i ++) {
		//Notify playback time: only the tick time is updated
		if (isRewind) {
			tickTime -= tickTimeStep;
		}
		else {
			tickTime += tickTimeStep;
		}
		m_MsgTrans.PostPlayTime((unsigned long)(startPlayTime/1000000), tickTime);

		//Wait
		Sleep(waitTimeInMsec);
	}

	return;
}

//******************************************************************************
// Clear note velocity
//******************************************************************************
void SMSequencer::_ClearNoteVelocity()
{
	memset(m_NoteVelocity, 0, sizeof(unsigned char) * SM_MIDIOUT_PORT_NUM_MAX * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);
}

//******************************************************************************
// Timer invocation
//******************************************************************************
int SMSequencer::_OnTimer()
{
	int result = 0;
	BOOL isContinue = true;

	unsigned long deltaTime = 0;

	//Prevent reentrancy (excludes concurrent callbacks if the while loop exceeds 1ms)
	if (m_isInTimer) {
		return 0;
	}
	m_isInTimer = true;

	//Set floating-point precision to double
	//  Executed only once, right after the timer starts
	if (!(m_FPUCtrl.IsLocked())) {
		result = m_FPUCtrl.Start(SMFPUCtrl::FPUDouble);
		if (result != 0) goto EXIT;
	}

	//Thread interval processing
	result = _IntervalProc(&isContinue);
	if (result != 0) goto EXIT;

	//Process the user request
	if (isContinue) {
		result = _ProcUserRequest(&isContinue);
		if (result != 0) goto EXIT;
	}

	if (!isContinue) {
		timeKillEvent(m_TimerID);
		m_TimerID = NULL;
		m_FPUCtrl.End();
	}

EXIT:;
	m_isInTimer = false;
	return result;
}

//******************************************************************************
// Timer callback function
//******************************************************************************
void SMSequencer::_TimerCallBack(
		UINT uTimerID,
		UINT uMsg,
		DWORD_PTR dwUser,
		DWORD_PTR dw1,
		DWORD_PTR dw2
	)
{
	int result = 0;

	SMSequencer* pSequencer = (SMSequencer*)dwUser;
	if (pSequencer == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = pSequencer->_OnTimer();
	if (result != 0) goto EXIT;

EXIT:;
	if (result != 0) YN_SHOW_ERR(NULL);
	return;
}

} // end of namespace

