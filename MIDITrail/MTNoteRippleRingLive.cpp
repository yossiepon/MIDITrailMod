//******************************************************************************
//
// MIDITrail / MTNoteRippleRingLive
//
// ���C�u���j�^�p�m�[�g�g�䃊���O�`��N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteDesignRing.h"
#include "MTNoteRippleRingLive.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteRippleRingLive::MTNoteRippleRingLive(void)
{
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteRippleRingLive::~MTNoteRippleRingLive(void)
{
}

//******************************************************************************
// �m�[�g�f�U�C������
//******************************************************************************
int MTNoteRippleRingLive::_CreateNoteDesign()
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


