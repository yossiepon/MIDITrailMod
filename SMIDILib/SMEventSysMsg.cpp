//******************************************************************************
//
// Simple MIDI Library / SMEventSysMsg
//
// システムメッセージイベントクラス
//
// Copyright (C) 2012-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventSysMsg.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMEventSysMsg::SMEventSysMsg()
{
	m_pEvent = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMEventSysMsg::~SMEventSysMsg(void)
{
}

//******************************************************************************
// イベント紐付け
//******************************************************************************
void SMEventSysMsg::Attach(
		SMEvent* pEvent
	)
{
	m_pEvent = pEvent;
}

//******************************************************************************
// MIDI出力メッセージ取得（ショート）
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

//******************************************************************************
// システムメッセージ種別取得
//******************************************************************************
SMEventSysMsg::SysMsg SMEventSysMsg::GetSysMsg()
{
	SysMsg msg = None;
	unsigned char* pData = NULL;
	unsigned char size = 0;

	if (m_pEvent == NULL) {
		goto EXIT;
	}

	switch (m_pEvent->GetStatus()) {
		case 0xF1: msg = Common_QuarterFrame;			size = 2; break;
		case 0xF2: msg = Common_SongPositionPointer;	size = 3; break;
		case 0xF3: msg = Common_SongSelect;				size = 2; break;
		case 0xF6: msg = Common_TuneRequest;			size = 1; break;
		case 0xF8: msg = RealTime_TimingClock;			size = 1; break;
		case 0xFA: msg = RealTime_Start;				size = 1; break;
		case 0xFB: msg = RealTime_Continue;				size = 1; break;
		case 0xFC: msg = RealTime_Stop;					size = 1; break;
		case 0xFE: msg = RealTime_ActiveSensing;		size = 1; break;
		case 0xFF: msg = RealTime_SystemReset;			size = 1; break;
		default: break;
	}
	if ((m_pEvent->GetDataSize() + 1) != size) {
		msg = None;
	}

EXIT:;
	return msg;
}

} // end of namespace


