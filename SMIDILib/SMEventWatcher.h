//******************************************************************************
//
// Simple MIDI Library / SMEventWatcher
//
// �C�x���g�E�H�b�`���[�N���X
//
// Copyright (C) 2012-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"
#include "SMEventMIDI.h"
#include "SMEventSysMsg.h"
#include "SMMsgTransmitter.h"
#include "SMCommon.h"


namespace SMIDILib {

//******************************************************************************
// �C�x���g�E�H�b�`���[�N���X
//******************************************************************************
class SMIDILIB_API SMEventWatcher
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	SMEventWatcher(void);
	virtual ~SMEventWatcher(void);
	
	//������
	int Initialize(SMMsgTransmitter* pMsgTrans);
	
	//�C�x���g�Ď�
	int WatchEvent(unsigned char portNo, SMEvent* pEvent);
	
	//�C�x���g�Ď��F�V�[�P���T����
	int WatchEventMIDI(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int WatchEventControlChange(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	
private:
	
	//RPN/NRPN�I�����
	enum RPN_NRPN_Select {
		RPN_NULL,
		RPN,
		NRPN
	};
	
	//RPN���
	enum RPN_Type {
		RPN_None,
		PitchBendSensitivity,
		MasterFineTune,
		MasterCourseTune
	};
	
	//���b�Z�[�W���M����
	SMMsgTransmitter* m_pMsgTrans;
	
	//�s�b�`�x���h����
	unsigned char m_PitchBendSensitivity[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	
	//RPN����n
	RPN_NRPN_Select m_RPN_NRPN_Select[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_RPN_MSB[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_RPN_LSB[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	
	void _ClearChInfo();
	int _WatchEventMIDI(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int _WatchEventControlChange(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int _WatchEventControlChange2(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	RPN_Type _GetCurRPNType(unsigned char portNo, unsigned char chNo);
	int _WatchEventSysMsg(unsigned char portNo, SMEventSysMsg* pEventSysMsg);
	
};

} // end of namespace


