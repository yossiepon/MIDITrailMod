//******************************************************************************
//
// MIDITrail / MTDashboard
//
// ダッシュボード描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// 曲名／演奏時間／テンポ／ビート／小節番号 を表示する。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "MTStaticCaption.h"
#include "MTDynamicCaption.h"

using namespace SMIDILib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
//フォント設定
#define MTDASHBOARD_FONTNAME  _T("MS Gothic")
#define MTDASHBOARD_FONTSIZE  (40)

//カウンタキャプション文字列
#define MTDASHBOARD_COUNTER_CHARS  _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:/% ")

//カウンタキャプションサイズ
//   12345678901234567890123456789012345678901234567890123456789012345678901234  (74)
//  "TIME:00:00/00:00 BPM:000 BEAT:4/4 BAR:000/000 NOTES:00000/00000 SPEED:000%"
//  余裕をみて80にしておく
#define MTDASHBOARD_COUNTER_SIZE  (80)

//枠サイズ（ピクセル）
#define MTDASHBOARD_FRAMESIZE  (5.0f)

//デフォルト表示拡大率
#define MTDASHBOARD_DEFAULT_MAGRATE  (0.5f)


//******************************************************************************
// ダッシュボード描画クラス
//******************************************************************************
class MTDashboard
{
public:

	//コンストラクタ／デストラクタ
	MTDashboard(void);
	virtual ~MTDashboard(void);

	//生成
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData, HWND hWnd);

	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector);

	//描画
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//解放
	void Release();

	//演奏経過時間と総演奏時間の登録
	void SetPlayTimeSec(unsigned long playTimeSec);
	void SetTotalPlayTimeSec(unsigned long totalPlayTimeSec);

	//テンポ登録
	void SetTempoBPM(unsigned long bpm);

	//小節番号と全小節数の登録
	void SetBarNo(unsigned long barNo);
	void SetBarNum(unsigned long barNum);

	//拍子記号登録
	void SetBeat(unsigned long numerator, unsigned long denominator);

	//ノートON登録
	void SetNoteOn();

	//演奏速度登録
	void SetPlaySpeedRatio(unsigned long ratio);

	//リセット
	void Reset();

	//ノート数登録
	void SetNotesCount(unsigned long notesCount);

	//演奏時間取得
	unsigned long GetPlayTimeSec();

	//表示設定
	void SetEnable(bool isEnable);

private:

	HWND m_hWnd;
	
	MTStaticCaption m_Title;
	
	MTDynamicCaption m_Counter;
	float m_PosCounterX;
	float m_PosCounterY;
	float m_CounterMag;

	unsigned long m_PlayTimeSec;
	unsigned long m_TotalPlayTimeSec;
	unsigned long m_TempoBPM;
	unsigned long m_BeatNumerator;
	unsigned long m_BeatDenominator;
	unsigned long m_BarNo;
	unsigned long m_BarNum;
	unsigned long m_NoteCount;
	unsigned long m_NoteNum;
	unsigned long m_PlaySpeedRatio;

	unsigned long m_TempoBPMOnStart;
	unsigned long m_BeatNumeratorOnStart;
	unsigned long m_BeatDenominatorOnStart;

	D3DXCOLOR m_CaptionColor;

	//表示可否
	bool m_isEnable;

	int _GetCounterPos(float* pX, float* pY);
	int _GetCounterStr(TCHAR* pStr, unsigned long bufSize);
	int _LoadConfFile(const TCHAR* pSceneName);

};

