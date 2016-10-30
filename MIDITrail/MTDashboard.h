//******************************************************************************
//
// MIDITrail / MTDashboard
//
// �_�b�V���{�[�h�`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �Ȗ��^���t���ԁ^�e���|�^�r�[�g�^���ߔԍ� ��\������B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "MTStaticCaption.h"
#include "MTDynamicCaption.h"

using namespace SMIDILib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�t�H���g�ݒ�
#define MTDASHBOARD_FONTNAME  _T("MS Gothic")
#define MTDASHBOARD_FONTSIZE  (40)

//�J�E���^�L���v�V����������
#define MTDASHBOARD_COUNTER_CHARS  _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:/% ")

//�J�E���^�L���v�V�����T�C�Y
//   12345678901234567890123456789012345678901234567890123456789012345678901234  (74)
//  "TIME:00:00/00:00 BPM:000 BEAT:4/4 BAR:000/000 NOTES:00000/00000 SPEED:000%"
//  �]�T���݂�80�ɂ��Ă���
#define MTDASHBOARD_COUNTER_SIZE  (80)

//�g�T�C�Y�i�s�N�Z���j
#define MTDASHBOARD_FRAMESIZE  (5.0f)

//�f�t�H���g�\���g�嗦
#define MTDASHBOARD_DEFAULT_MAGRATE  (0.5f)


//******************************************************************************
// �_�b�V���{�[�h�`��N���X
//******************************************************************************
class MTDashboard
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTDashboard(void);
	virtual ~MTDashboard(void);

	//����
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData, HWND hWnd);

	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���
	void Release();

	//���t�o�ߎ��ԂƑ����t���Ԃ̓o�^
	void SetPlayTimeSec(unsigned long playTimeSec);
	void SetTotalPlayTimeSec(unsigned long totalPlayTimeSec);

	//�e���|�o�^
	void SetTempoBPM(unsigned long bpm);

	//���ߔԍ��ƑS���ߐ��̓o�^
	void SetBarNo(unsigned long barNo);
	void SetBarNum(unsigned long barNum);

	//���q�L���o�^
	void SetBeat(unsigned long numerator, unsigned long denominator);

	//�m�[�gON�o�^
	void SetNoteOn();

	//���t���x�o�^
	void SetPlaySpeedRatio(unsigned long ratio);

	//���Z�b�g
	void Reset();

	//�m�[�g���o�^
	void SetNotesCount(unsigned long notesCount);

	//���t���Ԏ擾
	unsigned long GetPlayTimeSec();

	//�\���ݒ�
	void SetEnable(bool isEnable);

private:

	HWND m_hWnd;
	
	MTStaticCaption m_Title;
	
	MTDynamicCaption m_Counter;
	float m_PosCounterX;
	float m_PosCounterY;
	float m_CounterMag;

	unsigned long m_PlayTimeSec;
	unsigned long m_TotalPlayTimeSec;
	unsigned long m_TempoBPM;
	unsigned long m_BeatNumerator;
	unsigned long m_BeatDenominator;
	unsigned long m_BarNo;
	unsigned long m_BarNum;
	unsigned long m_NoteCount;
	unsigned long m_NoteNum;
	unsigned long m_PlaySpeedRatio;

	unsigned long m_TempoBPMOnStart;
	unsigned long m_BeatNumeratorOnStart;
	unsigned long m_BeatDenominatorOnStart;

	D3DXCOLOR m_CaptionColor;

	//�\����
	bool m_isEnable;

	int _GetCounterPos(float* pX, float* pY);
	int _GetCounterStr(TCHAR* pStr, unsigned long bufSize);
	int _LoadConfFile(const TCHAR* pSceneName);

};

