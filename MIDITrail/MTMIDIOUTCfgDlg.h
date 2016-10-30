//******************************************************************************
//
// MIDITrail / MTMIDIOUTCfgDlg
//
// MIDI OUT 設定ダイアログクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"
#include "SMIDILib.h"

using namespace YNBaseLib;
using namespace SMIDILib;


//******************************************************************************
// MIDI OUT 設定ダイアログクラス
//******************************************************************************
class MTMIDIOUTCfgDlg
{
public:

	//コンストラクタ／デストラクタ
	MTMIDIOUTCfgDlg(void);
	virtual ~MTMIDIOUTCfgDlg(void);

	//表示：ダイアログが閉じられるまで制御を返さない
	int Show(HWND hParentWnd);

private:

	//ウィンドウプロシージャ制御用ポインタ
	static MTMIDIOUTCfgDlg* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//設定ファイル
	YNConfFile m_ConfFile;

	//MIDI出力デバイス制御オブジェクト
	SMOutDevCtrl m_MIDIOutDevCtrl;

	//コンボボックスのウィンドウハンドル
	HWND m_hComboDevA;
	HWND m_hComboDevB;
	HWND m_hComboDevC;
	HWND m_hComboDevD;
	HWND m_hComboDevE;
	HWND m_hComboDevF;

	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ダイアログ表示直前初期化
	int _OnInitDlg(HWND hDlg);

	//設定ファイル初期化
	int _InitConfFile();

	//デバイス選択コンボボックス初期化
	int _InitComboDev(HWND hComboDev, TCHAR* pPortName);

	//保存処理
	int _Save();
	int _SavePortCfg(HWND hComboDev, TCHAR* pPortName);

};


