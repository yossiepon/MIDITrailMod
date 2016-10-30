//******************************************************************************
//
// Simple MIDI Library / SMEvent
//
// イベントクラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEvent.h"
#include <new>

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMEvent::SMEvent(void)
{
	m_pExData = NULL;
	Clear();
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMEvent::~SMEvent(void)
{
	delete [] m_pExData;
}

//******************************************************************************
// データ登録
//******************************************************************************
int SMEvent::SetData(
		EventType type,
		unsigned char status,
		unsigned char meta,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;

	m_Type = type;
	m_Status = status;
	m_MetaType = meta;

	delete [] m_pExData;
	m_pExData = NULL;
	m_DataSize = 0;
	ZeroMemory(m_Data, SMEVENT_INTERNAL_DATA_SIZE);

	if (size == 0) {
		//何もしない
	}
	else if (size <= SMEVENT_INTERNAL_DATA_SIZE) {
		memcpy(m_Data, pData, size);
	}
	else {
		try {
			m_pExData = new unsigned char[size];
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", size, 0);
			goto EXIT;
		}
		memcpy(m_pExData, pData, size);
	}

	m_DataSize = size;

EXIT:;
	return result;
}

//******************************************************************************
// MIDIイベントデータ登録
//******************************************************************************
int SMEvent::SetMIDIData(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;

	result = SetData(EventMIDI, status, 0, pData, size);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// SysExイベントデータ登録
//******************************************************************************
int SMEvent::SetSysExData(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;
	unsigned char* pExData = NULL;

	//ステータス 0xF0 の場合は先頭パケット
	// → 先頭に 0xF0 をつけて送信する
	if (status == 0xF0) {
		try {
			pExData = new unsigned char[size + 1];
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", size + 1, 0);
			goto EXIT;
		}
		pExData[0] = status;
		memcpy(&(pExData[1]), pData, size);
		result = SetData(EventSysEx, status, 0, pExData, size + 1);
		if (result != 0) goto EXIT;
	}
	//ステータス 0xF7 の場合は後続パケット
	// → 先頭に 0xF7 をつけて送信しない
	else if (status == 0xF7) {
		result = SetData(EventSysEx, status, 0, pData, size);
		if (result != 0) goto EXIT;
	}
	//それ以外はエラー
	else {
		result = YN_SET_ERR("Program error.", status, 0);
		goto EXIT;
	}

EXIT:;
	delete [] pExData;
	return result;
}

//******************************************************************************
// SysMsgイベントデータ登録
//******************************************************************************
int SMEvent::SetSysMsgData(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;
	
	result = SetData(EventSysMsg, status, 0, pData, size);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// メタイベントデータ登録
//******************************************************************************
int SMEvent::SetMetaData(
		unsigned char status,
		unsigned char type,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;

	result = SetData(EventMeta, status, type, pData, size);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// イベント種別取得
//******************************************************************************
SMEvent::EventType SMEvent::GetType()
{
	return m_Type;
}

//******************************************************************************
// ステータス取得
//******************************************************************************
unsigned char SMEvent::GetStatus()
{
	return m_Status;
}

//******************************************************************************
// メタイベント種別取得
//******************************************************************************
unsigned char SMEvent::GetMetaType()
{
	return m_MetaType;
}

//******************************************************************************
// データサイズ取得
//******************************************************************************
unsigned long SMEvent::GetDataSize()
{
	return m_DataSize;
}

//******************************************************************************
// データ位置取得
//******************************************************************************
unsigned char* SMEvent::GetDataPtr()
{
	unsigned char* pData = NULL;

	if (m_DataSize <= SMEVENT_INTERNAL_DATA_SIZE) {
		pData = m_Data;
	}
	else {
		pData = m_pExData;
	}

	return pData;
}

//******************************************************************************
// クリア
//******************************************************************************
void SMEvent::Clear()
{
	delete [] m_pExData;
	
	m_Type = EventNone;
	m_Status = 0;
	m_MetaType = 0;
	m_DataSize = 0;
	memset(m_Data, 0, SMEVENT_INTERNAL_DATA_SIZE);
	m_pExData = NULL;
}

} // end of namespace


