//******************************************************************************
//
// MIDITrail / MTViewModeDlg
//
// ビューモード選択ダイアログクラス
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"

using namespace YNBaseLib;


//******************************************************************************
// ビューモード選択ダイアログクラス
//******************************************************************************
class MTViewModeDlg
{
public:

	//コンストラクタ／デストラクタ
	MTViewModeDlg(void);
	virtual ~MTViewModeDlg(void);

	//シーン種別登録
	void SetSceneType(SceneType type);

	//シーン種別取得
	SceneType GetSceneType();

	//キャンセルフラグ参照
	bool IsCanceled();

	//表示：ダイアログが閉じられるまで制御を返さない
	int Show(HWND hParentWnd);

private:

	//----------------------------------------------------------------
	// メンバ定義
	//----------------------------------------------------------------
	//ウィンドウプロシージャ制御用ポインタ
	static MTViewModeDlg* m_pThis;

	//ウィンドウハンドル
	HWND m_hWnd;

	//シーン種別
	SceneType m_SceneType;

	//キャンセルフラグ
	bool m_isCanceled;

	//チェックマーク領域ハンドル
	HWND m_hCheckPianoRoll3D;
	HWND m_hCheckPianoRoll2D;
	HWND m_hCheckPianoRollRain;
	HWND m_hCheckPianoRollRain2D;
	HWND m_hCheckPianoRollRing;

	//ビューモード画像ボタンハンドル
	HWND m_hBtnImgPianoRoll3D;
	HWND m_hBtnImgPianoRoll2D;
	HWND m_hBtnImgPianoRollRain;
	HWND m_hBtnImgPianoRollRain2D;
	HWND m_hBtnImgPianoRollRing;

	//チェックマーク画像ハンドル
	HBITMAP m_hImgCheckmark;
	HBITMAP m_hImgNoCheckmark;

	//ボタン画像ハンドル
	HBITMAP m_hImgPianoRoll3D;
	HBITMAP m_hImgPianoRoll2D;
	HBITMAP m_hImgPianoRollRain;
	HBITMAP m_hImgPianoRollRain2D;
	HBITMAP m_hImgPianoRollRing;

	//----------------------------------------------------------------
	// メソッド定義
	//----------------------------------------------------------------
	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ダイアログ表示直前初期化
	int _OnInitDlg(HWND hDlg);

	//ボタン群初期化
	int _InitButtons();
	int _CreateButtonImage(HWND hButton, WCHAR* pImgFileName, HBITMAP* phBitmap);
	int _GetWindowSize(HWND hWnd, unsigned long* pWidth, unsigned long* pHeight);
	int _LoadImage(void** pPtrBitmap, WCHAR* pFilePath);
	void _ReleaseBitmap(HBITMAP hBitmap);
	void _UpdateCheckMarks();
	void _SetCheckMark(HWND hWnd, bool isCheck);

	//ボタン押下イベントハンドラ
	int _OnButtonPianoRoll3D();
	int _OnButtonPianoRoll2D();
	int _OnButtonPianoRollRain();
	int _OnButtonPianoRollRain2D();
	int _OnButtonPianoRollRing();
	int _OnOK();
	int _OnCancel();

};


