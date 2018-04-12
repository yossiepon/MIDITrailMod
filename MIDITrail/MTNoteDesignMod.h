//******************************************************************************
//
// MIDITrail / MTNoteDesignMod
//
// �m�[�g�f�U�C��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteDesign.h"
//******************************************************************************
// �m�[�g�f�U�C��Mod�N���X
//******************************************************************************
class MTNoteDesignMod : public MTNoteDesign
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteDesignMod(void);
	virtual ~MTNoteDesignMod(void);

	//������
	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	//�g��\�����Ԏ擾
	unsigned long GetRippleDecayDuration();
	unsigned long GetRippleReleaseDuration();

	//�g��`����擾
	D3DBLEND GetRippleSrcBlend();
	D3DBLEND GetRippleDestBlend();
	unsigned long GetRippleOverwriteTimes();
	float GetRippleSpacing();

	//�g��T�C�Y�擾
	float GetRippleHeight(float rate);
	float GetRippleWidth(float rate);
	float GetRippleAlpha(float rate);
	float GetDecayCoefficient(
				float rate,					//�T�C�Y�䗦
				float saturation = 20.0f	//�O�a���x��
			);

	//�������m�[�g�{�b�N�X���_���W�擾
	void GetActiveNoteBoxVirtexPos(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
				D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector3,	//YZ����+X�����������ĉE��
				short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
				unsigned char pitchBendSensitivity = 0,	//�ȗ��F�s�b�`�x���h���x
				float rate = 0.0f						//�ȗ��F�T�C�Y�䗦
			);

	//�������m�[�g�{�b�N�X�J���[�擾
	D3DXCOLOR GetActiveNoteBoxColor(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				float rate
			);

protected:

	virtual void _Clear();
	virtual int _LoadConfFile(const TCHAR* pSceneName);

private:

	//�g��f�B�P�C����
	int m_RippleDecayDuration;
	//�g�䃊���[�X����
	int m_RippleReleaseDuration;

	//�g��`�挳�i���b�v���摜�j�u�����h�w�� 
	D3DBLEND m_RippleSrcBlend;
	//�g��`���i�w�i�摜�j�u�����h�w��
	D3DBLEND m_RippleDestBlend;

	//�g��㏑����
	int m_RippleOverwriteTimes;
	//�g��`��Ԋu
	float m_RippleSpacing;
};


