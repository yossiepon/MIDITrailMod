//******************************************************************************
//
// Simple MIDI Library / SMEventSysMsg
//
// �V�X�e�����b�Z�[�W�C�x���g�N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventSysMsg.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMEventSysMsg::SMEventSysMsg()
{
	m_pEvent = NULL;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMEventSysMsg::~SMEventSysMsg(void)
{
}

//******************************************************************************
// �C�x���g�R�t��
//******************************************************************************
void SMEventSysMsg::Attach(
		SMEvent* pEvent
	)
{
	m_pEvent = pEvent;
}

//******************************************************************************
// MIDI�o�̓��b�Z�[�W�擾�i�V���[�g�j
//******************************************************************************
int SMEventSysMsg::GetMIDIOutShortMsg(
		unsigned long* pMsg,
		unsigned long* pSize
	)
{
	int result = 0;
	unsigned char status = 0;
	unsigned char* pData = NULL;
	unsigned char data1 = 0;
	unsigned char data2 = 0;
	
	if (m_pEvent == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0 );
		goto EXIT;
	}
	
	status = m_pEvent->GetStatus();
	pData = m_pEvent->GetDataPtr();
	
	if (m_pEvent->GetDataSize() == 2) {
		data1 = pData[0];
		data2 = pData[1];
		*pSize = 3;
	}
	else if (m_pEvent->GetDataSize() == 1) {
		data1 = pData[0];
		data2 = 0;
		*pSize = 2;
	}
	else if (m_pEvent->GetDataSize() == 0) {
		data1 = 0;
		data2 = 0;
		*pSize = 1;
	}
	else {
		result = YN_SET_ERR("Program error.", m_pEvent->GetDataSize(), 0);
		goto EXIT;
	}
	
	*pMsg = (unsigned long)((data2 << 16) | (data1 << 8) | (status));
	
EXIT:;
	return result;
}

} // end of namespace


