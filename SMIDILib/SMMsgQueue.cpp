//******************************************************************************
//
// Simple MIDI Library / SMMsgQueue
//
// メッセージキュークラス
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "SMMsgQueue.h"

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMMsgQueue::SMMsgQueue(void)
 : m_List(sizeof(unsigned long)*2, 10000)
{
	InitializeCriticalSection(&m_CriticalSection);
	m_MaxMsgNum = 0;
	m_NextPostIndex = 0;
	m_NextReadIndex = 0;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMMsgQueue::~SMMsgQueue(void)
{
	DeleteCriticalSection(&m_CriticalSection);
}

//******************************************************************************
// バッファ作成
//******************************************************************************
int SMMsgQueue::Initialize(
		unsigned long maxMsgNum
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long dummy[2] = {0, 0};
	
	//作成済みなら何もしない
	if (m_List.GetSize() > 0) goto EXIT;
	
	for (index = 0; index < maxMsgNum; index++) {
		result = m_List.AddItem(dummy);
		if (result != 0) goto EXIT;
	}
	m_MaxMsgNum = maxMsgNum;
	
EXIT:;
	return result;
}

//******************************************************************************
// メッセージ登録
//******************************************************************************
int SMMsgQueue::PostMessage(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	unsigned long params[2] = {0, 0};
	
	EnterCriticalSection(&m_CriticalSection);
	
	params[0] = param1;
	params[1] = param2;
	
	//パラメータ登録
	result = m_List.SetItem(m_NextPostIndex, params);
	if (result != 0) goto EXIT;
	
	//次回読み込み位置を更新
	m_NextPostIndex++;
	if (m_NextPostIndex == m_MaxMsgNum) {
		m_NextPostIndex = 0;
	}
	
	//読み込みしていない最も古いデータが上書きによって捨てられた場合
	if (m_NextPostIndex == m_NextReadIndex) {
		//読み込み位置を繰り上げる（捨てられたデータは無視する）
		m_NextReadIndex++;
		if (m_NextReadIndex == m_MaxMsgNum) {
			m_NextReadIndex = 0;
		}
	}
	
EXIT:;
	LeaveCriticalSection(&m_CriticalSection);
	return result;
}

//******************************************************************************
// メッセージ取得
//******************************************************************************
int SMMsgQueue::GetMessage(
		bool* pIsExist,
		unsigned long* pParam1,
		unsigned long* pParam2
	)
{
	int result = 0;
	unsigned long params[2] = {0, 0};
	
	EnterCriticalSection(&m_CriticalSection);
	
	*pIsExist = false;
	
	//メッセージが空の場合
	if (m_NextReadIndex == m_NextPostIndex) goto EXIT;
	
	//パラメータ取得
	result = m_List.GetItem(m_NextReadIndex, params);
	if (result != 0) goto EXIT;
	
	*pParam1 = params[0];
	*pParam2 = params[1];
	
	//次回読み取り位置を更新
	m_NextReadIndex++;
	if (m_NextReadIndex == m_MaxMsgNum) {
		m_NextReadIndex = 0;
	}
	
	*pIsExist = true;
	
EXIT:;
	LeaveCriticalSection(&m_CriticalSection);
	return result;
}

} // end of namespace

