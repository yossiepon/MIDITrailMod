//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesignMod
//
// �s�A�m�L�[�{�[�h�f�U�C��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardDesign.h"


//******************************************************************************
// �s�A�m�L�[�{�[�h�f�U�C��Mod�N���X
//******************************************************************************
class MTPianoKeyboardDesignMod : public MTPianoKeyboardDesign
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTPianoKeyboardDesignMod(void);
	virtual ~MTPianoKeyboardDesignMod(void);

	//������
	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	//�|�[�g���_���W�擾
	virtual float GetPortOriginY(unsigned char portNo);
	virtual float GetPortOriginZ(unsigned char portNo);

	//�`�����l���Ԋu�擾
	float GetChStep();

	//�������L�[�J���[�擾
	D3DXCOLOR GetActiveKeyColor(unsigned char chNo, unsigned char noteNo, unsigned long elapsedTime);

	//�L�[�{�[�h����W�擾
	virtual D3DXVECTOR3 GetKeyboardBasePos(unsigned char portNo, unsigned char chNo, float scale);

protected:

	virtual void _Initialize();
	virtual int _LoadConfFile(const TCHAR* pSceneName);

private:

	//�`�����l���Ԋu
	float m_ChStep;

	//�������L�[�F���
	D3DXCOLOR m_ActiveKeyColorList[16];

};


