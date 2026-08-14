//******************************************************************************
//
// YN Base Library / YNErrInfo
//
// Error information class.
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNErrInfo.h"

namespace YNBaseLib {

//******************************************************************************
// Constructor
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
// Destructor
//******************************************************************************
YNErrInfo::~YNErrInfo(void)
{
}

//******************************************************************************
// Get error level
//******************************************************************************
YNErrInfo::ErrLevel YNErrInfo::GetErrLevel()
{
	return m_ErrLevel;
}

//******************************************************************************
// Get line number
//******************************************************************************
unsigned long YNErrInfo::GetLineNo()
{
	return m_LineNo;
}

//******************************************************************************
// Get file name
//******************************************************************************
const TCHAR* YNErrInfo::GetFuncName()
{
	return m_FuncName.c_str();
}

//******************************************************************************
// Get message
//******************************************************************************
const TCHAR* YNErrInfo::GetMessage()
{
	return m_Message.c_str();
}

//******************************************************************************
// Get error information 1
//******************************************************************************
unsigned long long YNErrInfo::GetErrInfo1()
{
	return m_ErrInfo1;
}

//******************************************************************************
// Get error information 2
//******************************************************************************
unsigned long long YNErrInfo::GetErrInfo2()
{
	return m_ErrInfo2;
}

} // end of namespace

