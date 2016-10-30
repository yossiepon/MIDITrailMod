//******************************************************************************
//
// MIDITrail / MTNotePitchBend
//
// ピッチベンド情報クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNotePitchBend.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNotePitchBend::MTNotePitchBend(void)
{
	Reset();
	m_isEnable = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNotePitchBend::~MTNotePitchBend(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int MTNotePitchBend::Initialize()
{
	int result = 0;

	Reset();

	return result;
}

//******************************************************************************
// ピッチベンド設定
//******************************************************************************
int MTNotePitchBend::SetPitchBend(
		unsigned char portNo,
		unsigned char chNo,
		short value,
		unsigned char sensitivity
	)
{
	int result = 0;

	if (chNo >= SM_MAX_CH_NUM) {
		result = YN_SET_ERR("Program error.", value, sensitivity);
		goto EXIT;
	}

	m_PitchBend[portNo][chNo].value = value;
	m_PitchBend[portNo][chNo].sensitivity = sensitivity;

EXIT:;
	return result;
}

//******************************************************************************
// ピッチベンド値取得
//******************************************************************************
short MTNotePitchBend::GetValue(
		unsigned long portNo,
		unsigned long chNo
	)
{
	short value = 0;

	if ((portNo < SM_MAX_PORT_NUM) && (chNo < SM_MAX_CH_NUM)) {
		value = m_PitchBend[portNo][chNo].value;
	}

	if (!m_isEnable) {
		value = 0;
	}

	return value;
}

//******************************************************************************
// ピッチベンド感度取得
//******************************************************************************
unsigned char MTNotePitchBend::GetSensitivity(
		unsigned long portNo,
		unsigned long chNo
	)
{
	unsigned char sensitivity = 0;

	if ((portNo < SM_MAX_PORT_NUM) && (chNo < SM_MAX_CH_NUM)) {
		sensitivity = m_PitchBend[portNo][chNo].sensitivity;
	}

	if (!m_isEnable) {
		sensitivity = 0;
	}

	return sensitivity;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTNotePitchBend::Reset()
{
	unsigned long portNo = 0;
	unsigned long chNo = 0;

	for (portNo = 0; portNo < SM_MAX_PORT_NUM; portNo++) {
		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
			m_PitchBend[portNo][chNo].value = 0;
			m_PitchBend[portNo][chNo].sensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;
		}
	}

	return;
}

//******************************************************************************
// 表示設定
//******************************************************************************
void MTNotePitchBend::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


