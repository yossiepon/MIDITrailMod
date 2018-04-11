//******************************************************************************
//
// Simple MIDI Library / SMEvent
//
// イベントクラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

namespace SMIDILib {

//******************************************************************************
// パラメータ定義
//******************************************************************************
#define SMEVENT_INTERNAL_DATA_SIZE  (16)


//******************************************************************************
// イベントクラス
//******************************************************************************
class SMIDILIB_API SMEvent
{
public:

	//イベント種別
	enum EventType {
		EventNone,
		EventMIDI,
		EventSysEx,
		EventSysMsg,
		EventMeta
	};

	//コンストラクタ／デストラクタ
	SMEvent(void);
	virtual ~SMEvent(void);

	//データ登録
	int SetData(EventType type, unsigned char status, unsigned char meta, unsigned char* pData, unsigned long size);

	//MIDIイベント登録
	int SetMIDIData(unsigned char status, unsigned char* pData, unsigned long size);

	//SysExイベント登録
	int SetSysExData(unsigned char status, unsigned char* pData, unsigned long size);

	//SysMsgイベント登録
	int SetSysMsgData(unsigned char status, unsigned char* pData, unsigned long size);

	//メタイベント登録
	int SetMetaData(unsigned char status, unsigned char type, unsigned char* pData, unsigned long size);

	//イベント種別取得
	EventType GetType();

	//ステータス取得
	unsigned char GetStatus();

	//ステータス設定
	void SetStatus(unsigned char status);

	//メタ種別取得
	unsigned char GetMetaType();

	//データサイズ取得
	unsigned long GetDataSize();

	//データポインタ取得
	unsigned char* GetDataPtr();

	//クリア
	void Clear();

private:

	EventType m_Type;
	unsigned char m_Status;
	unsigned char m_MetaType;
	unsigned long m_DataSize;
	unsigned char m_Data[SMEVENT_INTERNAL_DATA_SIZE];
	unsigned char* m_pExData;

	//代入とコピーコンストラクタの禁止
	void operator=(const SMEvent&);
	SMEvent(const SMEvent&);

};

} // end of namespace

