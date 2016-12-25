//******************************************************************************
//
// MIDITrail / MTNoteDesignMod
//
// �m�[�g�f�U�C��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTConfFile.h"
#include "MTNoteDesignMod.h"


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteDesignMod::MTNoteDesignMod(void)
{
	// ���N���X��_Clear()���Ă΂�邽�߁A���N���X�̃R���X�g���N�^�͌Ă΂Ȃ�
	_Clear();
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteDesignMod::~MTNoteDesignMod(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int MTNoteDesignMod::Initialize(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	OutputDebugString(_T("MTNoteDesignMod::Initialize\n"));

	//���N���X�̏������������Ăяo��
	MTNoteDesign::Initialize(pSceneName, pSeqData);

	//�p�����[�^�ݒ�t�@�C���ǂݍ���
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �g��f�B�P�C���Ԏ擾(msec)
//******************************************************************************
unsigned long MTNoteDesignMod::GetRippleDecayDuration()
{
	return (unsigned long)m_RippleDecayDuration;
}

//******************************************************************************
// �g�䃊���[�X���Ԏ擾(msec)
//******************************************************************************
unsigned long MTNoteDesignMod::GetRippleReleaseDuration()
{
	return (unsigned long)m_RippleReleaseDuration;
}

//******************************************************************************
// �g��c�T�C�Y�擾
//******************************************************************************
float MTNoteDesignMod::GetRippleHeight(
		float rate	//�T�C�Y�䗦
	)
{
	return m_RippleHeight * GetDecayCoefficient(rate);
}

//******************************************************************************
// �g�䉡�T�C�Y�擾
//******************************************************************************
float MTNoteDesignMod::GetRippleWidth(
		float rate	//�T�C�Y�䗦
	)
{
	return m_RippleWidth * GetDecayCoefficient(rate);
}

//******************************************************************************
// �g�䓧���x�擾
//******************************************************************************
float MTNoteDesignMod::GetRippleAlpha(
		float rate	//�T�C�Y�䗦
	)
{
	return GetDecayCoefficient(rate);
}

//******************************************************************************
// �����W���擾
//******************************************************************************
float MTNoteDesignMod::GetDecayCoefficient(
		float rate	//�T�C�Y�䗦
	)
{
	float coeff = 1.0f;

	if(rate < 0.5f) {
		coeff = (pow(2.0f, (0.5f - rate) * 8.0f) + 14.0f) / 20.0f;
	} else {
		coeff = (16.0f - pow(2.0f, (rate - 0.5f) * 8.0f)) / 20.0f;
	}

	coeff = coeff > 1.0f ? 1.0f : coeff;

	return coeff;
}

//******************************************************************************
// �|�[�g���_Z���W�擾
//******************************************************************************
float MTNoteDesignMod::GetPortOriginZ(
		unsigned char portNo
	)
{
	float portIndex = 0.0f;
	float portWidth = 0.0f;

	//                  +y
	//                   |
	//         portC   portB   portA
	//       +-------+-------+-------+Note#127
	//       |       |   |   |       |
	//       |       |   |   |       |
	//       |       |   |   |       |
	// +z<---|-------@---0---@-------@--------->-z
	//       |       |   |   |       |
	//       |       |   |   |       |  @:OriginZ(for portA,B,C)
	//       |       |   |   |       |
	//       +-------+-------+-------+Note#0
	//    Ch. 16    0 16 |  0 16    0
	//                   |
	//                  -y

	portIndex = (float)(m_PortIndex[portNo]);
	portWidth = GetChStep() * 15.0f;

	return ((portWidth * portIndex) - (portWidth * m_PortList.GetSize() / 2.0f));
}

//******************************************************************************
// �������m�[�g�{�b�N�X�J���[�擾
//******************************************************************************
D3DXCOLOR MTNoteDesignMod::GetActiveNoteBoxColor(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		float rate
	)
{
	float alpha = 1.0f;

	if(rate < 0.5f) {
		alpha = (pow(2.0f, (0.5f - rate) * 8.0f) + 14.0f) / 20.0f;
	} else {
		alpha = (16.0f - pow(2.0f, (rate - 0.5f) * 8.0f)) / 20.0f;
	}

	alpha = alpha > 1.0f ? 1.0f : alpha;

	D3DXCOLOR color;
	float r,g,b,a = 0.0f;

	color = GetNoteBoxColor(portNo, chNo, noteNo);

	//m_ActiveNoteDuration �����[�X�^�C��
	//  �����J�n���_����m�[�g�F�����ɖ߂��܂ł̎���
	//  ������ m_ActiveNoteEmissive �ɂ���ă����[�X����m�[�gOFF�܂Ŕ�������

	//m_ActiveNoteWhiteRate �ő唒�F��
	//  0.0 �� �m�[�g�F�ω��Ȃ�
	//  0.5 �� �m�[�g�F�Ɣ��̒��ԐF
	//  1.0 �� ��

	r = color.r + ((1.0f - color.r) * alpha * m_ActiveNoteWhiteRate);
	g = color.g + ((1.0f - color.g) * alpha * m_ActiveNoteWhiteRate);
	b = color.b + ((1.0f - color.b) * alpha * m_ActiveNoteWhiteRate);
	a = color.a;
	color = D3DXCOLOR(r, g, b, a);

	return color;
}

//******************************************************************************
// �N���A
//******************************************************************************
void MTNoteDesignMod::_Clear(void)
{
	MTNoteDesign::_Clear();

	m_RippleDecayDuration = 100;
	m_RippleReleaseDuration = 250;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTNoteDesignMod::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	MTConfFile confFile;

	//���N���X�̓ǂݍ��ݏ������Ăяo��
	result = MTNoteDesign::_LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//�ݒ�t�@�C�����J��
	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//----------------------------------
	//�g����
	//----------------------------------
	result = confFile.SetCurSection(_T("Ripple"));
	if (result != 0) goto EXIT;

	//�g��f�B�P�C����(msec)
	result = confFile.GetInt(_T("DecayDuration"), &m_RippleDecayDuration, 100);
	if (result != 0) goto EXIT;

	//�g�䃊���[�X����(msec)
	result = confFile.GetInt(_T("ReleaseDuration"), &m_RippleReleaseDuration, 250);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

