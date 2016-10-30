//******************************************************************************
//
// MIDITrail / MTAboutDlg
//
// バージョン情報ダイアログクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "resource.h"


//******************************************************************************
// バージョン情報ダイアログクラス
//******************************************************************************
class MTAboutDlg
{
public:

	//コンストラクタ／デストラクタ
	MTAboutDlg(void);
	virtual ~MTAboutDlg(void);

	//表示
	int Show(HWND hParentWnd);

private:

	//ウィンドウプロシージャ制御用ポインタ
	static MTAboutDlg* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

};


