//******************************************************************************
//
// MIDITrail / MIDITrailMain
//
// アプリケーションエントリポイント
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MIDITrailApp.h"
#include "MIDITrailMain.h"

using namespace YNBaseLib;


//******************************************************************************
// エントリポイント
//******************************************************************************
int APIENTRY _tWinMain(
		HINSTANCE hInstance,		//インスタンスハンドル
		HINSTANCE hPrevInstance,	//以前のインスタンスハンドル：常にNULL
		LPTSTR lpCmdLine,			//コマンドライン
		int nCmdShow				//ウィンドウ表示状態指定
	)
{
	int result = 0;
	int winMainResult = 0;
	MIDITrailApp app;

	//未参照警告回避
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//アプリケーション初期化
	//メッセージループに入る前に終了する場合は戻り値を0とする
	result = app.Initialize(hInstance, lpCmdLine, nCmdShow);
	if (result != 0) {
		YN_SHOW_ERR(NULL);
		winMainResult = 0;
		goto EXIT;
	}

	//アプリケーション実行
	//RunはWinMainの戻り値となる値を返す
	winMainResult = app.Run();

EXIT:;
	app.Terminate();
	return winMainResult;
}


