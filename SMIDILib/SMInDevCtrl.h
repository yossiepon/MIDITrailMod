//******************************************************************************
//
// Simple MIDI Library / SMInDevCtrl
//
// MIDI入力デバイス制御クラス
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include <list>
#include "mmsystem.h"
#include "SMEvent.h"

#pragma warning(disable:4251)

namespace SMIDILib {


//******************************************************************************
// パラメータ定義
//******************************************************************************
//MIDIイベント読み込みコールバック関数
typedef int (*SMInReadCallBack)(SMEvent* pEvent, void* pUserParam);

//システムエクスクルーシブ用バッファサイズ
//  サイズの根拠は特になし
#define SM_MIDIIN_BUF_SIZE  (1024 * 10)


//******************************************************************************
// MIDI入力デバイス制御クラス
//******************************************************************************
class SMIDILIB_API SMInDevCtrl
{
public:
	
	//コンストラクタ／デストラクタ
	SMInDevCtrl(void);
	virtual ~SMInDevCtrl(void);
	
	//初期化
	int Initialize();
	
	//デバイス数取得
	unsigned long GetDevNum();
	
	//デバイスプロダクト名称取得
	int GetDevProductName(unsigned long index, std::string& name);
	
	//ポート対応デバイス登録
	int SetPortDev(const char* pProductName);
	
	//MIDIイベント読み込みコールバック関数登録
	void SetInReadCallBack(SMInReadCallBack pCallBack, void* pUserParam);
	
	//全デバイスのオープン／クローズ
	int OpenPortDev();
	int ClosePortDev();
	
	//ポート情報クリア
	int ClearPortInfo();
	
private:
	
	//ポート情報
	typedef struct {
		bool isExist;
		unsigned long devId;
		HMIDIIN hMidiIn;
		MIDIHDR midiHdr;
	} SMPortInfo;
	
	//デバイス情報
	typedef struct {
		unsigned long devId;
		char productName[MAXPNAMELEN];
	} SMInDevInfo;
	
	//入力デバイスリスト
	typedef std::list<SMInDevInfo> SMInDevList;
	typedef std::list<SMInDevInfo>::iterator SMInDevListItr;
	SMInDevList m_InDevList;
	
	//ポート情報
	SMPortInfo m_PortInfo;
	
	//コールバック関数
	SMInReadCallBack m_pInReadCallBack;
	void* m_pCallBackUserParam;
	
	//パケット解析系
	bool m_isContinueSysEx;
	
	int _InitDevList();
	static void CALLBACK _InReadCallBack(
			HMIDIIN hMidiIn,
			UINT wMsg,
			DWORD dwInstance,
			DWORD dwParam1,
			DWORD dwParam2
		);
	void _InReadProc(
			HMIDIIN hMidiIn,
			UINT wMsg,
			DWORD dwParam1,
			DWORD dwParam2
		);
	int _InReadProcMIDI(
			DWORD midiMessage,
			DWORD timestamp,
			SMEvent* pEvent
		);
	int _InReadProcSysEx(
			MIDIHDR* pMIDIHDR,
			DWORD timestamp,
			bool* pIsContinueSysEx,
			SMEvent* pEvent
		);
	unsigned long _GetMIDIMsgSize(unsigned char status);
	unsigned long _GetSysMsgSize(unsigned char status);
	
};

} // end of namespace

#pragma warning(default:4251)


