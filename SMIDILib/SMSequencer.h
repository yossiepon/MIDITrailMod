//******************************************************************************
//
// Simple MIDI Library / SMSequencer
//
// �V�[�P���T�N���X
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

#include "mmsystem.h"
#include "SMEventMIDI.h"
#include "SMEventSysEx.h"
#include "SMEventMeta.h"
#include "SMSeqData.h"
#include "SMMsgTransmitter.h"
#include "SMOutDevCtrl.h"
#include "SMCommon.h"
#include "SMFPUCtrl.h"
#include "SMEventWatcher.h"

namespace SMIDILib {


//******************************************************************************
// �V�[�P���T�N���X
//******************************************************************************
class SMIDILIB_API SMSequencer
{
public:

	//���t���
	enum Status {
		StatusPlay,
		StatusPause,
		StatusStop
	};

	//���[�U�v��
	enum UserRequest {
		RequestNone,
		RequestPause,
		RequestStop,
		RequestSkip
	};

	//�R���X�g���N�^�^�f�X�g���N�^
	SMSequencer(void);
	virtual ~SMSequencer(void);

	//������
	int Initialize(HWND hTargetWnd, unsigned long msgId);

	//�|�[�g�Ή��f�o�C�X�o�^
	int SetPortDev(unsigned char portNo, const char* pProductName);

	//�V�[�P���X�f�[�^�o�^
	int SetSeqData(SMSeqData* pSeqData);

	//���t�J�n
	int Play();

	//���t�ꎞ��~
	void Pause();

	//���t�ĊJ
	int Resume();

	//���t��~
	void Stop();

	//�Đ��X�s�[�h�ݒ�
	void SetPlaybackSpeed(unsigned long nTimes); //n�{��
	void SetPlaySpeedRatio(unsigned long ratio); //�p�[�Z���g

	//�����C���h�^�X�L�b�v�ړ����Ԑݒ�
	void SetMovingTimeSpanInMsec(unsigned long timeSpan);

	//���t�ʒu�ύX
	int Skip(int relativeTimeInMsec);

private:

	//���t���
	Status m_Status;
	unsigned long m_PlayIndex;
	UserRequest m_UserRequest;
	SMMsgTransmitter m_MsgTrans;
	SMEventWatcher m_EventWatcher;

	//MIDI�f�o�C�X�n
	SMOutDevCtrl m_OutDevCtrl;
	unsigned char m_PortNo;
	char m_PortDevName[SM_MIDIOUT_PORT_NUM_MAX][MAXPNAMELEN];

	//MIDI�f�[�^�n
	SMSeqData* m_pSeqData;
	SMTrack m_Track;
	SMEvent m_Event;

	//�^�C�}�[����n
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

	//���������_���Z����
	SMFPUCtrl m_FPUCtrl;

	//���ߔԍ�����n
	unsigned long m_TickTimeOfBar;
	unsigned long m_CurBarNo;
	unsigned long m_PrevBarTickTime;

	//���q�L��
	unsigned long m_BeatNumerator;
	unsigned long m_BeatDenominator;

	//�X�L�b�v����
	bool m_isSkipping;
	unsigned long long m_SkipTargetTime;
	unsigned long m_NotesCount;
	unsigned long m_MovingTimeSpanInMsec;
	unsigned char m_CachePitchBend[SM_MAX_PORT_NUM][SM_MAX_CH_NUM][2];
	unsigned char m_CacheCC001_Modulation[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_CacheCC007_Volume[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_CacheCC010_Panpot[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_CacheCC011_Expression[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];

	//�^�C�}�[�f�o�C�X����
	int _InitializeTimerDev();
	int _ReleaseTimerDev();

	//�|�[�g����
	void _ClearPortInfo();
	int _OpenMIDIOutDev();
	int _CloseMIDIOutDev();

	//�Đ�����
	int _InitializeParamsOnPlayStart();

	//���t�X���b�h�C���^�[�o������
	int _IntervalProc(BOOL* pIsContinue);

	//���Ԑ���
	int _UpdatePlayPosition();
	double _ConvTick2TimeNanosec(unsigned long tickTime);
	unsigned long _ConvTimeNanosec2Tick(unsigned long long timeMsec);
	unsigned long long _GetCurTimeInNano();

	//MIDI�o�͏���
	int _OutputMIDIEvent(unsigned char portNo, SMEvent* pEvent);
	int _SendMIDIEvent(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int _SendSysExEvent(unsigned char portNo, SMEventSysEx* pSysExEvent);
	int _SendMetaEvent(unsigned char portNo, SMEventMeta* pMetaEvent);
	int _AllTrackNoteOff();

	//���̑�
	int _ProcUserRequest(BOOL* pIsContinue);

	//�X�L�b�v����
	void _ClearMIDIEventCache();
	int _FilterMIDIEvent(unsigned char portNo, SMEventMIDI* pMIDIEvent, bool* pIsFiltered);
	int _SendMIDIEventCache();
	int _SendMIDIEventPitchBend(unsigned char portNo, unsigned char chNo, unsigned char* pPtichBend);
	int _SendMIDIEventCC(unsigned char portNo, unsigned char chNo, unsigned char ccNo, unsigned char ccValue);
	int _ProcSkip(unsigned long long targetTimeInNanoSec, BOOL* pIsContinue);
	void _SlidePlaybackTime(unsigned long long startPlayTime, unsigned long startTickTime, unsigned long endTickTime);

	//�^�C�}�[����
	static void CALLBACK _TimerCallBack(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

protected:

	int _OnTimer();

private:

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMSequencer&);
	SMSequencer(const SMSequencer&);

};

} // end of namespace

