//******************************************************************************
//
// MIDITrail / MTWindowSizeCfgDlg
//
// ウィンドウサイズ設定ダイアログクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "resource.h"
#include "YNBaseLib.h"
#include <list>

using namespace YNBaseLib;


//******************************************************************************
//ウィンドウサイズ設定ダイアログクラス
//******************************************************************************
class MTWindowSizeCfgDlg
{
public:

	//コンストラクタ／デストラクタ
	MTWindowSizeCfgDlg(void);
	virtual ~MTWindowSizeCfgDlg(void);

	//表示：ダイアログが閉じられるまで制御を返さない
	int Show(HWND hParentWnd);

	//変更確認
	bool IsCahnged();

private:

	typedef struct {
		unsigned long width;
		unsigned long height;
	} MTWindowSizeItem;

	typedef std::list<MTWindowSizeItem> MTWindowSizeList;

private:

	//ウィンドウプロシージャ制御用ポインタ
	static MTWindowSizeCfgDlg* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//ウィンドウサイズ選択リストボックスウィンドウハンドル
	HWND m_hSizeList;

	//ウィンドウサイズリスト
	MTWindowSizeList m_SizeList;

	//設定ファイル
	YNConfFile m_ConfFile;

	//保存実施フラグ
	bool m_isSaved;

	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ダイアログ表示直前初期化
	int _OnInitDlg(HWND hDlg);

	//設定ファイル初期化
	int _InitConfFile();

	//ウィンドウサイズ選択コンボボックス初期化
	int _InitSizeList();

	//保存処理
	int _Save();

};

