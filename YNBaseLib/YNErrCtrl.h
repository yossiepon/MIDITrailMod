//******************************************************************************
//
// YN Base Library / YNErrCtrl
//
// Error control class.
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef YNBASELIB_EXPORTS
#define YNBASELIB_API __declspec(dllexport)
#else
#define YNBASELIB_API __declspec(dllimport)
#endif

#include "YNErrInfo.h"

namespace YNBaseLib {

//******************************************************************************
//Error control macros
//******************************************************************************
#define YN_SET_ERR(msg,info1,info2)   YNErrCtrl::SetErr(YNErrInfo::LVL_ERR,__LINE__,__FUNCTION__,msg,info1,info2)
#define YN_SET_WARN(msg,info1,info2)  YNErrCtrl::SetErr(YNErrInfo::LVL_WARN,__LINE__,__FUNCTION__,msg,info1,info2)
#define YN_SET_INFO(msg,info1,info2)  YNErrCtrl::SetErr(YNErrInfo::LVL_INFO,__LINE__,__FUNCTION__,msg,info1,info2)
#define YN_SHOW_ERR(howner)   YNErrCtrl::ShowErr(howner)


//******************************************************************************
// Error control class
//******************************************************************************
class YNBASELIB_API YNErrCtrl
{
private:

	//Constructor / Destructor
	//Instance creation is not allowed
	YNErrCtrl();
	virtual ~YNErrCtrl();

public:

	//Initialize
	//  Runs on process attach
	//  Not intended for general use
	static int Initialize();

	//Terminate
	//  Runs on process termination
	//  Not intended for general use
	static int Terminate();

	//Register error information
	static int SetErr(
			YNErrInfo::ErrLevel errLevel,
			unsigned long lineNo,
			const TCHAR* pFuncName,
			const TCHAR* pMessage,
			unsigned long long errInfo1,
			unsigned long long errInfo2
		);

	//Get error information
	static YNErrInfo* GetErr();

	//Show the error information dialog
	static int ShowErr(HWND hOwner);

private:

	//Prohibit assignment and copy constructor
	void operator=(const YNErrCtrl&);
	YNErrCtrl(const YNErrCtrl&);

};

} // end of namespace

