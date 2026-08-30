//******************************************************************************
//
// MIDITrail / MTParam.h
//
// パラメータ定義ファイル
//
// Copyright (C) 2010-2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

//******************************************************************************
// パラメータ定義
//******************************************************************************
//演奏状態数
#define MT_PLAYSTATUS_NUM  (6)


//シーン種別
//TAG:シーン追加
enum SceneType {
	Title,				//タイトル
	PianoRoll3D,		//ピアノロール3D
	PianoRoll2D,		//ピアノロール2D
	PianoRollRain,		//ピアノロールレイン
	PianoRollRain2D,	//ピアノロールレイン2D
	PianoRollRing		//ピアノロールリング
};

//演奏状態
enum PlayStatus {
	NoData,			//データなし
	Stop,			//停止状態
	Play,			//再生中
	Pause,			//一時停止
	MonitorOFF,		//モニタ停止
	MonitorON		//モニタ中
};


//******************************************************************************
// ファイルパス定義
//******************************************************************************

//ユーザ設定保存ファイル：APPDATAフォルダからの相対パス
#define MT_USER_CONFFILE_DIR         _T("yknk\\MIDITrail\\")
#define MT_USER_CONFFILE_PLAYER      _T("Player.ini")
#define MT_USER_CONFFILE_VIEW        _T("View.ini")
#define MT_USER_CONFFILE_WINDOW      _T("Window.ini")
#define MT_USER_CONFFILE_MIDI        _T("MIDI.ini")
#define MT_USER_CONFFILE_SYNTHESIZER _T("Synthesizer.ini")
#define MT_USER_CONFFILE_GRAPHIC     _T("Graphic.ini")
#define MT_USER_CONFFILE_COLOR       _T("Color.ini")

//画像ファイル：実行ファイルからの相対パス
#define MT_IMGFILE_RIPPLE      _T("data\\Ripple.png")
#define MT_IMGFILE_BOARD       _T("data\\Board.png")
#define MT_IMGFILE_KEYBOARD    _T("data\\Keyboard.png")
#define MT_IMGFILE_HOWTOVIEW1  _T("data\\HowToView1.bmp")
#define MT_IMGFILE_HOWTOVIEW2  _T("data\\HowToView2.bmp")
#define MT_IMGFILE_HOWTOVIEW3  _T("data\\HowToView3.bmp")

//ノートデザインファイル：実行ファイルからの相対パス
#define MT_CONFFILE_DIR  _T("conf\\")

//ウェーブテーブルファイル格納ディレクトリ：実行ファイルからの相対パス
#define MT_WAVEFILE_DIR					L"wave\\"

//マニュアルファイル：実行ファイルからの相対パス
#define MT_MANUALFILE 					_T("doc\\index.html")
#define MT_MANUAL_EN					_T("doc/MANUAL.en.html")
#define MT_MANUAL_JA					_T("doc/MANUAL.ja.html")
#define MT_ACKNOWLEDGEMENTS_EN			_T("doc/MANUAL_Acknowledgements.en.html")
#define MT_ACKNOWLEDGEMENTS_JA			_T("doc/MANUAL_Acknowledgements.ja.html")

//パネル画像ディレクトリ：実行ファイルからの相対パス
#define MT_IMG_PANEL_DIR					L"img\\panel\\"

//ビューモード画像ディレクトリ：実行ファイルからの相対パス
#define MT_IMG_VIEWMODE_DIR					L"img\\view\\"

//ウェーブテーブルデフォルトファイル
#define MT_WAVETABLE_DEFAULT_FILE_NAME	L"GeneralUser-GS.sf2"
#define MT_WAVETABLE_DEFAULT_FILE_VER	L"v2.0.3"

