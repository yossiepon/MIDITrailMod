//******************************************************************************
//
// Simple MIDI Library / SMEventWatcher
//
// �C�x���g�E�H�b�`���[�N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventWatcher.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMEventWatcher::SMEventWatcher(void)
{
	m_pMsgTrans = NULL;
	_ClearChInfo();
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMEventWatcher::~SMEventWatcher(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int SMEventWatcher::Initialize(SMMsgTransmitter* pMsgTrans)
{
	int result = 0;
	
	m_pMsgTrans = pMsgTrans;
	
	_ClearChInfo();
	
	return result;
}

//******************************************************************************
// �C�x���g�E�H�b�`
//******************************************************************************
int SMEventWatcher::WatchEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;
	SMEventMIDI eventMIDI;
	
	if (pEvent->GetType() == SMEvent::EventMIDI) {
		eventMIDI.Attach(pEvent);
		
		//MIDI�C�x���g�Ď�
		result = _WatchEventMIDI(portNo, &eventMIDI);
		if (result != 0) goto EXIT;
		
		//�R���g���[���`�F���W�Ď�
		if (eventMIDI.GetChMsg() == SMEventMIDI::ControlChange) {
			result = _WatchEventControlChange(portNo, &eventMIDI);
			if (result != 0) goto EXIT;
			result = _WatchEventControlChange2(portNo, &eventMIDI);
			if (result != 0) goto EXIT;
		}
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI�C�x���g�E�H�b�`
//******************************************************************************
int SMEventWatcher::WatchEventMIDI(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	return _WatchEventMIDI(portNo, pMIDIEvent);
}

//******************************************************************************
// �R���g���[���`�F���W�C�x���g�E�H�b�`
//******************************************************************************
int SMEventWatcher::WatchEventControlChange(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	return _WatchEventControlChange(portNo, pMIDIEvent);
}

//******************************************************************************
// �`�����l�����N���A
//******************************************************************************
void SMEventWatcher::_ClearChInfo()
{
	unsigned long portNo = 0;
	unsigned long chNo = 0;
	
	for (portNo = 0; portNo < SM_MAX_PORT_NUM; portNo++) {
		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
			//RPN/NRPN�I�����
			m_RPN_NRPN_Select[portNo][chNo] = RPN_NULL;
			//RPN
			m_RPN_MSB[portNo][chNo] = 0x7F; //RPN NULL
			m_RPN_LSB[portNo][chNo] = 0x7F; //RPN NULL
			//�s�b�`�x���h���x
			m_PitchBendSensitivity[portNo][chNo] = SM_DEFAULT_PITCHBEND_SENSITIVITY;
		}
	}
	
	return;
}

//******************************************************************************
// MIDI�C�x���g�Ď�����
//******************************************************************************
int SMEventWatcher::_WatchEventMIDI(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	
	//�m�[�gOFF��ʒm
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOff) {
		m_pMsgTrans->PostNoteOff(
				portNo,
				pMIDIEvent->GetChNo(),
				pMIDIEvent->GetNoteNo()
			);
	}
	//�m�[�gON��ʒm
	else if (pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOn) {
		m_pMsgTrans->PostNoteOn(
				portNo,
				pMIDIEvent->GetChNo(),
				pMIDIEvent->GetNoteNo(),
				pMIDIEvent->GetVelocity()
			);
	}
	//�s�b�`�x���h��ʒm
	else if (pMIDIEvent->GetChMsg() == SMEventMIDI::PitchBend) {
		m_pMsgTrans->PostPitchBend(
				portNo,
				pMIDIEvent->GetChNo(),
				pMIDIEvent->GetPitchBendValue(),
				m_PitchBendSensitivity[portNo][pMIDIEvent->GetChNo()]
			);
	}
	
	return result;
}

//******************************************************************************
// �R���g���[���`�F���W�Ď�����
//******************************************************************************
int SMEventWatcher::_WatchEventControlChange(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	unsigned char msb = 0;
	unsigned char chNo = 0;
	
	chNo = pMIDIEvent->GetChNo();
	
	//----------------------------------------------------------------
	// NRPN MSB / LSB
	//----------------------------------------------------------------
	//NRPN MSB (CC#99)
	if (pMIDIEvent->GetCCNo() == 0x63) {
		m_RPN_NRPN_Select[portNo][chNo] = NRPN;
	}
	//NRPN LSB (CC#98)
	if (pMIDIEvent->GetCCNo() == 0x62) {
		m_RPN_NRPN_Select[portNo][chNo] = NRPN;
	}
	
	//----------------------------------------------------------------
	// RPN MSB / LSB
	//----------------------------------------------------------------
	//RPN MSB (CC#101)
	if (pMIDIEvent->GetCCNo() == 0x65) {
		m_RPN_NRPN_Select[portNo][chNo] = RPN;
		m_RPN_MSB[portNo][chNo] = pMIDIEvent->GetCCValue();
	}
	//RPN LSB (CC#100)
	if (pMIDIEvent->GetCCNo() == 0x64) {
		m_RPN_NRPN_Select[portNo][chNo] = RPN;
		m_RPN_LSB[portNo][chNo] = pMIDIEvent->GetCCValue();
		if ((m_RPN_MSB[portNo][chNo] == 0x7F)
			&& (m_RPN_LSB[portNo][chNo] == 0x7F)) {
			m_RPN_NRPN_Select[portNo][chNo] = RPN_NULL;
		}
	}
	
	//----------------------------------------------------------------
	// Data Entry MSB / LSB
	//----------------------------------------------------------------
	//Data Entry MSB (CC#6)
	if (pMIDIEvent->GetCCNo() == 0x06) {
		//�s�b�`�x���h���x MSB
		if (_GetCurRPNType(portNo, chNo) == PitchBendSensitivity) {
			m_PitchBendSensitivity[portNo][chNo] = pMIDIEvent->GetCCValue();
		}
	}
	//Data Entry LSB (CC#38)
	if (pMIDIEvent->GetCCNo() == 0x26) {
		//���ɐ���Ȃ�
	}
	
	//Data Increment (CC#96)
	if (pMIDIEvent->GetCCNo() == 0x60) {
		//�s�b�`�x���h���x MSB
		if (_GetCurRPNType(portNo, chNo) == PitchBendSensitivity) {
			msb = m_PitchBendSensitivity[portNo][chNo];
			if (msb < 24) {
				m_PitchBendSensitivity[portNo][chNo] = msb++;
			}
		}
	}
	//Data Decremnet (CC#97)
	if (pMIDIEvent->GetCCNo() == 0x61) {
		//�s�b�`�x���h���x MSB
		if (_GetCurRPNType(portNo, chNo) == PitchBendSensitivity) {
			msb = m_PitchBendSensitivity[portNo][chNo]++;
			if (msb > 0) {
				m_PitchBendSensitivity[portNo][chNo] = msb--;
			}
		}
	}
	
	//----------------------------------------------------------------
	// ���Z�b�g�I�[���R���g���[��
	//----------------------------------------------------------------
	//Reset All Controllers (CC#121)
	if (pMIDIEvent->GetCCNo() == 0x79) {
		//�s�b�`�x���h��ʒm�F0
		m_pMsgTrans->PostPitchBend(portNo, chNo, 0, m_PitchBendSensitivity[portNo][chNo]);
		//RPN/NRPN�I�����
		m_RPN_NRPN_Select[portNo][chNo] = RPN_NULL;
		//RPN
		m_RPN_MSB[portNo][chNo] = 0x7F; //RPN NULL
		m_RPN_LSB[portNo][chNo] = 0x7F; //RPN NULL
		
		//Roland SC�V���[�Y,Yamaha MU�V���[�Y�̏ꍇ
		//CC#121 ���Z�b�g�I�[���R���g���[���Ŏ��̒l���N���A�����
		//  An     �|���t�H�j�b�N�L�[�v���b�V���[  0
		//  Dn     �`�����l���v���b�V���[  0
		//  En     �s�b�`�x���h  0
		//  CC#1   ���W�����[�V����  0
		//  CC#11  �G�N�X�v���b�V����  127
		//  CC#64  �z�[���h1    0
		//  CC#65  �|���^�����g  0
		//  CC#66  �\�X�e�k�[�g  0
		//  CC#67  �\�t�g  0
		//  CC#98,99   NRPN  ���ݒ��ԁi�ݒ�ς݃f�[�^�͕ω����Ȃ��j
		//  CC#100,101 RPN   ���ݒ��ԁi�ݒ�ς݃f�[�^�͕ω����Ȃ��j
	}
	
	//EXIT:;
	return result;
}

//******************************************************************************
// RPN��ʎ擾
//******************************************************************************
SMEventWatcher::RPN_Type SMEventWatcher::_GetCurRPNType(
		unsigned char portNo,
		unsigned char chNo
	)
{
	RPN_Type type = RPN_None;
	
	if (m_RPN_NRPN_Select[portNo][chNo] == RPN) {
		if ((m_RPN_MSB[portNo][chNo] == 0x00)
			&& (m_RPN_LSB[portNo][chNo] == 0x00)) {
			type = PitchBendSensitivity;
		}
		if ((m_RPN_MSB[portNo][chNo] == 0x00)
			&& (m_RPN_LSB[portNo][chNo] == 0x01)) {
			type = MasterFineTune;
		}
		if ((m_RPN_MSB[portNo][chNo] == 0x00)
			&& (m_RPN_LSB[portNo][chNo] == 0x02)) {
			type = MasterCourseTune;
		}
	}
	
	return type;
}

//******************************************************************************
// �R���g���[���`�F���W�Ď�����2
//******************************************************************************
int SMEventWatcher::_WatchEventControlChange2(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	unsigned char chNo = 0;
	
	chNo = pMIDIEvent->GetChNo();
	
	//ALL SOUND OFF (CC#120)
	//ALL NOTE OFF (CC#123)
	if ((pMIDIEvent->GetCCNo() == 0x78) || (pMIDIEvent->GetCCNo() == 0x7B)) {
		m_pMsgTrans->PostAllNoteOff(portNo, chNo);
	}
	
	return result;
}

} // end of namespace


