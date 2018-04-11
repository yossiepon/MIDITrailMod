//******************************************************************************
//
// Simple Base Library / YNErrInfo
//
// エラー情報クラス
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

#include <string>
using namespace std;

namespace YNBaseLib {

//******************************************************************************
// エラー情報クラス
//******************************************************************************
class YNBASELIB_API YNErrInfo
{
public:

	//エラーレベル
	enum ErrLevel {
		LVL_ERR,
		LVL_WARN,
		LVL_INFO
	};

	//コンストラクタ／デストラクタ
	YNErrInfo(
			ErrLevel errLevel,
			unsigned long lineNo,
			const TCHAR* pFileName,
			const TCHAR* pMessage,
			unsigned long long errInfo1,
			unsigned long long errInfo2
		);
	virtual ~YNErrInfo(void);

	//エラーレベル取得
	ErrLevel GetErrLevel();

	//行番号取得
	unsigned long GetLineNo();

	//関数名取得
	const TCHAR* GetFuncName();

	//メッセージ取得
	const TCHAR* GetMessage();

	//エラー情報取得
	unsigned long long GetErrInfo1();
	unsigned long long GetErrInfo2();

private:

	ErrLevel m_ErrLevel;
	unsigned long m_LineNo;
	unsigned long long m_ErrInfo1;
	unsigned long long m_ErrInfo2;

//CRTをスタティックリンク(/MT)すると警告が出る
//#pragma warning(disable:4251)
#ifdef _UNICODE
	wstring m_FuncName;
	wstring m_Message;
#else
	string m_FuncName;
	string m_Message;
#endif
//#pragma warning(default:4251)

	//代入とコピーコンストラクタの禁止
	void operator=(const YNErrInfo&);
	YNErrInfo(const YNErrInfo&);

};

} // end of namespace

