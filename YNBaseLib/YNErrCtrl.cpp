//******************************************************************************
//
// Simple Base Library / YNErrCtrl
//
// エラー制御クラス
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "YNErrCtrl.h"

namespace YNBaseLib {

DWORD g_TlsIndex = 0xFFFFFFFF;

//******************************************************************************
// コンストラクタ
//******************************************************************************
YNErrCtrl::YNErrCtrl()
{
	return;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
YNErrCtrl::~YNErrCtrl()
{
	return;
}

//******************************************************************************
// 初期化
//******************************************************************************
int YNErrCtrl::Initialize()
{
	int result = 0;

	g_TlsIndex = TlsAlloc();
	if (g_TlsIndex == 0xFFFFFFFF) {
		result = -1;
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 終了処理
//******************************************************************************
int YNErrCtrl::Terminate()
{
	int result = 0;
	BOOL apiresult = false;

	if (g_TlsIndex != 0xFFFFFFFF) {
		apiresult = TlsFree(g_TlsIndex);
		if (!apiresult) {
			result = -1;
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// エラー登録
//******************************************************************************
int YNErrCtrl::SetErr(
		YNErrInfo::ErrLevel errLevel,
		unsigned long lineNo,
		const TCHAR* pFuncName,
		const TCHAR* pMessage,
		unsigned long long errInfo1,
		unsigned long long errInfo2
	)
{
	int result = 0;
	BOOL apiresult = false;
	YNErrInfo* pErrInfo = NULL;

	//エラー情報が登録されたままであれば破棄する
	pErrInfo = GetErr();
	if (pErrInfo != NULL) {
		delete pErrInfo;
		pErrInfo = NULL;
	}

	//エラー情報オブジェクトを生成
	pErrInfo = new YNErrInfo(errLevel, lineNo, pFuncName, pMessage, errInfo1, errInfo2);
	if (pErrInfo == NULL) {
		result = -2;
		goto EXIT;
	}

	//スレッドローカル記憶域に格納
	apiresult = TlsSetValue(g_TlsIndex, (void*)pErrInfo);
	if (!apiresult) {
		result = -2;
		goto EXIT;
	}
	pErrInfo = NULL;

	//TODO:エラーコード生成
	result = -1;

EXIT:;
	delete pErrInfo;
	return result;
}

//******************************************************************************
// エラー情報取得
//******************************************************************************
YNErrInfo* YNErrCtrl::GetErr()
{
	int result = 0;
	BOOL apiresult = false;
	YNErrInfo* pErrInfo = NULL;

	//スレッドローカル記憶域からエラー情報オブジェクトを取得
	pErrInfo = (YNErrInfo*)TlsGetValue(g_TlsIndex);
	if (pErrInfo == NULL) {
		result = -1;
		goto EXIT;
	}

	//スレッドローカル記憶域をクリア
	apiresult = TlsSetValue(g_TlsIndex, NULL);
	if (!apiresult) {
		result = -1;
		goto EXIT;
	}

EXIT:;
	return pErrInfo;
}

//******************************************************************************
// エラー表示
//******************************************************************************
int YNErrCtrl::ShowErr(
		HWND hOwner
	)
{
	int result = 0;
	int apiresult = 0;
	UINT type = 0;
	YNErrInfo* pErrInfo = NULL;
	TCHAR msgex[512];

#ifdef _UNICODE
	wstring msg;
	wstring title;
#else
	string msg;
	string title;
#endif

	//エラー情報がなければ何もしない
	pErrInfo = GetErr();
	if (pErrInfo == NULL) goto EXIT;

	switch (pErrInfo->GetErrLevel()) {
		case (YNErrInfo::LVL_ERR):
			type |= MB_ICONERROR;
			title = _T("ERROR");
			break;
		case (YNErrInfo::LVL_WARN):
			type |= MB_ICONWARNING;
			title = _T("WARNING");
			break;
		case (YNErrInfo::LVL_INFO):
			type |= MB_ICONINFORMATION;
			title = _T("INFORMATION");
			break;
	}

	if (hOwner == NULL) {
		type |= MB_SYSTEMMODAL;
	}

	msg = pErrInfo->GetMessage();
	_stprintf_s(
		msgex,
		512,
		_T("\n\nFUNC: %s\nLINE: %d\nINFO: %08X %08X"),
		pErrInfo->GetFuncName(),
		pErrInfo->GetLineNo(),
		pErrInfo->GetErrInfo1(),
		pErrInfo->GetErrInfo2()
	);
	msg += msgex;

	apiresult = MessageBox(
					hOwner,			//オーナーウィンドウ
					msg.c_str(),	//メッセージ
					title.c_str(),	//タイトル
					MB_OK | type	//フラグ
				);
	if (apiresult == 0) {
		result = -1;
		goto EXIT;
	}

EXIT:;
	delete pErrInfo;
	return result;
}

} // end of namespace

