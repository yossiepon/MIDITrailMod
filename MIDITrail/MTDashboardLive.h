//******************************************************************************
//
// MIDITrail / MTDashboardLive
//
// ライブモニタ用ダッシュボード描画クラス
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// MIDI IN デイバイス名, ノート数 を表示する。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXColorUtil.h"
#include "MTStaticCaption.h"
#include "MTDynamicCaption.h"


//******************************************************************************
// パラメータ定義
//******************************************************************************
//フォント設定
//  Windows ：フォントサイズ40 -> ビットマップサイズ縦40ピクセル (MS Gothic)
//  Mac OS X：フォントサイズ40 -> ビットマップサイズ縦50ピクセル (Monaco)
#define MTDASHBOARDLIVE_FONTNAME  _T("MS Gothic")
#define MTDASHBOARDLIVE_FONTSIZE  (40)

//カウンタキャプション文字列
#define MTDASHBOARDLIVE_COUNTER_CHARS  "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:/%[] "

//カウンタキャプションサイズ
//   1234567890123456789012345678901  (31)
//  "NOTES:00000000 [MONITERING OFF]"
//  余裕をみて40にしておく
#define MTDASHBOARDLIVE_COUNTER_SIZE  (40)

//枠サイズ（ピクセル）
#define MTDASHBOARDLIVE_FRAMESIZE  (5.0f)

//デフォルト表示拡大率
#define MTDASHBOARDLIVE_DEFAULT_MAGRATE  (0.45f)  //Windows版では0.5

//******************************************************************************
// ライブモニタ用ダッシュボード描画クラス
//******************************************************************************
class MTDashboardLive
{
public:
	
	//コンストラクタ／デストラクタ
	MTDashboardLive(void);
	virtual ~MTDashboardLive(void);
	
	//生成
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, HWND hWnd);
	
	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector);
	
	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//解放
	void Release();
	
	//モニタ状態登録
	void SetMonitoringStatus(bool isMonitoring);
	
	//ノートON登録
	void SetNoteOn();
	
	//リセット
	void Reset();
	
	//表示設定
	void SetEnable(bool isEnable);
	
	//MIDI IN デバイス名登録
	int SetMIDIINDeviceName(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pName);
	
private:
	
	HWND m_hWnd;

	MTStaticCaption m_Title;
	
	MTDynamicCaption m_Counter;
	float m_PosCounterX;
	float m_PosCounterY;
	float m_CounterMag;
	
	bool m_isMonitoring;
	unsigned long m_NoteCount;
	
	D3DXCOLOR m_CaptionColor;
	
	//表示可否
	bool m_isEnable;
	
	int _GetCounterPos(float* pX, float* pY);
	int _GetCounterStr(TCHAR* pStr, unsigned long bufSize);
	int _LoadConfFile(const TCHAR* pSceneName);
	
};


