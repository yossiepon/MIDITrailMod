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
// �g��㏑����
//******************************************************************************
unsigned long MTNoteDesignMod::GetRippleOverwriteTimes()
{
	return (unsigned long)m_RippleOverwriteTimes;
}

//******************************************************************************
// �g��`��Ԋu
//******************************************************************************
float MTNoteDesignMod::GetRippleSpacing()
{
	return m_RippleSpacing;
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
		float rate,			//�T�C�Y�䗦
		float saturation	//�O�a���x��
	)
{
	float coeff = 1.0f;

	if(rate < 0.5f) {
		coeff = (pow(2.0f, (0.5f - rate) * 8.0f) + 14.0f) / saturation;
	} else {
		coeff = (16.0f - pow(2.0f, (rate - 0.5f) * 8.0f)) / saturation;
	}

	coeff = coeff > 1.0f ? 1.0f : coeff;

	return coeff;
}

//******************************************************************************
// �������m�[�g�{�b�N�X���_���W�擾
//******************************************************************************
void MTNoteDesignMod::GetActiveNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
		D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector3,	//YZ����+X�����������ĉE��
		short pitchBendValue,				//�ȗ��F�s�b�`�x���h
		unsigned char pitchBendSensitivity,	//�ȗ��F�s�b�`�x���h���x
		float rate							//�ȗ��F�T�C�Y�䗦
	)
{
	D3DXVECTOR3 center;
	float bh, bw = 0.0f;
	float curSizeRatio = 1.0f;
	
	center = GetNoteBoxCenterPosX(curTickTime, portNo, chNo, noteNo, pitchBendValue, pitchBendSensitivity);
	
	if (rate > 0.0f) {
		curSizeRatio = 1.0f + (m_ActiveNoteBoxSizeRatio - 1.0f) * GetDecayCoefficient(rate, 30.0f);
	}
	
	bh = GetNoteBoxHeight() * curSizeRatio;
	bw = GetNoteBoxWidht() * curSizeRatio;
	
	*pVector0 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z+(bw/2.0f));
	*pVector1 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z-(bw/2.0f));
	*pVector2 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z+(bw/2.0f));
	*pVector3 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z-(bw/2.0f));
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
	float alpha = GetDecayCoefficient(rate, 30.0f);

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

	//�g��㏑����
	result = confFile.GetInt(_T("OverwriteTimes"), &m_RippleOverwriteTimes, 3);
	if (result != 0) goto EXIT;

	//�g��`��Ԋu
	result = confFile.GetFloat(_T("Spacing"), &m_RippleSpacing, 0.002f);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

