//******************************************************************************
//
// MIDITrail / MTScenePianoRoll2DMod
//
// �s�A�m���[��2D�V�[���`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRoll2DMod.h"


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTScenePianoRoll2DMod::MTScenePianoRoll2DMod(void)
{
	m_IsSingleKeyboard = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTScenePianoRoll2DMod::~MTScenePianoRoll2DMod(void)
{
}

//******************************************************************************
// ���̎擾
//******************************************************************************
const TCHAR* MTScenePianoRoll2DMod::GetName()
{
	return _T("PianoRoll2D");
}

//******************************************************************************
// �V�[������
//******************************************************************************
int MTScenePianoRoll2DMod::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	//�s�A�m���[��2D�̓��C�g�Ȃ�
	//  �m�[�g�{�b�N�X�̕����[���ɂ���̂ŕ\�Ɨ������ꕽ�ʏ�ŏd�Ȃ�
	//  ���C�g��L���ɂ���ƕ\�Ɨ��̐F���قȂ�Z�t�@�C�e�B���O��U������
	m_IsEnableLight = FALSE;

	result = MTScenePianoRoll3DMod::Create(hWnd, pD3DDevice, pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

