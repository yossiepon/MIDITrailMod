//******************************************************************************
//
// MIDITrail / MTDlgLib
//
// ダイアログライブラリクラス
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// ダイアログライブラリクラス
//******************************************************************************
class MTDlgLib
{
public:

	//ダイアログ位置を親ウィンドウの中央に移動
	static int SetWindowPositionToCenter(HWND hWnd, bool isInsideScreen);

	//ダイアログ位置をスクリーン内側に移動
	static int SetWindowPositionToInsideScreen(HWND hWnd);

};

