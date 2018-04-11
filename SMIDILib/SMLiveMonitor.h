//******************************************************************************
//
// Simple MIDI Library / SMLiveMonitor
//
// ライブモニタクラス
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"
#include "SMMsgQueue.h"
#include "SMMsgTransmitter.h"
#include "SMInDevCtrl.h"
#include "SMOutDevCtrl.h"
#include "SMEventWatcher.h"

namespace SMIDILib {


//******************************************************************************
// パラメータ定義
//******************************************************************************


//******************************************************************************
// ライブモニタクラス
//******************************************************************************
class SMIDILIB_API SMLiveMonitor
{
public:
	
	//演奏状態
	enum Status {
		StatusMonitorOFF,
		StatusMonitorON
	};
	
	//コンストラクタ／デストラクタ
	SMLiveMonitor(void);
	virtual ~SMLiveMonitor(void);
	
	//初期化
	int Initialize(SMMsgQueue* pMsgQueue);
	
	//ポート対応デバイス登録
	int SetInPortDev(const char* pProductName, bool isMIDITHRU);
	int SetOutPortDev(const char* pProductName);
	
	//入力ポートデバイス表示名取得
	//NSString* GetInPortDevDisplayName(NSString* pIdName);
	int GetInPortDevDisplayName(std::string& name);
	
	//モニタ開始
	int Start();
	
	//モニタ停止
	int Stop();
	
private:
	
	//演奏状態
	Status m_Status;
	SMMsgTransmitter m_MsgTrans;
	SMMsgQueue* m_pMsgQue;
	SMEventWatcher m_EventWatcher;
	
	//MIDIデバイス系
	char m_InPortDevName[MAXPNAMELEN];
	char m_OutPortDevName[MAXPNAMELEN];
	bool m_isMIDITHRU;
	SMInDevCtrl m_InDevCtrl;
	SMOutDevCtrl m_OutDevCtrl;
	
	//ポート制御
	void _ClearPortInfo();
	int _OpenMIDIDev();
	int _CloseMIDIDev();
	
	static int _InReadCallBack(SMEvent* pEvent, void* pUserParam);
	int _InReadProc(SMEvent* pEvent);
	int _InReadProcParseEvent(SMEvent* pEvent);
	int _InReadProcMIDITHRU(SMEvent* pEvent);
	int _InReadProcSendMIDIEvent(unsigned char portNo, SMEvent* pEvent);
	int _InReadProcSendSysExEvent(unsigned char portNo, SMEvent* pEvent);
	int _InReadProcSendSysMsgEvent(unsigned char portNo, SMEvent* pEvent);
	
};

} // end of namespace


