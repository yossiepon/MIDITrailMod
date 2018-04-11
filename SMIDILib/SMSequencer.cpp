//******************************************************************************
//
// Simple MIDI Library / SMSequencer
//
// �V�[�P���T�N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �^�C�}�[�X���b�h�͉��t�������s�����߁AMIDI�o�̓f�o�C�X�̐���ɐ�O
// ������B���̃X���b�h�ŉ�ʍX�V���������s���Ă͂Ȃ�Ȃ��B
// ���X���b�h�ւ̒ʒm��PostMessage���Ŏ�������B
// _TimerCallBack()��_OnTimer()���E�E�E

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
// �R���X�g���N�^
//******************************************************************************
SMSequencer::SMSequencer(void)
{
	m_Status = StatusStop;
	m_PlayIndex = 0;
	m_UserRequest = RequestNone;
	m_PortNo = 0;
	m_pSeqData = NULL;
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

	//�X�L�b�v����
	m_isSkipping = false;
	m_SkipTargetTime = 0;
	m_NotesCount = 0;
	m_MovingTimeSpanInMsec = 0;

	//���ߔԍ�����n
	m_TickTimeOfBar = 0;
	m_CurBarNo = 1;
	m_PrevBarTickTime = 0;

	//���q�L��
	m_BeatNumerator = 0;
	m_BeatDenominator = 0;

	//�|�[�g���N���A
	_ClearPortInfo();
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMSequencer::~SMSequencer(void)
{
	//�|�[�g���N���A
	_ClearPortInfo();

	//MIDI�o�̓f�o�C�X�����
	_CloseMIDIOutDev();

	//�^�C�}�f�o�C�X���
	_ReleaseTimerDev();
}

//******************************************************************************
// ������
//******************************************************************************
int SMSequencer::Initialize(
		HWND hTargetWnd,
		unsigned long msgId
	)
{
	int result = 0;

	if (m_Status != StatusStop) {
		result = YN_SET_ERR("Program error.", m_Status, 0);
		goto EXIT;
	}

	//MIDI�o�̓f�o�C�X������
	result = m_OutDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	//�|�[�g���N���A
	_ClearPortInfo();

	//�C�x���g�]���I�u�W�F�N�g������
	result = m_MsgTrans.Initialize(hTargetWnd, msgId);
	if (result != 0) goto EXIT;

	//�C�x���g�E�H�b�`���[������
	result = m_EventWatcher.Initialize(&m_MsgTrans);
	if (result != 0) goto EXIT;

	//�^�C�}�f�o�C�X������
	result = _InitializeTimerDev();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �|�[�g�Ή��f�o�C�X�o�^
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
// �V�[�P���X�f�[�^�o�^
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

	//�}�[�W�ς݃g���b�N�擾
	result = m_pSeqData->GetMergedTrack(&m_Track);
	if (result != 0) goto EXIT;

	//����\�擾�F�l�������̒����������l (ex. 48, 480, ...)
	m_TimeDivision = m_pSeqData->GetTimeDivision();
	if (m_TimeDivision == 0) {
		//�f�[�^�ُ�FSMF�ǂݍ��ݎ��Ƀ`�F�b�N���Ă���͂�
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�e���|�擾
	m_Tempo = m_pSeqData->GetTempo();
	if (m_Tempo == 0) {
		//�f�[�^�ُ�
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}

	//���q�L������1���߂�����̃`�b�N�^�C�����Z�o
	numerator = m_pSeqData->GetBeatNumerator();
	denominator = m_pSeqData->GetBeatDenominator();
	if (denominator == 0) {
		//�f�[�^�ُ�
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
// ���t�J�n
//******************************************************************************
int SMSequencer::Play()
{
	int result = 0;
	SMFPUCtrl fpuCtrl;

	if (m_pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//���t���Ȃ牽�����Ȃ�
	if (m_Status == StatusPlay) goto EXIT;

	//���������_���Z���x��{���x�ɐݒ�
	result = fpuCtrl.Start(SMFPUCtrl::FPUDouble);
	if (result != 0) goto EXIT;

	//�擪���牉�t�J�n
	if (m_Status == StatusStop) {
		//�Đ��J�n�p�����[�^������
		result = _InitializeParamsOnPlayStart();
		if (result != 0) goto EXIT;

		//MIDI�o�̓f�o�C�X���J��
		result = _OpenMIDIOutDev();
		if (result != 0) goto EXIT;
	}
	//�ꎞ��~���牉�t�ĊJ
	if (m_Status == StatusPause) {
		m_PrevTimerTime = _GetCurTimeInNano();
	}
	m_Status = StatusPlay;
	m_UserRequest = RequestNone;
	m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_PLAY);

	//�^�C�}�N��
	m_TimerID = timeSetEvent(
					m_TimerResolution, //�C�x���g�x���i�~���b�j
					m_TimerResolution, //�C�x���g����\�i�~���b�j
					_TimerCallBack,    //�R�[���o�b�N�֐�
					(DWORD_PTR)this,   //���[�U�[�R�[���o�b�N�f�[�^
					TIME_PERIODIC      //�^�C�}�[��ʁF�����Ăяo��
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
// ���t�ꎞ��~
//******************************************************************************
void SMSequencer::Pause()
{
	//�v�����󂯕t���邾���i�L���[�C���O�͂��Ȃ��j
	//���ۂ̏����̓^�C�}�[�X���b�h�ɈϔC����
	m_UserRequest = RequestPause;
}

//******************************************************************************
// ���t�ĊJ
//******************************************************************************
int SMSequencer::Resume()
{
	int result = 0;

	//���݂�Play()���ĊJ���������˂Ă���
	result = Play();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���t��~
//******************************************************************************
void SMSequencer::Stop()
{

	if (m_Status == StatusPause) {
		//�ꎞ��~���̏ꍇ�̓^�C�}�[�X���b�h����~���Ă��邽��
		//��������I����ʒm����
		m_Status = StatusStop;
		m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_STOP);
	}
	else {
		//���t���͗v�����󂯕t���邾���i�L���[�C���O�͂��Ȃ��j
		//���ۂ̏����̓^�C�}�[�X���b�h�ɈϔC����
		m_UserRequest = RequestStop;
	}
}

//******************************************************************************
// �Đ��X�s�[�h�ݒ�in�{���j
//******************************************************************************
void SMSequencer::SetPlaybackSpeed(
		unsigned long nTimes
	)
{
	m_PlaybackSpeed =  nTimes;
}

//******************************************************************************
// �Đ��X�s�[�h�ݒ�i�p�[�Z���g�j
//******************************************************************************
void SMSequencer::SetPlaySpeedRatio(
		unsigned long ratio
	)
{
	m_PlaySpeedRatio =  (double)ratio / 100.0;
}

//******************************************************************************
// �����C���h�^�X�L�b�v�ړ����Ԑݒ�
//******************************************************************************
void SMSequencer::SetMovingTimeSpanInMsec(
		unsigned long timeSpan
	)
{
	m_MovingTimeSpanInMsec = timeSpan;
}

//******************************************************************************
//���t�ʒu�X�L�b�v
//******************************************************************************
int SMSequencer::Skip(
		int relativeTimeInMsec
	)
{
	int result = 0;
	unsigned long long diffTime = 0;

	//���t���łȂ���Ή������Ȃ�
	if (m_Status != StatusPlay) goto EXIT;

	//���t�ʒu
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
		//�Ȃ̏I�����Ԃ𒴂���\��������
	}

	//���t���͗v�����󂯕t���邾���i�L���[�C���O�͂��Ȃ��j
	//���ۂ̏����̓^�C�}�[�X���b�h�ɈϔC����
	m_UserRequest = RequestSkip;

EXIT:;
	return result;
}

//******************************************************************************
// �^�C�}�f�o�C�X������
//******************************************************************************
int SMSequencer::_InitializeTimerDev()
{
	int result = 0;
	UINT apiresult = 0;
	TIMECAPS tc;

	if (m_TimerResolution != 0) goto EXIT;

	//�^�C�}�f�o�C�X�̍ŏ�����\���擾�i�ʏ�1ms�j
	apiresult = timeGetDevCaps(&tc, sizeof(TIMECAPS));
	if (apiresult != TIMERR_NOERROR) {
		result = YN_SET_ERR("Timer device error.", apiresult, 0);
		goto EXIT;
	}
	m_TimerResolution = tc.wPeriodMin;

	//�ŏ��^�C�}����\�̐ݒ�
	timeBeginPeriod(m_TimerResolution);

EXIT:;
	return result;
}

//******************************************************************************
// �^�C�}�f�o�C�X���
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
// �|�[�g���N���A
//******************************************************************************
void SMSequencer::_ClearPortInfo()
{
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortDevName[portNo][0] = '\0';
	}
}

//******************************************************************************
// MIDI�o�̓f�o�C�X�I�[�v��
//******************************************************************************
int SMSequencer::_OpenMIDIOutDev()
{
	int result = 0;
	unsigned char portNo = 0;

	//�|�[�g�Ή��f�o�C�X����MIDI�o�̓f�o�C�X����ɓo�^
	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		if (strlen(m_PortDevName[portNo]) > 0) {
			result = m_OutDevCtrl.SetPortDev(portNo, m_PortDevName[portNo]);
			if (result != 0) goto EXIT;
		}
	}

	//�S�|�[�g�̃f�o�C�X���J��
	result = m_OutDevCtrl.OpenPortDevAll();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// MIDI�o�̓f�o�C�X�N���[�Y
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
// ���t�C���^�[�o������
//******************************************************************************
int SMSequencer::_IntervalProc(
		BOOL* pIsContinue
	)
{
	int result = 0;
	unsigned long deltaTime = 0;

	*pIsContinue = true;

	//���t�ʒu���X�V
	result = _UpdatePlayPosition();
	if (result != 0) goto EXIT;

	//�C�x���g���������ɓ��B���Ă����瑗�M�������s��
	if ((unsigned long long)m_NextEventTime <= m_CurPlayTime) {

		//�`�b�N�^�C�����v
		m_TotalTickTime += m_PrevDeltaTime;

		while (deltaTime == 0) {
			//�C�x���g���M
			result = _OutputMIDIEvent(m_PortNo, &m_Event);
			if (result != 0) goto EXIT;

			//�f�[�^�I�[�Ȃ牉�t�I��
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

			//���C�x���g�擾
			m_Track.GetDataSet(m_PlayIndex, &deltaTime, &m_Event, &m_PortNo);
		}
		//����ʒm�̂��߃C�x���g�����������L������
		//����ʒm�͌����Ȑ��x��K�v�Ƃ��Ȃ�����1msec�����͖�������
		m_PrevEventTime = (unsigned long long)m_NextEventTime;

		//���C�x���g���M�ʒu���Z�o
		m_NextEventTime += _ConvTick2TimeNanosec(deltaTime);
		m_PrevDeltaTime = deltaTime;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���t�ʒu�X�V
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

	//�O��^�C�}�[����̌o�ߎ��Ԃ𗘗p���ĉ��t���Ԃ��X�V
	//  �N���ォ��49�����܂����P�[�X�ł����̌v�Z�Ŗ��Ȃ�
	diffTime = curTime - m_PrevTimerTime;

	//�O��^�C�}�[����̌o�ߎ��Ԃ𗘗p���ĉ��t���Ԃ��X�V
	if (m_isSkipping) {
		//�X�L�b�v���̏ꍇ�͉��z�I��5msec.�o�߂�����
		diffTime = 5 * 1000000;
	}
	else {
		//�X�L�b�v���łȂ���Ύ��ۂ̌o�ߎ��Ԃ��Z�o����
		diffTime = curTime - m_PrevTimerTime;
	}

	//�Đ��X�s�[�h�𔽉f�in�{���j
	if (m_PlaybackSpeed == 1) {
		diffTime = (unsigned long long)((double)diffTime * m_PlaySpeedRatio);
	}
	else {
		diffTime = diffTime * m_PlaybackSpeed;
	}

	m_CurPlayTime += diffTime;
	m_PrevTimerTime = curTime;

	//�O��C�x���g��������̌o�ߎ��Ԃ��`�b�N�^�C���Ɋ��Z
	//  �ϊ��덷�������邪�덷��~�ς����Ȃ����ߖ��Ȃ�
	diffTickTime = _ConvTimeNanosec2Tick(m_CurPlayTime - m_PrevEventTime);

	//�Ȑ擪����̃`�b�N�^�C�����v
	//m_TotalTickTime�̓C�x���g�������ɂ̂ݍX�V���邽�߂����ł͏��������Ȃ�
	m_TotalTickTimeTemp = m_TotalTickTime + diffTickTime;

	//�ʒm���Ԃɓ��B�����牉�t���Ԃ�ʒm����
	if ((m_NextNtcTime <= m_CurPlayTime) && (!m_isSkipping)) {
		m_MsgTrans.PostPlayTime((unsigned long)(m_CurPlayTime/1000000), m_TotalTickTimeTemp);
		//�ʒm�Ԋu��60FPS�\�����l������1,000,000,000/120[nanosec]�~�Đ��X�s�[�h�Ƃ���
		//TODO: �O������Ԋu���w��ł���悤�ɂ���
		ntcSpan = (unsigned long long)(1000000000.0 * m_PlaySpeedRatio / 120.0);
		m_NextNtcTime = m_CurPlayTime - (m_CurPlayTime % ntcSpan) + ntcSpan;
	}

	//���ߔԍ��X�V�̊m�F
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
// �`�b�N�^�C����������Ԃւ̕ϊ��i�i�m�b�j
//******************************************************************************
double SMSequencer::_ConvTick2TimeNanosec(
		unsigned long tickTime
	)
{
	double timeNanosec = 0;
	
	//(1) �l������������̕���\ division
	//    ��F48
	//(2) �g���b�N�f�[�^�̃f���^�^�C�� delta
	//    ����\�̒l��p���ĕ\�����鎞�ԍ�
	//    ����\��48�Ńf���^�^�C����24�Ȃ甪���������̎��ԍ�
	//(3) �e���|�ݒ�i�}�C�N���b�j tempo
	//    �l�������̎����ԊԊu
	//
	// �f���^�^�C���ɑΉ���������ԊԊu�i�~���b�j
	//  = (delta / division) * tempo / 1000
	//  = (delta * tempo) / (division * 1000)
	
	timeNanosec = ((double)tickTime * (double)m_Tempo) * 1000.0 / ((double)m_TimeDivision);
	
	return timeNanosec;
}

//******************************************************************************
// �����ԁi�i�m�b�j����`�b�N�^�C���ւ̕ϊ�
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
// �C�x���g���M����
//******************************************************************************
int SMSequencer::_OutputMIDIEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;

	//MIDI�C�x���g���M
	if (pEvent->GetType() == SMEvent::EventMIDI) {
		SMEventMIDI eventMIDI;
		eventMIDI.Attach(pEvent);
		result = _SendMIDIEvent(portNo, &eventMIDI);
		if (result != 0) goto EXIT;
	}
	//SysEx�C�x���g���M
	else if (pEvent->GetType() == SMEvent::EventSysEx) {
		SMEventSysEx eventSysEx;
		eventSysEx.Attach(pEvent);
		result = _SendSysExEvent(portNo, &eventSysEx);
		if (result != 0) goto EXIT;
	}
	//���^�C�x���g���M
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
// MIDI�C�x���g���M
//******************************************************************************
int SMSequencer::_SendMIDIEvent(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	unsigned long msg = 0;
	bool isFiltered = false;

	//���b�Z�[�W�擾
	result = pMIDIEvent->GetMIDIOutShortMsg(&msg);
	if (result != 0) goto EXIT;

	//MIDI�C�x���g�t�B���^
	result = _FilterMIDIEvent(portNo, pMIDIEvent, &isFiltered);
	if (result != 0) goto EXIT;

	//MIDI�C�x���g���M
	if (!isFiltered) {
		//���b�Z�[�W�o�́F�o�͊����܂Ő��䂪�߂�Ȃ�
		result = m_OutDevCtrl.SendShortMsg(portNo, msg);
		if (result != 0) goto EXIT;

		//MIDI�C�x���g���b�Z�[�W�|�X�g
		result =  m_EventWatcher.WatchEventMIDI(portNo, pMIDIEvent);
		if (result != 0) goto EXIT;
	}

	//�m�[�gON���J�E���g
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOn) {
		m_NotesCount++;
	}

	//�R���g���[���`�F���W�Ď�����
	//  �s�b�`�x���h���x���E������RPN���Ď�����
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::ControlChange) {
		result = m_EventWatcher.WatchEventControlChange(portNo, pMIDIEvent);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// SysEx�C�x���g���M
//******************************************************************************
int SMSequencer::_SendSysExEvent(
		unsigned char portNo,
		SMEventSysEx* pSysExEvent
	)
{
	int result = 0;
	unsigned char* pVarMsg = NULL;
	unsigned long size = 0;

	//���b�Z�[�W�擾
	pSysExEvent->GetMIDIOutLongMsg(&pVarMsg, &size);

	//���b�Z�[�W�o�́F�o�͊����܂Ő��䂪�߂�Ȃ�
	result = m_OutDevCtrl.SendLongMsg(portNo, pVarMsg, size);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���^�C�x���g���M
//******************************************************************************
int SMSequencer::_SendMetaEvent(
		unsigned char portNo,
		SMEventMeta* pMetaEvent
	)
{
	int result = 0;

	//���^�C�x���g��MIDI�f�o�C�X�ɑ��M���Ȃ�

	//�e���|���
	if (pMetaEvent->GetType() == 0x51) {
		//�f���^�^�C���v�Z�ɔ��f
		m_Tempo = pMetaEvent->GetTempo();
		if (m_Tempo == 0) {
			//�f�[�^�ُ�
			result = YN_SET_ERR("Invalid data found.", 0, 0);
			goto EXIT;
		}

		//�ʒm
		if (!m_isSkipping) {
			m_MsgTrans.PostTempo(m_Tempo);
		}
	}

	//���q�L��
	if (pMetaEvent->GetType() == 0x58) {
		//���q������擾
		unsigned long numerator = 0;
		unsigned long denominator = 0;
		pMetaEvent->GetTimeSignature(&numerator, &denominator);
		if (denominator == 0) {
			//�f�[�^�ُ�
			result = YN_SET_ERR("Invalid data found.", numerator, denominator);
			goto EXIT;
		}
		m_BeatNumerator = numerator;
		m_BeatDenominator = denominator;

		//�ʒm
		if (!m_isSkipping) {
			m_MsgTrans.PostBeat((unsigned short)numerator, (unsigned short)denominator);
		}

		//1���߂�����̃`�b�N�^�C�����X�V
		m_TickTimeOfBar = (numerator * m_TimeDivision * 4) / denominator;

		//���q�L���X�V�̂���1���ߖڊJ�n�n�_�Ƃ��Ēʒm
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
// ���[�U�v������
//******************************************************************************
int SMSequencer::_ProcUserRequest(
		BOOL* pIsContinue
	)
{
	int result = 0;

	if (m_UserRequest == RequestNone) goto EXIT;

	//�S�g���b�N�m�[�g�I�t
	result = _AllTrackNoteOff();
	if (result != 0) goto EXIT;

	*pIsContinue = false;

	//�ꎞ��~��v�����ꂽ�ꍇ
	if (m_UserRequest == RequestPause) {
		m_Status = StatusPause;
		m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_PAUSE);
	}

	//��~��v�����ꂽ�ꍇ
	if (m_UserRequest == RequestStop) {
		m_Status = StatusStop;
		m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_STOP);
	}

	//�X�L�b�v��v�����ꂽ�ꍇ
	if (m_UserRequest == RequestSkip) {
		*pIsContinue = true;
		result = _ProcSkip(m_SkipTargetTime, pIsContinue);
		if (result != 0) goto EXIT;
	}

	m_UserRequest = RequestNone;

EXIT:;
	return result;
}

//******************************************************************************
// �S�g���b�N�m�[�g�I�t
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
// ���ݎ����擾�i�i�m�b�j
//******************************************************************************
unsigned long long SMSequencer::_GetCurTimeInNano()
{
	return ((unsigned long long)(timeGetTime()) * 1000000);
}

//******************************************************************************
// �Đ��J�n�p�����[�^������
//******************************************************************************
int SMSequencer::_InitializeParamsOnPlayStart()
{
	int result = 0;
	unsigned long deltaTime = 0;

	//���t�ʒu���Ȃ̐擪�ɖ߂�
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

	//�C�x���g�E�H�b�`���[������
	result = m_EventWatcher.Initialize(&m_MsgTrans);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// MIDI�C�x���g�L���b�V���N���A
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
// MIDI�C�x���g�t�B���^
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

	//�X�L�b�v���̂݃t�B���^����
	if (!m_isSkipping) goto EXIT;

	chNo = pMIDIEvent->GetChNo();

	//�m�[�gON/OFF�͑��M���Ȃ�
	if ((pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOff) ||
		(pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOn)) {
		*pIsFiltered = true;
	}

	//�s�b�`�x���h�͑��M���Ȃ�
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::PitchBend) {
		*pIsFiltered = true;
		result = pMIDIEvent->GetMIDIOutShortMsg(&shortMsg);
		if (result != 0) goto EXIT;
		
		//�s�b�`�x���h�̒l���L������FEn dl dm ��2,3�o�C�g�ڂ��Q��
		pData = (unsigned char*)(&shortMsg);
		m_CachePitchBend[portNo][chNo][0] = pData[1];
		m_CachePitchBend[portNo][chNo][1] = pData[2];
	}

	//�R���g���[���`�F���W�̈ꕔ�͑��M���Ȃ�
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::ControlChange) {
		ccNo = pMIDIEvent->GetCCNo();
		ccValue = pMIDIEvent->GetCCValue();

		//CC#1 ���W�����[�V����
		if (ccNo == 1) {
			*pIsFiltered = true;
			m_CacheCC001_Modulation[portNo][chNo] = ccValue;
		}
		//CC#7 �{�����[��
		else if (ccNo == 7) {
			*pIsFiltered = true;
			m_CacheCC007_Volume[portNo][chNo] = ccValue;
		}
		//CC#10 �p���|�b�g
		else if (ccNo == 10) {
			*pIsFiltered = true;
			m_CacheCC010_Panpot[portNo][chNo] = ccValue;
		}
		//CC#11 �G�N�X�v���b�V����
		else if (ccNo == 11) {
			*pIsFiltered = true;
			m_CacheCC011_Expression[portNo][chNo] = ccValue;
		}
		//CC#121 ���Z�b�g�I�[���R���g���[��
		else if (ccNo = 121) {
			//�N���A�Ώۃp�����[�^�̃L���b�V����j������
			m_CachePitchBend[portNo][chNo][0] = 0xFF;
			m_CachePitchBend[portNo][chNo][1] = 0xFF;
			m_CacheCC001_Modulation[portNo][chNo] = 0xFF;
			//�ΏۊO m_CacheCC007_Volume[portNo][chNo] = 0xFF;
			//�ΏۊO m_CacheCC010_Panpot[portNo][chNo] = 0xFF;
			m_CacheCC011_Expression[portNo][chNo] = 0xFF;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDI�C�x���g�L���b�V�����M
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
			//�s�b�`�x���h
			pitchBend[0] = m_CachePitchBend[portNo][chNo][0];
			pitchBend[1] = m_CachePitchBend[portNo][chNo][1];
			if (pitchBend[0] < 0xFF) {
				result = _SendMIDIEventPitchBend(portNo, chNo, pitchBend);
				if (result != 0) goto EXIT;
			}
			//CC#1 ���W�����[�V����
			ccValue = m_CacheCC001_Modulation[portNo][chNo];
			if (ccValue < 0x80) {
				result = _SendMIDIEventCC(portNo, chNo, 1, ccValue);
				if (result != 0) goto EXIT;
			}
			//CC#7 �{�����[��
			ccValue = m_CacheCC007_Volume[portNo][chNo];
			if (ccValue < 0x80) {
				result = _SendMIDIEventCC(portNo, chNo, 7, ccValue);
				if (result != 0) goto EXIT;
			}
			//CC#10 �p���|�b�g
			ccValue = m_CacheCC010_Panpot[portNo][chNo];
			if (ccValue < 0x80) {
				result = _SendMIDIEventCC(portNo, chNo, 10, ccValue);
				if (result != 0) goto EXIT;
			}
			//CC#11 �G�N�X�v���b�V����
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
// MIDI�C�x���g�L���b�V�����M�F�s�b�`�x���h
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

	//MIDI�C�x���g�f�[�^�쐬
	event.SetMIDIData(0xE0 | chNo, pPtichBend, 2);
	eventMIDI.Attach(&event);

	//MIDI�C�x���g���M
	result = _SendMIDIEvent(portNo, &eventMIDI);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// MIDI�C�x���g�L���b�V�����M�F�R���g���[���`�F���W
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

	//MIDI�C�x���g�f�[�^�쐬
	data[0] = ccNo;
	data[1] = ccValue;
	event.SetMIDIData(0xB0 | chNo, data, 2);
	eventMIDI.Attach(&event);

	//MIDI�C�x���g���M
	result = _SendMIDIEvent(portNo, &eventMIDI);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �X�L�b�v����
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

	//����X�L�b�v�̏ꍇ
	if (targetTimeInNanoSec < m_CurPlayTime) {
		//�Đ��J�n�p�����[�^������
		result = _InitializeParamsOnPlayStart();
		if (result != 0) goto EXIT;
		
		m_MsgTrans.PostSkipStart(SM_SKIP_BACK);
	}
	//�O���X�L�b�v�̏ꍇ
	else {
		m_MsgTrans.PostSkipStart(SM_SKIP_FORWARD);
	}

	//MIDI�C�x���g�L���b�V���N���A
	_ClearMIDIEventCache();

	//�w�莞���܂�MIDI�C�x���g����������
	m_isSkipping = true;
	while (*pIsContinue) {
		//�X���b�h�C���^�[�o������
		result = _IntervalProc(pIsContinue);
		if (result != 0) goto EXIT;
		
		//�w�莞���ɒB������X�L�b�v�I���Ƃ���
		if (targetTimeInNanoSec <= m_CurPlayTime) break;
	}
	m_isSkipping = false;

	//�L���b�V�����M
	result = _SendMIDIEventCache();
	if (result != 0) goto EXIT;

	//�Đ������ړ�
	endTickTime = m_TotalTickTimeTemp;
	_SlidePlaybackTime(startPlayTime, startTickTime, endTickTime);

	//�X�L�b�v�ړ���̏�Ԃ�ʒm
	m_MsgTrans.PostPlayTime((unsigned long)(m_CurPlayTime/1000000), endTickTime);
	m_MsgTrans.PostTempo(m_Tempo);
	m_MsgTrans.PostBeat((unsigned short)m_BeatNumerator, (unsigned short)m_BeatDenominator);
	m_MsgTrans.PostBar(m_CurBarNo);

	//�Đ��J�n�������X�V
	m_PrevTimerTime = _GetCurTimeInNano();

	//�X�L�b�v�I��
	m_MsgTrans.PostSkipEnd(m_NotesCount);

	//�O���X�L�b�v�ɂ��Đ��I��
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
// �Đ������ړ�
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
	unsigned long waitTimeInMsec = 10;  //10msec.���Ƃɒʒm
	unsigned long stepNum = 0;
	bool isRewind = false;

	//�Đ������ʒm��
	stepNum = m_MovingTimeSpanInMsec / waitTimeInMsec;

	//�`�b�N�^�C�����ݒl
	if (startTickTime > endTickTime) {
		isRewind = true;
		tickTimeStep = (startTickTime - endTickTime) / stepNum;
	}
	else {
		isRewind = false;
		tickTimeStep = (endTickTime - startTickTime) / stepNum;
	}

	//�Đ������ړ�
	tickTime = startTickTime;
	for (i = 0; i < stepNum; i ++) {
		//�Đ�������ʒm�F�`�b�N�^�C���̂ݍX�V
		if (isRewind) {
			tickTime -= tickTimeStep;
		}
		else {
			tickTime += tickTimeStep;
		}
		m_MsgTrans.PostPlayTime((unsigned long)(startPlayTime/1000000), tickTime);
		
		//�ҋ@
		Sleep(waitTimeInMsec);
	}

	return;
}

//******************************************************************************
// �^�C�}�[�Ăяo��
//******************************************************************************
int SMSequencer::_OnTimer()
{
	int result = 0;
	BOOL isContinue = true;

	unsigned long deltaTime = 0;

	//���������_���Z���x��{���x�ɐݒ�
	//  �^�C�}�[�J�n�����1�񂾂����s����
	if (!(m_FPUCtrl.IsLocked())) {
		result = m_FPUCtrl.Start(SMFPUCtrl::FPUDouble);
		if (result != 0) goto EXIT;
	}

	//�X���b�h�C���^�[�o������
	result = _IntervalProc(&isContinue);
	if (result != 0) goto EXIT;

	//���[�U���N�G�X�g�̏���
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
	return result;
}

//******************************************************************************
// �^�C�}�[�R�[���o�b�N�֐�
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

