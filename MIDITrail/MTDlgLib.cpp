//******************************************************************************
//
// MIDITrail / MTDlgLib
//
// ダイアログライブラリクラス
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTDlgLib.h"

using namespace YNBaseLib;


//******************************************************************************
// ダイアログ位置を親ウィンドウの中央に移動
//******************************************************************************
int MTDlgLib::SetWindowPositionToCenter(
		HWND hWnd,
		bool isInsideScreen
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	HWND hWndOwner = NULL;
	RECT ownerRect;
	RECT dlgRect;
	int ownerWidth = 0;
	int ownerHeight = 0;
	int dlgWidth = 0;
	int dlgHeight = 0;
	int posX = 0;
	int posY = 0;

	//親ウィンドウのハンドルを取得
	hWndOwner = GetParent(hWnd);
	if (hWndOwner == NULL) {
		hWndOwner = GetDesktopWindow();
	}

	//親ウィンドウと対象ダイアログの座標を取得
	GetWindowRect(hWndOwner, &ownerRect);
	GetWindowRect(hWnd, &dlgRect);

	//親ウィンドウと対象ダイアログのサイズ
	ownerWidth  = ownerRect.right - ownerRect.left;
	ownerHeight = ownerRect.bottom - ownerRect.top;
	dlgWidth    = dlgRect.right - dlgRect.left;
	dlgHeight   = dlgRect.bottom - dlgRect.top;

	//対象ダイアログの座標計算
	posX = ownerRect.left + ((ownerWidth - dlgWidth) / 2);
	posY = ownerRect.top + ((ownerHeight - dlgHeight) / 2);

	//ダイアログ位置変更
	bresult = SetWindowPos(
					hWnd,		//ウィンドウハンドル
					NULL,		//配置順序：SWP_NOZORDER指定により無視される
					posX,		//横方向の位置
					posY,		//縦方向の位置
					0,			//幅  ：SWP_NOSIZE指定により無視される
					0,			//高さ：SWP_NOSIZE指定により無視される
					SWP_NOSIZE | SWP_NOZORDER	//ウィンドウ位置指定
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//スクリーン内側に移動
	if (isInsideScreen) {
		result = SetWindowPositionToInsideScreen(hWnd);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ダイアログ位置をスクリーン内側に移動
//******************************************************************************
int MTDlgLib::SetWindowPositionToInsideScreen(HWND hWnd)
{
	int result = 0;
	BOOL bresult = FALSE;
	RECT dlgRect;
	RECT workAreaRect;
	HMONITOR hMonitor = NULL;
	MONITORINFO mi;
	int dlgWidth = 0;
	int dlgHeight = 0;
	int posX = 0;
	int posY = 0;

	//対象ダイアログの座標とサイズを取得
	GetWindowRect(hWnd, &dlgRect);
	dlgWidth  = dlgRect.right - dlgRect.left;
	dlgHeight = dlgRect.bottom - dlgRect.top;

	//最も近いモニタのハンドルを取得
	hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	if (hMonitor == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//モニタの作業領域を取得
	mi.cbSize = sizeof(MONITORINFO);
	bresult = GetMonitorInfo(hMonitor, &mi);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	workAreaRect = mi.rcWork;

	//左右のはみ出しを修正
	posX = dlgRect.left;
	if (posX < workAreaRect.left) {
		posX = workAreaRect.left;
	}
	if (workAreaRect.right < (posX + dlgWidth)) {
		posX = workAreaRect.right - dlgWidth;
	}

	//上下のはみ出しを修正
	posY = dlgRect.top;
	if (posY < workAreaRect.top) {
		posY = workAreaRect.top;
	}
	if (workAreaRect.bottom < (posY + dlgHeight)) {
		posY = workAreaRect.bottom - dlgHeight;
	}

	//ダイアログ位置変更
	bresult = SetWindowPos(
					hWnd,		//ウィンドウハンドル
					NULL,		//配置順序：SWP_NOZORDER指定により無視される
					posX,		//横方向の位置
					posY,		//縦方向の位置
					0,			//幅  ：SWP_NOSIZE指定により無視される
					0,			//高さ：SWP_NOSIZE指定により無視される
					SWP_NOSIZE | SWP_NOZORDER	//ウィンドウ位置指定
				);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;

}



