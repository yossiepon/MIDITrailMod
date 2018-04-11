//******************************************************************************
//
// Simple MIDI Library / SMEventMeta
//
// メタイベントクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventMeta.h"
#include <new>

// >>> add 20170528 yossiepon begin
#include <algorithm>
#include <functional>
#include <cctype>
// <<< add 20170528 yossiepon end

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMEventMeta::SMEventMeta()
{
	m_pEvent = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMEventMeta::~SMEventMeta(void)
{
}

//******************************************************************************
// イベント紐付け
//******************************************************************************
void SMEventMeta::Attach(
		SMEvent* pEvent
	)
{
	m_pEvent = pEvent;
}

//******************************************************************************
// メタイベント種別取得
//******************************************************************************
unsigned char SMEventMeta::GetType()
{
	unsigned char type = 0;

	if (m_pEvent == NULL) goto EXIT;

	type = m_pEvent->GetMetaType();

EXIT:;
	return type;
}

//******************************************************************************
// テンポ取得
//******************************************************************************
unsigned long SMEventMeta::GetTempo()
{
	unsigned long tempo = 0;
	unsigned char* pData = NULL;

	if (m_pEvent == NULL) goto EXIT;

	if (m_pEvent->GetMetaType() != 0x51) {
		goto EXIT;
	}
	if (m_pEvent->GetDataSize() != 3) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();
	tempo = (pData[0] << 16) | (pData[1] << 8) | (pData[3]);

EXIT:;
	return tempo;
}

//******************************************************************************
// テンポ取得(BPM)
//******************************************************************************
unsigned long SMEventMeta::GetTempoBPM()
{
	unsigned long tempo = 0;
	unsigned long tempoBPM = 0;

	tempo = GetTempo();
	if (tempo == 0) goto EXIT;

	tempoBPM = (60 * 1000 * 1000) / tempo;

EXIT:;
	return tempoBPM;
}

//******************************************************************************
// テキスト取得
//******************************************************************************
int SMEventMeta::GetText(
		std::string* pText
	)
{
	int result = 0;
	char* pBuf = NULL;
	unsigned long size = 0;

	if (m_pEvent == NULL) goto EXIT;

	size =  m_pEvent->GetDataSize();

	try {
		pBuf = new char[size + 1];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", size + 1, 0);
		goto EXIT;
	}

	memcpy(pBuf, m_pEvent->GetDataPtr(), size);
	pBuf[size] = '\0';

	*pText = pBuf;

// >>> add 20170528 yossiepon begin
	// rtrim
	pText->erase(std::find_if(pText->rbegin(), pText->rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), pText->end());
// <<< add 20170528 yossiepon end

EXIT:;
	delete [] pBuf;
	return result;
}

//******************************************************************************
// ポート番号取得
//******************************************************************************
unsigned char SMEventMeta::GetPortNo()
{
	unsigned char portNo = 0;
	unsigned char* pData = NULL;

	if (m_pEvent == NULL) goto EXIT;

	if (m_pEvent->GetMetaType() != 0x21) {
		goto EXIT;
	}
	if (m_pEvent->GetDataSize() != 1) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();
	portNo = pData[0];

EXIT:;
	return portNo;
}

//******************************************************************************
// 拍子記号取得
//******************************************************************************
void SMEventMeta::GetTimeSignature(
		unsigned long* pNumerator,
		unsigned long* pDenominator
	)
{
	unsigned char* pData = NULL;
	unsigned long i = 0;

	if (m_pEvent == NULL) goto EXIT;

	if (m_pEvent->GetMetaType() != 0x58) {
		goto EXIT;
	}
	if (m_pEvent->GetDataSize() != 4) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();

	// FF 58 04 nn dd cc bb
	//   nn: 分子
	//   dd: 分母（2のマイナス乗）
	//   cc: 1メトロノームクリックあたりのMIDIクロック数
	//   bb: MIDI四分音符（24 MIDIクロック）中に記譜上の32分音符が入る数（通常は8）
	//   → cc bb は無視

	//分子
	*pNumerator   = pData[0];

	//分母
	*pDenominator = 1;
	for (i = 0; i < pData[1]; i++) {
		*pDenominator = *pDenominator * 2;
	}

EXIT:;
	return;
}

} // end of namespace

