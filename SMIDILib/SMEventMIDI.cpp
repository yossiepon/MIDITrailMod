//******************************************************************************
//
// Simple MIDI Library / SMEventMIDI
//
// MIDI�C�x���g�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventMIDI.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMEventMIDI::SMEventMIDI()
{
	m_pEvent = NULL;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMEventMIDI::~SMEventMIDI(void)
{
}

//******************************************************************************
// �C�x���g�R�t��
//******************************************************************************
void SMEventMIDI::Attach(
		SMEvent* pEvent
	)
{
	m_pEvent = pEvent;
}

//******************************************************************************
// MIDI�o�̓��b�Z�[�W�擾�i�V���[�g�j
//******************************************************************************
int SMEventMIDI::GetMIDIOutShortMsg(
		unsigned long* pMsg
	)
{
	int result = 0;
	unsigned char status = 0;
	unsigned char* pData = NULL;
	unsigned char data1 = 0;
	unsigned char data2 = 0;

	if (m_pEvent == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0 );
		goto EXIT;
	}

	status = m_pEvent->GetStatus();
	pData = m_pEvent->GetDataPtr();

	if (m_pEvent->GetDataSize() == 2) {
		data1 = pData[0];
		data2 = pData[1];
	}
	else if (m_pEvent->GetDataSize() == 1) {
		data1 = pData[0];
		data2 = 0;
	}
	else {
		result = YN_SET_ERR("Program error.", m_pEvent->GetDataSize(), 0);
		goto EXIT;
	}

	*pMsg = (unsigned long)((data2 << 16) | (data1 << 8) | (status));

EXIT:;
	return result;
}

//******************************************************************************
// �`�����l�����b�Z�[�W��ʎ擾
//******************************************************************************
SMEventMIDI::ChMsg SMEventMIDI::GetChMsg()
{
	ChMsg msg = None;
	unsigned char* pData = NULL;
	unsigned char size = 0;
	unsigned char velocity = 0;

	if (m_pEvent == NULL) {
		goto EXIT;
	}

	switch (m_pEvent->GetStatus() & 0xF0) {
		case 0x80: msg = NoteOff;          size = 3; break;
		case 0x90: msg = NoteOn;           size = 3; break;
		case 0xA0: msg = PolyphonicKeyPressure; size = 3; break;
		case 0xB0: msg = ControlChange;    size = 3; break;
		case 0xC0: msg = ProgramChange;    size = 2; break;
		case 0xD0: msg = ChannelPressure;  size = 2; break;
		case 0xE0: msg = PitchBend;        size = 3; break;
		default: break;
	}
	if ((m_pEvent->GetDataSize() + 1) != size) {
		msg = None;
	}

	//�m�[�g�I���ł����Ă��x���V�e�B���[���Ȃ�m�[�g�I�t�Ƃ���
	if (msg == NoteOn) {
		pData = m_pEvent->GetDataPtr();
		velocity = pData[1];
		if (velocity == 0) {
			msg = NoteOff;
		}
	}

EXIT:;
	return msg;
}

//******************************************************************************
// �`�����l���ԍ��擾
//******************************************************************************
unsigned char SMEventMIDI::GetChNo()
{
	unsigned char chNo = 0;

	if (m_pEvent == NULL) {
		goto EXIT;
	}

	chNo = m_pEvent->GetStatus() & 0x0F;

EXIT:;
	return chNo;
}

//******************************************************************************
// �m�[�g�ԍ��擾
//******************************************************************************
unsigned char SMEventMIDI::GetNoteNo()
{
	ChMsg msg = None;
	unsigned char* pData = NULL;
	unsigned char noteNo = 0;

	msg = GetChMsg();
	if ((msg != NoteOn) && (msg != NoteOff) && (msg != PolyphonicKeyPressure)) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();
	noteNo = pData[0];

EXIT:;
	return noteNo;
}

//******************************************************************************
// �x���V�e�B�擾
//******************************************************************************
unsigned char SMEventMIDI::GetVelocity()
{
	ChMsg msg = None;
	unsigned char* pData = NULL;
	unsigned char velocity = 0;

	msg = GetChMsg();
	if ((msg != NoteOn) && (msg != NoteOff) && (msg != PolyphonicKeyPressure)) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();
	velocity = pData[1];

EXIT:;
	return velocity;
}

//******************************************************************************
// �R���g���[���`�F���W�ԍ��擾
//******************************************************************************
unsigned char SMEventMIDI::GetCCNo()
{
	unsigned char* pData = NULL;
	unsigned char ccNo = 0;

	if (GetChMsg() != ControlChange) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();
	ccNo = pData[0];

EXIT:;
	return ccNo;
}

//******************************************************************************
// �R���g���[���`�F���W�l�擾
//******************************************************************************
unsigned char SMEventMIDI::GetCCValue()
{
	unsigned char* pData = NULL;
	unsigned char value = 0;

	if (GetChMsg() != ControlChange) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();
	value = pData[1];

EXIT:;
	return value;
}

//******************************************************************************
// �v���O�����擾�i���F�j
//******************************************************************************
unsigned char SMEventMIDI::GetProgramNo()
{
	unsigned char* pData = NULL;
	unsigned char prgNo = 0;

	if (GetChMsg() != ProgramChange) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();
	prgNo = pData[0];

EXIT:;
	return prgNo;
}

//******************************************************************************
// �v���b�V���[�l�擾
//******************************************************************************
unsigned char SMEventMIDI::GetPressureValue()
{
	unsigned char* pData = NULL;
	unsigned char value = 0;

	if (GetChMsg() != ChannelPressure) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();
	value = pData[0];

EXIT:;
	return value;
}

//******************************************************************************
// �s�b�`�x���h�擾
//******************************************************************************
short SMEventMIDI::GetPitchBendValue()
{
	unsigned char* pData = NULL;
	unsigned char dl = 0;
	unsigned char dm = 0;
	short value = 0;

	if (GetChMsg() != PitchBend) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();
	dl = pData[0]; // 0x00-7F
	dm = pData[1]; // 0x00-7F
	value = (((short)dm << 7) + dl) - 8192;

EXIT:;
	return value;
}

} // end of namespace

