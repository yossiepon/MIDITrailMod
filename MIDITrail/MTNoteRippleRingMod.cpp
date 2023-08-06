//******************************************************************************
//
// MIDITrail / MTNoteRippleRingMod
//
// �m�[�g�g�䃊���O�`��Mod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteDesignRingMod.h"
#include "MTNoteRippleRingMod.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteRippleRingMod::MTNoteRippleRingMod(void)
{
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteRippleRingMod::~MTNoteRippleRingMod(void)
{
}

//******************************************************************************
// �m�[�g�f�U�C������
//******************************************************************************
int MTNoteRippleRingMod::_CreateNoteDesign()
{
	int result = 0;

	try {
		//�m�[�g�f�U�C��Mod�I�u�W�F�N�g����
		m_pNoteDesignMod = new MTNoteDesignRingMod();
		m_pNoteDesign = m_pNoteDesignMod;
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}


