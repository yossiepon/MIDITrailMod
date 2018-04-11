//******************************************************************************
//
// MIDITrail / MTPianoKeyboardMod
//
// �s�A�m�L�[�{�[�h�`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboard.h"
#include "MTPianoKeyboardDesignMod.h"


//******************************************************************************
// �s�A�m�L�[�{�[�h�`��Mod�N���X
//******************************************************************************
class MTPianoKeyboardMod : public MTPianoKeyboard
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTPianoKeyboardMod(void);
	virtual ~MTPianoKeyboardMod(void);

	//����
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			LPDIRECT3DTEXTURE9 pTexture = NULL
		);

	//�X�V
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 moveVector1, D3DXVECTOR3 moveVector2, float scale, float z, float rollAngle);

	//�L�[��ԕύX
	virtual int PushKey(unsigned char chNo, unsigned char noteNo, float keyDownRate, unsigned long elapsedTime);

private:

	//�L�[�{�[�h�f�U�C��
	MTPianoKeyboardDesignMod m_KeyboardDesignMod;

};


