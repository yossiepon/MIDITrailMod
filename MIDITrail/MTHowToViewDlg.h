//******************************************************************************
//
// MIDITrail / MTHowToViewDlg
//
// 操作方法ダイアログ
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


//******************************************************************************
// 操作方法ダイアログ
//******************************************************************************
class MTHowToViewDlg
{
public:

	//コンストラクタ／デストラクタ
	MTHowToViewDlg(void);
	virtual ~MTHowToViewDlg(void);

	//表示：ダイアログが閉じられるまで制御を返さない
	int Show(HWND hParentWnd);

private:

	BITMAPFILEHEADER m_BmpHead;
	BITMAPINFOHEADER m_BmpInfo;
	HANDLE m_hMemBmpPixel;
	BYTE* m_pBmpPixcel;
	unsigned long m_PageNo;
	HWND m_hWnd;

	//ウィンドウプロシージャ制御用ポインタ
	static MTHowToViewDlg* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ダイアログ表示直前初期化
	int _OnInitDlg(HWND hDlg);

	//HowToビットマップ読み込み
	int _LoadHowToBmp();

	//HowToビットマップ描画
	int _DrawHowToBmp();

	//クリア
	void _Clear();

	//Previousボタン
	int _OnPreviousButton();

	//Nextボタン
	int _OnNextButton();

	//画像表示
	int _DrawHowToImage();

	//ボタン状態更新
	void _UpdateButtonStatus();

};


