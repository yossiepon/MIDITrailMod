//******************************************************************************
//
// Simple MIDI Library / SMEventSysEx
//
// SysExイベントクラス
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
// コンストラクタ
//******************************************************************************
SMEventSysEx::SMEventSysEx()
{
	m_pEvent = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMEventSysEx::~SMEventSysEx(void)
{
}

//******************************************************************************
// イベント紐付け
//******************************************************************************
void SMEventSysEx::Attach(
		SMEvent* pEvent
	)
{
	m_pEvent = pEvent;
}

//******************************************************************************
// MIDI出力メッセージ取得（ロング）
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


