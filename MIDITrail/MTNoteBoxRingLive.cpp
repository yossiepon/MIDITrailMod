//******************************************************************************
//
// MIDITrail / MTNoteBoxRingLive
//
// ���C�u���j�^�p�m�[�g�{�b�N�X�����O�`��N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteDesignRing.h"
#include "MTNoteBoxRingLive.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteBoxRingLive::MTNoteBoxRingLive(void)
{
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteBoxRingLive::~MTNoteBoxRingLive(void)
{
}

//******************************************************************************
// �m�[�g�f�U�C������
//******************************************************************************
int MTNoteBoxRingLive::_CreateNoteDesign()
{
	int result = 0;

	try {
		m_pNoteDesign = new MTNoteDesignRing();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	
	//���C�u���j�^���[�h�ݒ�
	((MTNoteDesignRing*)m_pNoteDesign)->SetLiveMode();
	
EXIT:;
	return result;
}


