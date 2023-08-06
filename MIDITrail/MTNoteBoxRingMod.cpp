//******************************************************************************
//
// MIDITrail / MTNoteBoxRingMod
//
// �m�[�g�{�b�N�X�����O�`��Mod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteDesignRingMod.h"
#include "MTNoteBoxRingMod.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteBoxRingMod::MTNoteBoxRingMod(void)
{
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteBoxRingMod::~MTNoteBoxRingMod(void)
{
}

//******************************************************************************
// �m�[�g�f�U�C������
//******************************************************************************
int MTNoteBoxRingMod::_CreateNoteDesign()
{
	int result = 0;

	try {
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
