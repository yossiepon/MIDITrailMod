//******************************************************************************
//
// Simple MIDI Library / SMEventWatcher
//
// イベントウォッチャークラス
//
// Copyright (C) 2012-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"
#include "SMEventMIDI.h"
#include "SMEventSysMsg.h"
#include "SMMsgTransmitter.h"
#include "SMCommon.h"


namespace SMIDILib {

//******************************************************************************
// イベントウォッチャークラス
//******************************************************************************
class SMIDILIB_API SMEventWatcher
{
public:
	
	//コンストラクタ／デストラクタ
	SMEventWatcher(void);
	virtual ~SMEventWatcher(void);
	
	//初期化
	int Initialize(SMMsgTransmitter* pMsgTrans);
	
	//イベント監視
	int WatchEvent(unsigned char portNo, SMEvent* pEvent);
	
	//イベント監視：シーケンサ向け
	int WatchEventMIDI(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int WatchEventControlChange(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	
private:
	
	//RPN/NRPN選択状態
	enum RPN_NRPN_Select {
		RPN_NULL,
		RPN,
		NRPN
	};
	
	//RPN種別
	enum RPN_Type {
		RPN_None,
		PitchBendSensitivity,
		MasterFineTune,
		MasterCourseTune
	};
	
	//メッセージ送信制御
	SMMsgTransmitter* m_pMsgTrans;
	
	//ピッチベンド制御
	unsigned char m_PitchBendSensitivity[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	
	//RPN制御系
	RPN_NRPN_Select m_RPN_NRPN_Select[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_RPN_MSB[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_RPN_LSB[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	
	void _ClearChInfo();
	int _WatchEventMIDI(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int _WatchEventControlChange(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int _WatchEventControlChange2(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	RPN_Type _GetCurRPNType(unsigned char portNo, unsigned char chNo);
	int _WatchEventSysMsg(unsigned char portNo, SMEventSysMsg* pEventSysMsg);
	
};

} // end of namespace


