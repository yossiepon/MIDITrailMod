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
	int Transform(
			LPDIRECT3DDEVICE9 pD3DDevice,
			D3DXVECTOR3 basePosVector,
			D3DXVECTOR3 playbackPosVector,
			float rollAngle
		);

	//�L�[��ԕύX
	virtual int PushKey(
			unsigned char chNo,
			unsigned char noteNo,
			float keyDownRate,
			unsigned long elapsedTime,
			D3DXCOLOR* pActiveKeyColor = NULL
		);

private:

	virtual int _CreateVertexOfKeyWhite1(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyWhite2(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyWhite3(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyBlack(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);

private:

	//�L�[�{�[�h�f�U�C��
	MTPianoKeyboardDesignMod m_KeyboardDesignMod;

};


