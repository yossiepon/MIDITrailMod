//******************************************************************************
//
// Simple MIDI Library / SMEventWatcher
//
// イベントウォッチャークラス
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventWatcher.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMEventWatcher::SMEventWatcher(void)
{
	m_pMsgTrans = NULL;
	_ClearChInfo();
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMEventWatcher::~SMEventWatcher(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int SMEventWatcher::Initialize(SMMsgTransmitter* pMsgTrans)
{
	int result = 0;
	
	m_pMsgTrans = pMsgTrans;
	
	_ClearChInfo();
	
	return result;
}

//******************************************************************************
// イベントウォッチ
//******************************************************************************
int SMEventWatcher::WatchEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;
	SMEventMIDI eventMIDI;
	
	if (pEvent->GetType() == SMEvent::EventMIDI) {
		eventMIDI.Attach(pEvent);
		
		//MIDIイベント監視
		result = _WatchEventMIDI(portNo, &eventMIDI);
		if (result != 0) goto EXIT;
		
		//コントロールチェンジ監視
		if (eventMIDI.GetChMsg() == SMEventMIDI::ControlChange) {
			result = _WatchEventControlChange(portNo, &eventMIDI);
			if (result != 0) goto EXIT;
			result = _WatchEventControlChange2(portNo, &eventMIDI);
			if (result != 0) goto EXIT;
		}
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDIイベントウォッチ
//******************************************************************************
int SMEventWatcher::WatchEventMIDI(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	return _WatchEventMIDI(portNo, pMIDIEvent);
}

//******************************************************************************
// コントロールチェンジイベントウォッチ
//******************************************************************************
int SMEventWatcher::WatchEventControlChange(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	return _WatchEventControlChange(portNo, pMIDIEvent);
}

//******************************************************************************
// チャンネル情報クリア
//******************************************************************************
void SMEventWatcher::_ClearChInfo()
{
	unsigned long portNo = 0;
	unsigned long chNo = 0;
	
	for (portNo = 0; portNo < SM_MAX_PORT_NUM; portNo++) {
		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
			//RPN/NRPN選択状態
			m_RPN_NRPN_Select[portNo][chNo] = RPN_NULL;
			//RPN
			m_RPN_MSB[portNo][chNo] = 0x7F; //RPN NULL
			m_RPN_LSB[portNo][chNo] = 0x7F; //RPN NULL
			//ピッチベンド感度
			m_PitchBendSensitivity[portNo][chNo] = SM_DEFAULT_PITCHBEND_SENSITIVITY;
		}
	}
	
	return;
}

//******************************************************************************
// MIDIイベント監視処理
//******************************************************************************
int SMEventWatcher::_WatchEventMIDI(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	
	//ノートOFFを通知
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOff) {
		m_pMsgTrans->PostNoteOff(
				portNo,
				pMIDIEvent->GetChNo(),
				pMIDIEvent->GetNoteNo()
			);
	}
	//ノートONを通知
	else if (pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOn) {
		m_pMsgTrans->PostNoteOn(
				portNo,
				pMIDIEvent->GetChNo(),
				pMIDIEvent->GetNoteNo(),
				pMIDIEvent->GetVelocity()
			);
	}
	//ピッチベンドを通知
	else if (pMIDIEvent->GetChMsg() == SMEventMIDI::PitchBend) {
		m_pMsgTrans->PostPitchBend(
				portNo,
				pMIDIEvent->GetChNo(),
				pMIDIEvent->GetPitchBendValue(),
				m_PitchBendSensitivity[portNo][pMIDIEvent->GetChNo()]
			);
	}
	
	return result;
}

//******************************************************************************
// コントロールチェンジ監視処理
//******************************************************************************
int SMEventWatcher::_WatchEventControlChange(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	unsigned char msb = 0;
	unsigned char chNo = 0;
	
	chNo = pMIDIEvent->GetChNo();
	
	//----------------------------------------------------------------
	// NRPN MSB / LSB
	//----------------------------------------------------------------
	//NRPN MSB (CC#99)
	if (pMIDIEvent->GetCCNo() == 0x63) {
		m_RPN_NRPN_Select[portNo][chNo] = NRPN;
	}
	//NRPN LSB (CC#98)
	if (pMIDIEvent->GetCCNo() == 0x62) {
		m_RPN_NRPN_Select[portNo][chNo] = NRPN;
	}
	
	//----------------------------------------------------------------
	// RPN MSB / LSB
	//----------------------------------------------------------------
	//RPN MSB (CC#101)
	if (pMIDIEvent->GetCCNo() == 0x65) {
		m_RPN_NRPN_Select[portNo][chNo] = RPN;
		m_RPN_MSB[portNo][chNo] = pMIDIEvent->GetCCValue();
	}
	//RPN LSB (CC#100)
	if (pMIDIEvent->GetCCNo() == 0x64) {
		m_RPN_NRPN_Select[portNo][chNo] = RPN;
		m_RPN_LSB[portNo][chNo] = pMIDIEvent->GetCCValue();
		if ((m_RPN_MSB[portNo][chNo] == 0x7F)
			&& (m_RPN_LSB[portNo][chNo] == 0x7F)) {
			m_RPN_NRPN_Select[portNo][chNo] = RPN_NULL;
		}
	}
	
	//----------------------------------------------------------------
	// Data Entry MSB / LSB
	//----------------------------------------------------------------
	//Data Entry MSB (CC#6)
	if (pMIDIEvent->GetCCNo() == 0x06) {
		//ピッチベンド感度 MSB
		if (_GetCurRPNType(portNo, chNo) == PitchBendSensitivity) {
			m_PitchBendSensitivity[portNo][chNo] = pMIDIEvent->GetCCValue();
		}
	}
	//Data Entry LSB (CC#38)
	if (pMIDIEvent->GetCCNo() == 0x26) {
		//特に制御なし
	}
	
	//Data Increment (CC#96)
	if (pMIDIEvent->GetCCNo() == 0x60) {
		//ピッチベンド感度 MSB
		if (_GetCurRPNType(portNo, chNo) == PitchBendSensitivity) {
			msb = m_PitchBendSensitivity[portNo][chNo];
			if (msb < 24) {
				m_PitchBendSensitivity[portNo][chNo] = msb++;
			}
		}
	}
	//Data Decremnet (CC#97)
	if (pMIDIEvent->GetCCNo() == 0x61) {
		//ピッチベンド感度 MSB
		if (_GetCurRPNType(portNo, chNo) == PitchBendSensitivity) {
			msb = m_PitchBendSensitivity[portNo][chNo]++;
			if (msb > 0) {
				m_PitchBendSensitivity[portNo][chNo] = msb--;
			}
		}
	}
	
	//----------------------------------------------------------------
	// リセットオールコントローラ
	//----------------------------------------------------------------
	//Reset All Controllers (CC#121)
	if (pMIDIEvent->GetCCNo() == 0x79) {
		//ピッチベンドを通知：0
		m_pMsgTrans->PostPitchBend(portNo, chNo, 0, m_PitchBendSensitivity[portNo][chNo]);
		//RPN/NRPN選択状態
		m_RPN_NRPN_Select[portNo][chNo] = RPN_NULL;
		//RPN
		m_RPN_MSB[portNo][chNo] = 0x7F; //RPN NULL
		m_RPN_LSB[portNo][chNo] = 0x7F; //RPN NULL
		
		//Roland SCシリーズ,Yamaha MUシリーズの場合
		//CC#121 リセットオールコントローラで次の値がクリアされる
		//  An     ポリフォニックキープレッシャー  0
		//  Dn     チャンネルプレッシャー  0
		//  En     ピッチベンド  0
		//  CC#1   モジュレーション  0
		//  CC#11  エクスプレッション  127
		//  CC#64  ホールド1    0
		//  CC#65  ポルタメント  0
		//  CC#66  ソステヌート  0
		//  CC#67  ソフト  0
		//  CC#98,99   NRPN  未設定状態（設定済みデータは変化しない）
		//  CC#100,101 RPN   未設定状態（設定済みデータは変化しない）
	}
	
	//EXIT:;
	return result;
}

//******************************************************************************
// RPN種別取得
//******************************************************************************
SMEventWatcher::RPN_Type SMEventWatcher::_GetCurRPNType(
		unsigned char portNo,
		unsigned char chNo
	)
{
	RPN_Type type = RPN_None;
	
	if (m_RPN_NRPN_Select[portNo][chNo] == RPN) {
		if ((m_RPN_MSB[portNo][chNo] == 0x00)
			&& (m_RPN_LSB[portNo][chNo] == 0x00)) {
			type = PitchBendSensitivity;
		}
		if ((m_RPN_MSB[portNo][chNo] == 0x00)
			&& (m_RPN_LSB[portNo][chNo] == 0x01)) {
			type = MasterFineTune;
		}
		if ((m_RPN_MSB[portNo][chNo] == 0x00)
			&& (m_RPN_LSB[portNo][chNo] == 0x02)) {
			type = MasterCourseTune;
		}
	}
	
	return type;
}

//******************************************************************************
// コントロールチェンジ監視処理2
//******************************************************************************
int SMEventWatcher::_WatchEventControlChange2(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	unsigned char chNo = 0;
	
	chNo = pMIDIEvent->GetChNo();
	
	//ALL SOUND OFF (CC#120)
	//ALL NOTE OFF (CC#123)
	if ((pMIDIEvent->GetCCNo() == 0x78) || (pMIDIEvent->GetCCNo() == 0x7B)) {
		m_pMsgTrans->PostAllNoteOff(portNo, chNo);
	}
	
	return result;
}

} // end of namespace


