//******************************************************************************
//
// MIDITrail / MTNotePitchBend
//
// ピッチベンド情報クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ポート／チャンネル単位のピッチベンド情報を保持する。

#pragma once

#include "SMCommon.h"


//******************************************************************************
// ピッチベンド情報クラス
//******************************************************************************
class MTNotePitchBend
{
public:

	//コンストラクタ／デストラクタ
	MTNotePitchBend(void);
	virtual ~MTNotePitchBend(void);

	//初期化
	int Initialize();

	//ピッチベンド登録
	int SetPitchBend(
			unsigned char portNo,
			unsigned char chNo,
			short value,
			unsigned char sensitivity
		);

	//ピッチベンド値取得
	short GetValue(unsigned long portNo, unsigned long chNo);

	//ピッチベンド感度取得
	unsigned char GetSensitivity(unsigned long portNo, unsigned long chNo);

	//リセット
	void Reset();

	//ピッチベンド表示効果設定
	void SetEnable(bool isEnable);

private:

	//ピッチベンド情報
	struct MTNOTEPITCHBEND_PITCHBEND_INFO {
		short value;
		unsigned char sensitivity;
	};

private:

	//ピッチベンド表示効果
	bool m_isEnable;

	//ピッチベンド情報
	MTNOTEPITCHBEND_PITCHBEND_INFO m_PitchBend[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];

};

