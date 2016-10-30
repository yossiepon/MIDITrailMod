//******************************************************************************
//
// Simple MIDI Library / SMOutDevCtrl
//
// MIDI出力デバイス制御クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "mmsystem.h"
#include <string>
#include <list>

#pragma warning(disable:4251)

namespace SMIDILib {

//******************************************************************************
// パラメータ定義
//******************************************************************************
//最大ポート数：A,B,C,D,E,F
#define SM_MIDIOUT_PORT_NUM_MAX   (6)

//******************************************************************************
// MIDI出力デバイス制御クラス
//******************************************************************************
class SMIDILIB_API SMOutDevCtrl
{
public:

	//コンストラクタ／デストラクタ
	SMOutDevCtrl(void);
	virtual ~SMOutDevCtrl(void);

	//初期化
	int Initialize();

	//デバイス数取得
	unsigned long GetDevNum();

	//デバイスプロダクト名称取得
	int GetDevProductName(unsigned long index, std::string& name);

	//ポート対応デバイス登録
	int SetPortDev(unsigned char portNo, const char* pProductName);

	//ポート対応デバイスID取得
	int GetPortDevId(unsigned char portNo, unsigned long* pDevId);

	//全デバイスのオープン／クローズ
	int OpenPortDevAll();
	int ClosePortDevAll();

	//ポート情報クリア
	int ClearPortInfo();

	//MIDI出力メッセージ送信
	int SendShortMsg(unsigned char portNo, unsigned long msg);
	int SendLongMsg(unsigned char portNo, unsigned char* pMsg, unsigned long size);
	int NoteOffAll();

private:

	typedef struct {
		bool isExist;
		unsigned long devId;
		HMIDIOUT hMIDIOut;
	} SMPortInfo;

	SMPortInfo m_PortInfo[SM_MIDIOUT_PORT_NUM_MAX];

	typedef struct {
		unsigned long devId;
		char productName[MAXPNAMELEN];
	} SMOutDevInfo;

	typedef std::list<SMOutDevInfo> SMOutDevList;
	typedef std::list<SMOutDevInfo>::iterator SMOutDevListItr;

	SMOutDevList m_OutDevList;

	int _InitDevList();

	//代入とコピーコンストラクタの禁止
	void operator=(const SMOutDevCtrl&);
	SMOutDevCtrl(const SMOutDevCtrl&);

};

} // end of namespace

#pragma warning(default:4251)

