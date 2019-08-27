//******************************************************************************
//
// MIDITrail / MTDashboardLive
//
// ライブモニタ用ダッシュボード描画クラス
//
// Copyright (C) 2012-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTDashboardLive.h"

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
	TCHAR counter[100];
	
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
	TCHAR counter[100];
	
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
	charWidth = tw / (unsigned long)_tcslen(MTDASHBOARDLIVE_COUNTER_CHARS);
	
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
		TCHAR* pStr,
		unsigned long bufSize
	)
{
	int result = 0;
	int eresult = 0;
	const TCHAR* pMonitorStatus = _T("");
	
	if (m_isMonitoring) {
		pMonitorStatus = _T("");
	}
	else {
		pMonitorStatus = _T("[MONITERING OFF]");
	}
	
	eresult = _stprintf_s(
				pStr,
				bufSize,
				_T("NOTES:%08lu %s"),
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
	TCHAR hexColor[16] = {_T('\0')};
	MTConfFile confFile;
	
	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;
	
	//----------------------------------
	//色情報
	//----------------------------------
	result = confFile.SetCurSection(_T("Color"));
	if (result != 0) goto EXIT;
	
	//キャプションカラー
	result = confFile.GetStr(_T("CaptionRGBA"), hexColor, 16, _T("FFFFFFFF"));
	if (result != 0) goto EXIT;
	m_CaptionColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);
	
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
	TCHAR title[256] = {0}; //MAXPNAMELEN 32 より大きいサイズにする
	const TCHAR* pDisplayName = NULL;
	
	m_Title.Release();
	
	if (pName == NULL) {
		pDisplayName = _T("(none)");
	}
	else if (_tcslen(pName) == 0) {
		pDisplayName = _T("(none)");
	}
	else {
		pDisplayName = pName;
	}
	
	//タイトルキャプション
	eresult = _stprintf_s(
				title,
				256,
				_T("MIDI IN: %s"),
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


