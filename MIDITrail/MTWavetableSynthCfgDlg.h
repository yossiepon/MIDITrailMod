//******************************************************************************
//
// MIDITrail / MTWavetableSynthCfgDlg
//
// ウェーブテーブルシンセサイザ設定ダイアログクラス
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"

using namespace YNBaseLib;


//******************************************************************************
// パラメータ定義
//******************************************************************************


//******************************************************************************
// グラフィック設定ダイアログクラス
//******************************************************************************
class MTWavetableSynthCfgDlg
{
public:

	//コンストラクタ／デストラクタ
	MTWavetableSynthCfgDlg(void);
	virtual ~MTWavetableSynthCfgDlg(void);

	//表示：ダイアログが閉じられるまで制御を返さない
	int Show(HWND hParentWnd);

	//パラメータ変更確認
	bool IsChanged();

private:

	//ウィンドウプロシージャ制御用ポインタ
	static MTWavetableSynthCfgDlg* m_pThis;

	//ウィンドウハンドル
	HWND m_hWnd;

	//設定ファイル
	YNConfFile m_ConfFile;

	//ウェーブテーブルファイルパス
	int m_SelectedWavetableFileIndex;
	WCHAR m_UserWavetableFilePath[_MAX_PATH];

	//コンボボックス：ウェーブテーブルファイル
	HWND m_hComboWavetableFile;

	//コンボボックス：Max Voices
	HWND m_hComboMaxVoices;

	//コンボボックス：Sustain
	HWND m_hComboSustain;

	//更新フラグ
	bool m_isChanged;

private:

	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ダイアログ表示直前初期化
	int _OnInitDlg(HWND hDlg);

	//設定ファイル初期化
	int _InitConfFile();

	//コンボボックス初期化：ウェーブテーブルファイル
	int _InitComboWavetableFile();

	//コンボボックス初期化：Max Voices
	int _InitComboMaxVoices();

	//コンボボックス初期化：Sustain
	int _InitComboSustain();

	//ウェーブテーブルファイルリスト更新
	int _UpdateWavetableFileList();

	//Selectボタン押下
	int _OnBtnSelectFile();

	//Clearボタン押下
	int _OnBtnClearFile();

	//Acknowledgementsボタン押下
	int _OnBtnAcknowledgements();

	//設定保存
	int _Save();

	//設定保存：ウェーブテーブルファイル
	int _SaveWavetableFile();

	//設定保存：最大同時発音数
	int _SaveMaxVoices();

	//設定保存：サスティン
	int _SaveSustain();

};


