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

	//�g��\�����Ԏ擾
	unsigned long GetRippleDecayDuration();
	unsigned long GetRippleReleaseDuration();

	//�g��T�C�Y�擾
	float GetRippleHeight(float rate);
	float GetRippleWidth(float rate);
	float GetRippleAlpha(float rate);
	float GetDecayCoefficient(float rate);

	//�|�[�g���_���W�擾
	virtual float GetPortOriginZ(unsigned char portNo);

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

	int m_RippleDecayDuration;
	int m_RippleReleaseDuration;

};


