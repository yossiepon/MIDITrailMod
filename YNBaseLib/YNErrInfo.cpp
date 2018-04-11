//******************************************************************************
//
// Simple Base Library / YNErrInfo
//
// エラー情報クラス
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNErrInfo.h"

namespace YNBaseLib {

//******************************************************************************
// コンストラクタ
//******************************************************************************
YNErrInfo::YNErrInfo(
		ErrLevel errLevel,
		unsigned long lineNo,
		const TCHAR* pFuncName,
		const TCHAR* pMessage,
		unsigned long long errInfo1,
		unsigned long long errInfo2
	)
{	
	m_ErrLevel = errLevel;
	m_LineNo = lineNo;
	m_FuncName = pFuncName; 
	m_Message = pMessage;
	m_ErrInfo1 = errInfo1;
	m_ErrInfo2 = errInfo2;
	return;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
YNErrInfo::~YNErrInfo(void)
{
}

//******************************************************************************
// エラーレベル取得
//******************************************************************************
YNErrInfo::ErrLevel YNErrInfo::GetErrLevel()
{
	return m_ErrLevel;
}

//******************************************************************************
// 行番号取得
//******************************************************************************
unsigned long YNErrInfo::GetLineNo()
{
	return m_LineNo;
}

//******************************************************************************
// ファイル名取得
//******************************************************************************
const TCHAR* YNErrInfo::GetFuncName()
{
	return m_FuncName.c_str();
}

//******************************************************************************
// メッセージ取得
//******************************************************************************
const TCHAR* YNErrInfo::GetMessage()
{
	return m_Message.c_str();
}

//******************************************************************************
// エラー情報1取得
//******************************************************************************
unsigned long long YNErrInfo::GetErrInfo1()
{
	return m_ErrInfo1;
}

//******************************************************************************
// エラー情報2取得
//******************************************************************************
unsigned long long YNErrInfo::GetErrInfo2()
{
	return m_ErrInfo2;
}

} // end of namespace

