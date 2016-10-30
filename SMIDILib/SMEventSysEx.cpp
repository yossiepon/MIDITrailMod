//******************************************************************************
//
// Simple MIDI Library / SMEventSysEx
//
// SysEx�C�x���g�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventSysEx.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMEventSysEx::SMEventSysEx()
{
	m_pEvent = NULL;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMEventSysEx::~SMEventSysEx(void)
{
}

//******************************************************************************
// �C�x���g�R�t��
//******************************************************************************
void SMEventSysEx::Attach(
		SMEvent* pEvent
	)
{
	m_pEvent = pEvent;
}

//******************************************************************************
// MIDI�o�̓��b�Z�[�W�擾�i�����O�j
//******************************************************************************
int SMEventSysEx::GetMIDIOutLongMsg(
		unsigned char** pPtrMsg,
		unsigned long* pSize
	)
{
	int result = 0;

	if (m_pEvent == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	*pSize = m_pEvent->GetDataSize();
	*pPtrMsg = m_pEvent->GetDataPtr();

EXIT:;
	return result;
}

} // end of namespace


