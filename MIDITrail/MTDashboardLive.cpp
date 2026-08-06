//******************************************************************************
//
// MIDITrail / MTDashboardLive
//
// ライブモニタ用ダッシュボード描画クラス
//
// Copyright (C) 2012-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTColorConf.h"
#include "MTDashboardLive.h"
#include <mbctype.h>

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTDashboardLive::MTDashboardLive(void)
{
	m_hWnd = NULL;
	m_PosCounterX = 0.0f;
	m_PosCounterY = 0.0f;
	m_CounterMag = MTDASHBOARDLIVE_DEFAULT_MAGRATE;
	m_isMonitoring = false;
	m_NoteCount = 0;
	m_CaptionColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_isEnable = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTDashboardLive::~MTDashboardLive(void)
{
	Release();
}

//******************************************************************************
// ダッシュボード生成
//******************************************************************************
int MTDashboardLive::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		HWND hWnd
   )
{
	int result = 0;
	WCHAR counter[100];
	
	Release();
	
	m_hWnd = hWnd;
	
	//設定読み込み
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;
	
	//タイトルキャプション
	result = SetMIDIINDeviceName(pD3DDevice, _T(""));
	if (result != 0) goto EXIT;
	
	//カウンタキャプション
	result = m_Counter.Create(
					pD3DDevice,
					MTDASHBOARDLIVE_FONTNAME,		//フォント名称
					MTDASHBOARDLIVE_FONTSIZE,		//フォントサイズ
					MTDASHBOARDLIVE_COUNTER_CHARS,	//表示文字
					MTDASHBOARDLIVE_COUNTER_SIZE	//キャプションサイズ
				);
	if (result != 0) goto EXIT;
	m_Counter.SetColor(m_CaptionColor);
	
	//カウンタ表示文字列生成
	result = _GetCounterStr(counter, 100);
	if (result != 0) goto EXIT;
	
	result = m_Counter.SetString(counter);
	if (result != 0) goto EXIT;
	
	//カウンタ表示位置を算出
	result = _GetCounterPos(&m_PosCounterX, &m_PosCounterY);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTDashboardLive::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector
	)
{
	int result = 0;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTDashboardLive::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	WCHAR counter[100];
	
	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	if (!m_isEnable) goto EXIT;
	
	//タイトル描画：カウンタと同じ拡大率で表示する
	result = m_Title.Draw(pD3DDevice, MTDASHBOARDLIVE_FRAMESIZE, MTDASHBOARDLIVE_FRAMESIZE, m_CounterMag);
	if (result != 0) goto EXIT;
	
	//カウンタ文字列描画
	result = _GetCounterStr(counter, 100);
	if (result != 0) goto EXIT;
	
	result = m_Counter.SetString(counter);
	if (result != 0) goto EXIT;
	
	result = m_Counter.Draw(pD3DDevice, m_PosCounterX, m_PosCounterY, m_CounterMag);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTDashboardLive::Release()
{
	m_Title.Release();
	m_Counter.Release();
}

//******************************************************************************
// カウンタ表示位置取得
//******************************************************************************
int MTDashboardLive::_GetCounterPos(
		float* pX,
		float* pY
	)
{
	int result = 0;
	BOOL bresult = 0;
	RECT rect;
	unsigned long cw = 0;
	unsigned long ch = 0;
	unsigned long tw = 0;
	unsigned long th = 0;
	unsigned long charHeight = 0;
	unsigned long charWidth = 0;
	unsigned long captionWidth = 0;
	float newMag = 0.0f;
	
	//クライアント領域のサイズを取得
	bresult = GetClientRect(m_hWnd, &rect);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	cw = rect.right - rect.left;
	ch = rect.bottom - rect.top;
	
	//テクスチャサイズ取得
	m_Counter.GetTextureSize(&th, &tw);
	
	//文字サイズ
	charHeight = th;
	charWidth = tw / (unsigned long)wcslen(MTDASHBOARDLIVE_COUNTER_CHARS);
	
	//拡大率1.0のキャプションサイズ
	captionWidth = (unsigned long)(charWidth * MTDASHBOARDLIVE_COUNTER_SIZE);
	
	//カウンタ文字列が画面からはみ出す場合は画面に収まるように拡大率を更新する
	//  タイトルがはみ出すのは気にしないことにする
	if (((cw - (MTDASHBOARDLIVE_FRAMESIZE*2)) < captionWidth) && (tw > 0)) {
		newMag = (float)(cw - (MTDASHBOARDLIVE_FRAMESIZE*2)) / (float)captionWidth;
		if (m_CounterMag > newMag) {
			m_CounterMag = newMag;
		}
	}
	
	//テクスチャの表示倍率を考慮して表示位置を算出
	*pX = MTDASHBOARDLIVE_FRAMESIZE;
	*pY = (float)ch - ((float)th * m_CounterMag) - MTDASHBOARDLIVE_FRAMESIZE;

EXIT:;
	return result;
}

//******************************************************************************
// モニタ状態登録
//******************************************************************************
void MTDashboardLive::SetMonitoringStatus(
		bool isMonitoring
	)
{
	m_isMonitoring = isMonitoring;
}

//******************************************************************************
// ノートON登録
//******************************************************************************
void MTDashboardLive::SetNoteOn()
{
	m_NoteCount++;
}

//******************************************************************************
// カウンタ文字列取得
//******************************************************************************
int MTDashboardLive::_GetCounterStr(
		WCHAR* pStr,
		unsigned long bufSize
	)
{
	int result = 0;
	int eresult = 0;
	const WCHAR* pMonitorStatus = L"";
	
	if (m_isMonitoring) {
		pMonitorStatus = L"";
	}
	else {
		pMonitorStatus = L"[MONITERING OFF]";
	}
	
	eresult = swprintf_s(
				pStr,
				bufSize,
				L"NOTES:%08lu %s",
				m_NoteCount,
				pMonitorStatus
			);
	if (eresult < 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTDashboardLive::Reset()
{
	m_isMonitoring = false;
	m_NoteCount = 0;
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTDashboardLive::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	MTColorConf colorConf;
	MTColorPalette colorPalette;
	D3DXCOLOR color;
	
	//カラー設定初期化
	result = colorConf.Initialize(pSceneName);
	if (result != 0) goto EXIT;
	
	//選択カラーパレットからカウンター色取得
	colorConf.GetSelectedColorPalette(&colorPalette);
	colorPalette.GetCounterColor(&color);
	m_CaptionColor = color;
	
EXIT:;
	return result;
}

//******************************************************************************
// 表示設定
//******************************************************************************
void MTDashboardLive::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}

// メモ
// SMLib側でデバイス名をワイド文字列で返却すべきであるが、
// INIファイルへのパラメータ保存など広範囲に影響が及ぶため、
// 本クラスでワイド文字列変換をして吸収する。
//******************************************************************************
//MIDI IN デバイス名登録
//******************************************************************************
int MTDashboardLive::SetMIDIINDeviceName(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pName
	)
{
	int result = 0;
	int eresult = 0;
	WCHAR title[256] = {0}; //MAXPNAMELEN 32 より大きいサイズにする
	const WCHAR* pDisplayName = NULL;
	std::string deviceName;
	std::wstring deviceNameW;
	
	m_Title.Release();
	
	//ワイド文字列変換
	if (pName != NULL) {
		deviceName = pName;
		result = _StringToWstring(&deviceName, &deviceNameW);
		if (result != 0) goto EXIT;
	}

	//表示名設定
	if (pName == NULL) {
		pDisplayName = L"(none)";
	}
	else if (_tcslen(pName) == 0) {
		pDisplayName = L"(none)";
	}
	else {
		pDisplayName = deviceNameW.c_str();
	}
	
	//タイトルキャプション
	eresult = swprintf_s(
				title,
				256,
				L"MIDI IN: %s",
				pDisplayName
			);

	result = m_Title.Create(
					pD3DDevice,					//デバイス
					MTDASHBOARDLIVE_FONTNAME,	//フォント名称
					MTDASHBOARDLIVE_FONTSIZE,	//フォントサイズ
					title						//キャプション
				);
	if (result != 0) goto EXIT;
	m_Title.SetColor(m_CaptionColor);

EXIT:;
	return result;
}

//******************************************************************************
// ワイド文字列変換
//******************************************************************************
int MTDashboardLive::_StringToWstring(std::string* pStr, std::wstring* pWstr)
{
	int result = 0;
	int apiresult = 0;
	int buffSize = 0;
	WCHAR* wstrBuff = NULL;

	//空文字の場合は変換なし
	if (pStr->length() == 0) {
		*pWstr = L"";
		goto EXIT;
	}

	//サロゲートペアと0終端を考慮したバッファサイズ
	buffSize = (int)(pStr->length()) * 2 + 1;

	try {
		wstrBuff = new WCHAR[buffSize];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", buffSize, 0);
		goto EXIT;
	}

	memset(wstrBuff, 0, sizeof(WCHAR) * buffSize);

	apiresult = MultiByteToWideChar(
						_getmbcp(),			//コードページ
						MB_PRECOMPOSED,		//フラグ：
						pStr->c_str(),		//変換元マルチバイト文字列
						(int)(pStr->length()),	//変換元マルチバイト文字列バイト数
						wstrBuff,			//変換先ワイド文字列バッファ
						buffSize - 1		//バッファサイズ（ワイド文字数単位）
					);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	*pWstr = wstrBuff;

EXIT:;
	delete [] wstrBuff;
	return result;
}


