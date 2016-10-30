#//******************************************************************************
//
// Simple MIDI Library / SMMsgParser
//
// ���b�Z�[�W��̓N���X
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

namespace SMIDILib {

//******************************************************************************
// ���b�Z�[�W��̓N���X
//******************************************************************************
class SMIDILIB_API SMMsgParser
{
public:

	//�V�[�P���T���b�Z�[�W���
	enum Message {
		MsgUnknown,		//���b�Z�[�W�s��
		MsgPlayStatus,	//���t��Ԓʒm
		MsgPlayTime,	//���t���Ԓʒm
		MsgTempo,		//�e���|�ύX�ʒm
		MsgBar,			//���ߔԍ��ʒm
		MsgBeat,		//���q�L���ύX�ʒm
		MsgNoteOff,		//�m�[�gOFF�ʒm
		MsgNoteOn,		//�m�[�gON�ʒm
		MsgPitchBend,	//�s�b�`�x���h�ʒm
		MsgSkipStart,	//�X�L�b�v�J�n�ʒm
		MsgSkipEnd,		//�X�L�b�v�I���ʒm
		MsgAllNoteOff	//�I�[���m�[�gOFF�ʒm
	};

	//���t���
	enum PlayStatus {
		StatusUnknown,	//���b�Z�[�W�s��
		StatusStop,		//��~
		StatusPlay,		//���t
		StatusPause		//�ꎞ��~
	};

	//�X�L�b�v����
	enum SkipDirection {
		SkipBack,
		SkipForward
	};

public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMMsgParser(void);
	virtual ~SMMsgParser(void);

	//���b�Z�[�W���
	void Parse(unsigned long wParam, unsigned long lParam);

	//���b�Z�[�W��ʎ擾
	Message GetMsg();

	//���t��Ԏ擾
	PlayStatus GetPlayStatus();

	//���t���Ԏ擾
	unsigned long GetPlayTimeSec();
	unsigned long GetPlayTimeMSec();
	unsigned long GetPlayTickTime();

	//�e���|�擾
	unsigned long GetTempoBPM();

	//���ߔԍ��擾
	unsigned long GetBarNo();

	//���q�L���擾
	unsigned long GetBeatNumerator();
	unsigned long GetBeatDenominator();

	//�m�[�gON/OFF���擾
	unsigned char GetPortNo();
	unsigned char GetChNo();
	unsigned char GetNoteNo();
	unsigned char GetVelocity();

	//�s�b�`�x���h���擾
	short GetPitchBendValue();
	unsigned char GetPitchBendSensitivity();

	//�X�L�b�v�J�n���擾
	SkipDirection GetSkipStartDirection();

	//�X�L�b�v�I�����擾
	unsigned long GetSkipEndNotesCount();

private:

	unsigned long m_WParam;
	unsigned long m_LParam;
	Message m_Msg;

};

} // end of namespace

