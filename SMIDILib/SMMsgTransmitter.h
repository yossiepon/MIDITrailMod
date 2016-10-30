//******************************************************************************
//
// Simple MIDI Library / SMMsgTransmitter
//
// ���b�Z�[�W�]���N���X
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

#include "SMEvent.h"

namespace SMIDILib {


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//���b�Z�[�W���
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

//���t���
#define SM_PLAYSTATUS_STOP       (0x00)
#define SM_PLAYSTATUS_PLAY       (0x01)
#define SM_PLAYSTATUS_PAUSE      (0x02)

//�X�L�b�v����
#define SM_SKIP_BACK           (0x00)
#define SM_SKIP_FORWARD        (0x01)


//******************************************************************************
// ���b�Z�[�W�]���N���X
//******************************************************************************
class SMIDILIB_API SMMsgTransmitter
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMMsgTransmitter(void);
	virtual ~SMMsgTransmitter(void);

	//������
	int Initialize(HWND hTargetWnd, unsigned long msgId);

	//���t���
	int PostPlayStatus(unsigned long playStatus);

	//���t���Ԓʒm
	//  ������(playTimeSec)��3byte(0x00FFFFFF)�܂ł̐�������
	int PostPlayTime(unsigned long playTimeMSec, unsigned long tickTime);

	//�e���|�ʒm
	int PostTempo(unsigned long bpm);

	//���ߔԍ��ʒm�F1����J�n
	int PostBar(unsigned long barNo);

	//���q�L���ʒm
	//  ����͍ő�65535�܂œn���邪
	//  MIDI�̎d�l�ł͕��q255�^����2��255��܂ŕ\���ł���
	int PostBeat(unsigned short numerator, unsigned short denominator);

	//�m�[�gON�ʒm
	int PostNoteOn(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				unsigned char verocity
			);

	//�m�[�gOFF�ʒm
	int PostNoteOff(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo
			);

	//�s�b�`�x���h�ʒm
	int PostPitchBend(
				unsigned char portNo,
				unsigned char chNo,
				short pitchBendValue,
				unsigned char pitchBendSensitivity
			);

	//�X�L�b�v�J�n
	int PostSkipStart(unsigned long skipDirection);

	//�X�L�b�v�I��
	int PostSkipEnd(unsigned long notesCount);

	//�I�[���m�[�gOFF
	int PostAllNoteOff(
				unsigned char portNo,
				unsigned char chNo
			);

private:

	HWND m_hTargetWnd;
	unsigned long m_MsgId;

	int _Post(
			unsigned char msg,
			unsigned long param1, //3byte�܂�
			unsigned long param2  //4byte�܂�
		);

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMMsgTransmitter&);
	SMMsgTransmitter(const SMMsgTransmitter&);

};

} // end of namespace

