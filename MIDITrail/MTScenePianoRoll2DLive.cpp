//******************************************************************************
//
// MIDITrail / MTScenePianoRoll2DLive
//
// ���C�u���j�^�p�s�A�m���[��2D�V�[���`��N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRoll2DLive.h"


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTScenePianoRoll2DLive::MTScenePianoRoll2DLive(void)
{
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTScenePianoRoll2DLive::~MTScenePianoRoll2DLive(void)
{
}

//******************************************************************************
// ���̎擾
//******************************************************************************
const TCHAR* MTScenePianoRoll2DLive::GetName()
{
	return _T("PianoRoll2D");
}

//******************************************************************************
// �V�[������
//******************************************************************************
int MTScenePianoRoll2DLive::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	
	//�s�A�m���[��2D�̓��C�g�Ȃ�
	//  �m�[�g�{�b�N�X�̕����[���ɂ���̂ŕ\�Ɨ������ꕽ�ʏ�ŏd�Ȃ�
	//  ���C�g��L���ɂ���ƕ\�Ɨ��̐F���قȂ�Z�t�@�C�e�B���O��U������
	m_IsEnableLight = false;
	
	result = MTScenePianoRoll3DLive::Create(hWnd, pD3DDevice, pSeqData);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

