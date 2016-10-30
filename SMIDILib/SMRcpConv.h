//******************************************************************************
//
// Simple MIDI Library / SMRcpConv
//
// RCPファイル変換クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ふみぃ氏が公開している RCPCV.DLL(*1) を用いて、レコンポーザの
// データファイル(*.rcp,*.r36,*.g36)を標準MIDIファイルに変換する。
// RCPCV.DLL がアプリケーションと同じフォルダに存在するときに利用可能とする。
// RCPCV.DLL が存在しなければ本クラスの機能は利用できない。
// 事前にIsAvailable()メソッドを用いて利用可否を確認すること。
//
// (*1) RCPCV.DLL
// http://www.vector.co.jp/soft/win95/art/se114143.html

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include <windows.h>

namespace SMIDILib {

//******************************************************************************
// SysExイベントクラス
//******************************************************************************
class SMIDILIB_API SMRcpConv
{
public:

	//コンストラクタ／デストラクタ
	SMRcpConv();
	virtual ~SMRcpConv(void);

	//初期化
	int Initialize();

	//利用可否判定
	bool IsAvailable();

	//標準MIDファイル変換
	int Convert(const TCHAR* pRCPPath, const TCHAR* pSMFPath);

	//拡張子によるサポート対象ファイル判定
	bool IsSupportFileExt(const TCHAR* pFilePath);

	//GetOpenFileName用ファイルフィルタ取得
	const TCHAR* GetOpenFileNameFilter();

private:

	HMODULE m_hModule;

	//RCPCV.DLL API定義
	typedef DWORD (WINAPI *RCPCV_ConvertFile)(LPCSTR, UINT, DWORD, UINT, DWORD);
	typedef int   (WINAPI *RCPCV_SaveSMF)(DWORD, LPCSTR);
	typedef void  (WINAPI *RCPCV_DeleteObject)(DWORD);

	//関数ポインタ
	RCPCV_ConvertFile  m_pFuncConvertFile;
	RCPCV_SaveSMF      m_pFuncSaveSMF;
	RCPCV_DeleteObject m_pFuncDeleteObject;

	void _Release();

};

} // end of namespace

