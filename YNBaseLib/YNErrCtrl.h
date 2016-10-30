//******************************************************************************
//
// Simple Base Library / YNErrCtrl
//
// エラー制御クラス
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
//エラー制御マクロ
//******************************************************************************
#define YN_SET_ERR(msg,info1,info2)   YNErrCtrl::SetErr(YNErrInfo::LVL_ERR,__LINE__,__FUNCTION__,msg,info1,info2)
#define YN_SET_WARN(msg,info1,info2)  YNErrCtrl::SetErr(YNErrInfo::LVL_WARN,__LINE__,__FUNCTION__,msg,info1,info2)
#define YN_SET_INFO(msg,info1,info2)  YNErrCtrl::SetErr(YNErrInfo::LVL_INFO,__LINE__,__FUNCTION__,msg,info1,info2)
#define YN_SHOW_ERR(howner)   YNErrCtrl::ShowErr(howner)


//******************************************************************************
// エラー制御クラス
//******************************************************************************
class YNBASELIB_API YNErrCtrl
{
private:

	//コンストラクタ／デストラクタ
	//インスタンス生成を許可しない
	YNErrCtrl();
	virtual ~YNErrCtrl();

public:

	//初期化
	//  プロセスアタッチ時に実行する
	//  一般利用者は利用しないこと
	static int Initialize();

	//終了
	//  プロセス終了時に実行する
	//  一般利用者は利用しないこと
	static int Terminate();

	//エラー情報登録
	static int SetErr(
			YNErrInfo::ErrLevel errLevel,
			unsigned long lineNo,
			const TCHAR* pFuncName,
			const TCHAR* pMessage,
			unsigned long long errInfo1,
			unsigned long long errInfo2
		);

	//エラー情報取得
	static YNErrInfo* GetErr();

	//エラー情報ダイアログ表示
	static int ShowErr(HWND hOwner);

private:

	//代入とコピーコンストラクタの禁止
	void operator=(const YNErrCtrl&);
	YNErrCtrl(const YNErrCtrl&);

};

} // end of namespace

