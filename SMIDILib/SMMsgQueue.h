//******************************************************************************
//
// Simple MIDI Library / SMMsgQueue
//
// メッセージキュークラスヘッダ
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMSimpleList.h"

namespace SMIDILib {


//******************************************************************************
// メッセージキュークラス
//******************************************************************************
class SMIDILIB_API SMMsgQueue
{
public:
	
	//コンストラクタ／デストラクタ
	SMMsgQueue(void);
	virtual ~SMMsgQueue(void);
	
	//初期化
	int Initialize(unsigned long maxMsgNum);
	
	//メッセージ登録
	int PostMessage(unsigned long param1, unsigned long param2);
	
	//メッセージ取得
	int GetMessage(bool* pIsExist, unsigned long* pParam1, unsigned long* pParam2);
	
private:
	
	CRITICAL_SECTION m_CriticalSection;
	
	SMSimpleList m_List;
	unsigned long m_MaxMsgNum;
	unsigned long m_NextPostIndex;
	unsigned long m_NextReadIndex;

};

} // end of namespace

