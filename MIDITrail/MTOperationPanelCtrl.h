//******************************************************************************
//
// MIDITrail / MTOperationPanelCtrl
//
// 操作パネル制御
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"
#include "MTParam.h"

using namespace YNBaseLib;


//******************************************************************************
// パラメータ定義
//******************************************************************************

//ボタン画像セット
typedef struct {
	HBITMAP hImgN;		//通常
	HBITMAP hImgD;		//無効
	HBITMAP hImgS;		//選択状態
} MTOperationPanelButtonImage;

typedef struct {
	bool isEnabled;			//活性状態
	bool isSwitchStatus;	//スイッチステータス
} MTOperationPanelButtonStatus;


//******************************************************************************
// 操作パネル制御クラス
//******************************************************************************
class MTOperationPanelCtrl
{
private:

	//パネルボタンクラス
	class MTPanelButton
	{
	public:
		MTPanelButton(void);
		virtual ~MTPanelButton(void);

		//初期化
		void Initialize(HWND hWnd, int buttonId);
		
		//活性状態取得/登録
		void SetEnabled(bool isEnabled);
		bool IsEnabled();
		
		//スイッチ状態取得/登録
		void SetSwitchStatus(bool isON);
		bool IsSwitchStatus();
		
		//ボタンサイズ取得
		int GetSize(unsigned long* pWidth, unsigned long* pHeight);
		
		//画像登録
		void SetImage(HBITMAP);
		
		//ツールチップ生成
		int CreateToolTip(LPTSTR pszText);
		
	private:
		
		HWND m_hPanel;
		HWND m_hButton;
		HWND m_hToolTip;

		bool m_isEnabled;
		bool m_isSwitchStatus;
	};

public:

	//コンストラクタ／デストラクタ
	MTOperationPanelCtrl(void);
	virtual ~MTOperationPanelCtrl(void);

	//初期化
	int Initialize(HWND hMainWnd, bool isDisplay);

	//ウィンドウハンドル取得
	HWND GetWindowHandle();

	//表示状態設定
	void SetDisplay(bool isDisplay);

	//表示状態取得
	bool IsDisplay();

	//演奏状態設定
	void SetPlayStatus(PlayStatus status);

	//ボタン活性状態設定
	void SetEnabledForMenu(int menuId, bool isEnabled);

	//マーク設定
	void SetMenuMark(int menuId, bool isON);

	//フルスクリーン状態設定
	void SetFullScreenStatus(bool isFullScreen);

	//パネル位置更新
	int UpdatePanelPosition();

private:

	//----------------------------------------------------------------
	//メンバ定義
	//----------------------------------------------------------------
	//ウィンドウプロシージャ制御用ポインタ
	static MTOperationPanelCtrl* m_pThis;

	//アプリケーションインスタンス
	HINSTANCE m_hInstance;

	//ウィンドウ系
	HWND m_hMainWnd;
	HWND m_hWnd;
	HBRUSH m_hBackgroundBrush;

	//演奏状態
	PlayStatus m_PlayStatus;

	//パネル表示フラグ
	bool m_isDisplay;

	//フルスクリーンフラグ
	bool m_isFullScreen;

	//ボタンオブジェクト
	MTPanelButton m_ButtonOpenFile;
	MTPanelButton m_ButtonOpenFolder;
	MTPanelButton m_ButtonPreviousFile;
	MTPanelButton m_ButtonNextFile;
	MTPanelButton m_ButtonFolderPlayback;
	MTPanelButton m_ButtonRepeat;
	MTPanelButton m_ButtonPlay;
	MTPanelButton m_ButtonStop;
	MTPanelButton m_ButtonSkipBack;
	MTPanelButton m_ButtonSkipForward;
	MTPanelButton m_ButtonPlaySpeedDown;
	MTPanelButton m_ButtonPlaySpeedUp;
	MTPanelButton m_ButtonViewpoint1;
	MTPanelButton m_ButtonViewpoint2;
	MTPanelButton m_ButtonViewpoint3;
	MTPanelButton m_ButtonMyViewpoint1;
	MTPanelButton m_ButtonMyViewpoint2;
	MTPanelButton m_ButtonMyViewpoint3;
	MTPanelButton m_ButtonViewMode;
	MTPanelButton m_ButtonMIDIOUT;

	//ボタン画像
	HBITMAP m_hImgButtonOpenFileN;
	HBITMAP m_hImgButtonOpenFileD;
	HBITMAP m_hImgButtonOpenFolderN;
	HBITMAP m_hImgButtonOpenFolderD;
	HBITMAP m_hImgButtonPreviousFileN;
	HBITMAP m_hImgButtonPreviousFileD;
	HBITMAP m_hImgButtonNextFileN;
	HBITMAP m_hImgButtonNextFileD;
	HBITMAP m_hImgButtonFolderPlaybackN;
	HBITMAP m_hImgButtonFolderPlaybackD;
	HBITMAP m_hImgButtonFolderPlaybackS;
	HBITMAP m_hImgButtonRepeatN;
	HBITMAP m_hImgButtonRepeatD;
	HBITMAP m_hImgButtonRepeatS;
	HBITMAP m_hImgButtonPlayN;
	HBITMAP m_hImgButtonPlayD;
	HBITMAP m_hImgButtonPauseN;
	HBITMAP m_hImgButtonPauseD;
	HBITMAP m_hImgButtonStopN;
	HBITMAP m_hImgButtonStopD;
	HBITMAP m_hImgButtonSkipBackN;
	HBITMAP m_hImgButtonSkipBackD;
	HBITMAP m_hImgButtonSkipForwardN;
	HBITMAP m_hImgButtonSkipForwardD;
	HBITMAP m_hImgButtonSpeedDownN;
	HBITMAP m_hImgButtonSpeedDownD;
	HBITMAP m_hImgButtonSpeedUpN;
	HBITMAP m_hImgButtonSpeedUpD;
	HBITMAP m_hImgButtonViewpoint1N;
	HBITMAP m_hImgButtonViewpoint1D;
	HBITMAP m_hImgButtonViewpoint2N;
	HBITMAP m_hImgButtonViewpoint2D;
	HBITMAP m_hImgButtonViewpoint3N;
	HBITMAP m_hImgButtonViewpoint3D;
	HBITMAP m_hImgButtonMyViewpoint1N;
	HBITMAP m_hImgButtonMyViewpoint1D;
	HBITMAP m_hImgButtonMyViewpoint2N;
	HBITMAP m_hImgButtonMyViewpoint2D;
	HBITMAP m_hImgButtonMyViewpoint3N;
	HBITMAP m_hImgButtonMyViewpoint3D;
	HBITMAP m_hImgButtonViewModeN;
	HBITMAP m_hImgButtonViewModeD;
	HBITMAP m_hImgButtonMIDIOUTN;
	HBITMAP m_hImgButtonMIDIOUTD;

	//----------------------------------------------------------------
	//メソッド定義
	//----------------------------------------------------------------
	//ウィンドウプロシージャ
	static INT_PTR CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	INT_PTR _WndProcImpl(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam);

	//ボタン群初期化
	int _InitButtons();
	int _InitButtonObjects();
	int _InitButtonImages();
	int _InitButtonToolTips();
	int _CreateButtonImage(MTPanelButton* pButton, WCHAR* pImgFileName, HBITMAP* phBitmap);
	int _LoadImage(void** pPtrBitmap, WCHAR* pFilePath);
	void _ReleaseBitmap(HBITMAP hBitmap);
	MTPanelButton* _GetButtonObjectForMenu(int menuId);
	MTOperationPanelButtonImage _GetButtonImageForMenu(int menuId);

	//ボタンイベント処理
	int _OnBtnOpenFile();
	int _OnBtnOpenFolder();
	int _OnBtnPreviousFile();
	int _OnBtnNextFile();
	int _OnBtnFolderPlayback();
	int _OnBtnRepeat();
	int _OnBtnPlay();
	int _OnBtnStop();
	int _OnBntSkipBack();
	int _OnBtnSkipForward();
	int _OnBtnSpeedDown();
	int _OnBtnSpeedUp();
	int _OnBtnViewPoint1();
	int _OnBtnViewPoint2();
	int _OnBtnViewPoint3();
	int _OnBtnMyViewPoint1();
	int _OnBtnMyViewPoint2();
	int _OnBtnMyViewPoint3();
	int _OnBtnViewMode();
	int _OnBtnMIDIOUT();
};

