//******************************************************************************
//
// MIDITrail / MTMIDIINCfgDlg
//
// MIDI IN 設定ダイアログ
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"
#include "SMIDILib.h"

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// MIDI IN 設定ダイアログクラス
//******************************************************************************
class MTMIDIINCfgDlg
{
public:

	//コンストラクタ／デストラクタ
	MTMIDIINCfgDlg(void);
	virtual ~MTMIDIINCfgDlg(void);

	//表示：ダイアログが閉じられるまで制御を返さない
	int Show(HWND hParentWnd);

private:

	//ウィンドウプロシージャ制御用ポインタ
	static MTMIDIINCfgDlg* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//設定ファイル
	YNConfFile m_ConfFile;

	//MIDI入力デバイス制御オブジェクト
	SMInDevCtrl m_MIDIInDevCtrl;

	//コンボボックスのウィンドウハンドル
	HWND m_hComboDevA;

	//MIDITHRUチェックボックスのウィンドウハンドル
	HWND m_hMIDITHRU;

	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ダイアログ表示直前初期化
	int _OnInitDlg(HWND hDlg);

	//設定ファイル初期化
	int _InitConfFile();

	//デバイス選択コンボボックス初期化
	int _InitComboDev(HWND hComboDev, TCHAR* pPortName);

	//MIDITHRUチェックボタン初期化
	int _InitCheckBtnMIDITHRU();

	//保存処理
	int _Save();
	int _SavePortCfg(HWND hComboDev, TCHAR* pPortName);
	int _SaveMIDITHRU();

};


