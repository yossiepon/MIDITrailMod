//******************************************************************************
//
// YN Base Library / YNErrInfo
//
// Error information class.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef YNBASELIB_EXPORTS
#define YNBASELIB_API __declspec(dllexport)
#else
#define YNBASELIB_API __declspec(dllimport)
#endif

#include <string>
using namespace std;

namespace YNBaseLib {

//******************************************************************************
// Error information class
//******************************************************************************
class YNBASELIB_API YNErrInfo
{
public:

	//Error level
	enum ErrLevel {
		LVL_ERR,
		LVL_WARN,
		LVL_INFO
	};

	//Constructor / Destructor
	YNErrInfo(
			ErrLevel errLevel,
			unsigned long lineNo,
			const TCHAR* pFileName,
			const TCHAR* pMessage,
			unsigned long long errInfo1,
			unsigned long long errInfo2
		);
	virtual ~YNErrInfo(void);

	//Get error level
	ErrLevel GetErrLevel();

	//Get line number
	unsigned long GetLineNo();

	//Get function name
	const TCHAR* GetFuncName();

	//Get message
	const TCHAR* GetMessage();

	//Get error information
	unsigned long long GetErrInfo1();
	unsigned long long GetErrInfo2();

private:

	ErrLevel m_ErrLevel;
	unsigned long m_LineNo;
	unsigned long long m_ErrInfo1;
	unsigned long long m_ErrInfo2;

//A warning occurs when the CRT is statically linked (/MT)
#pragma warning(disable:4251)
#ifdef _UNICODE
	wstring m_FuncName;
	wstring m_Message;
#else
	string m_FuncName;
	string m_Message;
#endif
#pragma warning(default:4251)

	//Prohibit assignment and copy constructor
	void operator=(const YNErrInfo&);
	YNErrInfo(const YNErrInfo&);

};

} // end of namespace

