//******************************************************************************
//
// MIDITrail / MTDashboard
//
// ダッシュボード描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTDashboard.h"
#include <string>

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTDashboard::MTDashboard(void)
{
	m_hWnd = NULL;
	m_PosCounterX = 0.0f;
	m_PosCounterY = 0.0f;
	m_CounterMag = MTDASHBOARD_DEFAULT_MAGRATE;

	m_PlayTimeSec = 0;
	m_TotalPlayTimeSec = 0;
	m_TempoBPM = 0;
	m_BeatNumerator = 0;
	m_BeatDenominator = 0;
	m_BarNo = 0;
	m_BarNum = 0;
	m_NoteCount = 0;
	m_NoteNum = 0;
	m_PlaySpeedRatio = 100;

	m_TempoBPMOnStart = 0;
	m_BeatNumeratorOnStart = 0;
	m_BeatDenominatorOnStart = 0;

	m_CaptionColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	m_isEnable = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTDashboard::~MTDashboard(void)
{
	Release();
}

//******************************************************************************
// ダッシュボード生成
//******************************************************************************
int MTDashboard::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		HWND hWnd
   )
{
	int result = 0;
	std::string title;
	SMTrack track;
	SMNoteList noteList;
	TCHAR counter[100];

	Release();

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_hWnd = hWnd;

	//設定読み込み
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//タイトルキャプション
	//TODO: UNICODE版ビルドには対応していない
	title = "TITLE: ";
	title += pSeqData->GetTitle();
	result = m_Title.Create(
					pD3DDevice,
					MTDASHBOARD_FONTNAME,	//フォント名称
					MTDASHBOARD_FONTSIZE,	//フォントサイズ
					(TCHAR*)title.c_str()	//キャプション
				);
	if (result != 0) goto EXIT;
	m_Title.SetColor(m_CaptionColor);

	//カウンタキャプション
	result = m_Counter.Create(
					pD3DDevice,
					MTDASHBOARD_FONTNAME,		//フォント名称
					MTDASHBOARD_FONTSIZE,		//フォントサイズ
					MTDASHBOARD_COUNTER_CHARS,	//表示文字
					MTDASHBOARD_COUNTER_SIZE	//キャプションサイズ
				);
	if (result != 0) goto EXIT;
	m_Counter.SetColor(m_CaptionColor);

// >>> modify 20120728 yossiepon begin

	//全体演奏時間
	//GetTotalPlayTimeの戻り値が[sec.]から[msec.]になっているため、/1000を削除
	SetTotalPlayTimeSec(pSeqData->GetTotalPlayTime());

// <<< modify 20120728 yossiepon end

	//テンポ(BPM)
	SetTempoBPM(pSeqData->GetTempoBPM());
	m_TempoBPMOnStart = pSeqData->GetTempoBPM();

	//拍子記号
	SetBeat(pSeqData->GetBeatNumerator(), pSeqData->GetBeatDenominator());
	m_BeatNumeratorOnStart = pSeqData->GetBeatNumerator();
	m_BeatDenominatorOnStart = pSeqData->GetBeatDenominator();

	//小節番号
	SetBarNo(1);

	//小節数
	SetBarNum(pSeqData->GetBarNum());

	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	result = track.GetNoteList(&noteList);
	if (result != 0) goto EXIT;

	m_NoteNum = noteList.GetSize();

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
int MTDashboard::Transform(
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
int MTDashboard::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	D3DXMATRIX mtxWorld;
	TCHAR counter[100];

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (!m_isEnable) goto EXIT;

	//タイトル描画：カウンタと同じ拡大率で表示する
	result = m_Title.Draw(pD3DDevice, MTDASHBOARD_FRAMESIZE, MTDASHBOARD_FRAMESIZE, m_CounterMag);
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
void MTDashboard::Release()
{
	m_Title.Release();
	m_Counter.Release();
}

//******************************************************************************
// カウンタ表示位置取得
//******************************************************************************
int MTDashboard::_GetCounterPos(
		float* pX,
		float* pY
	)
{
	int result = 0;
	BOOL bresult = 0;
	RECT rect;
	unsigned long cw, ch = 0;
	unsigned long tw, th = 0;
	unsigned long charHeight, charWidth = 0;
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
	charWidth = tw / _tcslen(MTDASHBOARD_COUNTER_CHARS);

	//拡大率1.0のキャプションサイズ
	captionWidth = (unsigned long)(charWidth * MTDASHBOARD_COUNTER_SIZE);

	//カウンタ文字列が画面からはみ出す場合は画面に収まるように拡大率を更新する
	//  タイトルがはみ出すのは気にしないことにする
	if (((cw - (MTDASHBOARD_FRAMESIZE*2)) < captionWidth) && (tw > 0)) {
		newMag = (float)(cw - (MTDASHBOARD_FRAMESIZE*2)) / (float)captionWidth;
		if (m_CounterMag > newMag) {
			m_CounterMag = newMag;
		}
	}

	//テクスチャの表示倍率を考慮して表示位置を算出
	*pX = MTDASHBOARD_FRAMESIZE;
	*pY = (float)ch - ((float)th * m_CounterMag) - MTDASHBOARD_FRAMESIZE;

EXIT:;
	return result;
}

//******************************************************************************
// 演奏時間登録（秒）
//******************************************************************************
void MTDashboard::SetPlayTimeSec(
		unsigned long playTimeSec
	)
{
	m_PlayTimeSec = playTimeSec * 1000;
}

//******************************************************************************
// 演奏時間登録（ミリ秒）
//******************************************************************************
void MTDashboard::SetPlayTimeMSec(
		unsigned long playTimeSec
	)
{
	m_PlayTimeSec = playTimeSec;
}

//******************************************************************************
// 全体演奏時間登録（秒）
//******************************************************************************
void MTDashboard::SetTotalPlayTimeSec(
		unsigned long totalPlayTimeSec
	)
{
	m_TotalPlayTimeSec = totalPlayTimeSec;
}

//******************************************************************************
// テンポ登録(BPM)
//******************************************************************************
void MTDashboard::SetTempoBPM(
		unsigned long bpm
	)
{
	m_TempoBPM = bpm;
}

//******************************************************************************
// 拍子記号登録
//******************************************************************************
void MTDashboard::SetBeat(
		unsigned long numerator,
		unsigned long denominator
	)
{
	m_BeatNumerator = numerator;
	m_BeatDenominator = denominator;
}

//******************************************************************************
// 小節数登録
//******************************************************************************
void MTDashboard::SetBarNum(
		unsigned long barNum
	)
{
	m_BarNum = barNum;
}

//******************************************************************************
// 小節番号登録
//******************************************************************************
void MTDashboard::SetBarNo(
		unsigned long barNo
	)
{
	m_BarNo = barNo;
}

//******************************************************************************
// ノートON登録
//******************************************************************************
void MTDashboard::SetNoteOn()
{
	m_NoteCount++;
}

//******************************************************************************
// 演奏速度登録
//******************************************************************************
void MTDashboard::SetPlaySpeedRatio(
		unsigned long ratio
	)
{
	m_PlaySpeedRatio = ratio;
}

//******************************************************************************
// ノート数登録
//******************************************************************************
void MTDashboard::SetNotesCount(
		unsigned long notesCount
	)
{
	m_NoteCount = notesCount;
}

//******************************************************************************
// カウンタ文字列取得
//******************************************************************************
int MTDashboard::_GetCounterStr(
		TCHAR* pStr,
		unsigned long bufSize
	)
{
	int result = 0;
	int eresult = 0;
	TCHAR spdstr[16] = {0};

	eresult = _stprintf_s(
				pStr,
				bufSize,
// >>> modify 20120728 yossiepon begin
				_T("TIME:%02d:%02d.%03d/%02d:%02d.%03d BPM:%03d BEAT:%d/%d BAR:%03d/%03d NOTES:%05d/%05d"),
				m_PlayTimeSec / 60000,
				(m_PlayTimeSec % 60000) / 1000,
				m_PlayTimeSec % 1000,
				m_TotalPlayTimeSec / 60000,
				(m_TotalPlayTimeSec % 60000) / 1000,
				m_TotalPlayTimeSec % 1000,
// <<< modify 20120728 yossiepon end
				m_TempoBPM,
				m_BeatNumerator,
				m_BeatDenominator,
				m_BarNo,
				m_BarNum,
				m_NoteCount,
				m_NoteNum
			);
	if (eresult < 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//演奏速度が100%以外の場合に限りカウンタに表示する
	if (m_PlaySpeedRatio != 100) {
		eresult = _stprintf_s(spdstr, 16, _T(" SPEED:%03lu%%"), m_PlaySpeedRatio);
		if (eresult < 0) {
			result = YN_SET_ERR("Program error.", 0, 0);
			goto EXIT;
		}
		_tcscat_s(pStr, bufSize, spdstr);
	}

EXIT:;
	return result;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTDashboard::Reset()
{
	m_PlayTimeSec = 0;
	m_TempoBPM = m_TempoBPMOnStart;
	m_BeatNumerator = m_BeatNumeratorOnStart;
	m_BeatDenominator = m_BeatDenominatorOnStart;
	m_BarNo = 1;
	m_NoteCount = 0;
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTDashboard::_LoadConfFile(
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
// 演奏時間取得
//******************************************************************************
unsigned long MTDashboard::GetPlayTimeSec()
{
	return m_PlayTimeSec;
}

//******************************************************************************
// 表示設定
//******************************************************************************
void MTDashboard::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}

