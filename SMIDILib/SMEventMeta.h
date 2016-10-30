//******************************************************************************
//
// Simple MIDI Library / SMEventMeta
//
// メタイベントクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// イベントクラスから派生させる設計が理想だが、newの実施回数を激増させる
// ため、スタックで処理できるデータ解析ユーティリティクラスとして実装する。

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"
#include <string>

namespace SMIDILib {


//******************************************************************************
// メタイベントクラス
//******************************************************************************
class SMIDILIB_API SMEventMeta
{
public:

	//コンストラクタ／デストラクタ
	SMEventMeta();
	~SMEventMeta(void);

	//イベントアタッチ
	void Attach(SMEvent* pEvent);

	//メタタイプ取得
	unsigned char GetType();

	//テンポ取得
	unsigned long GetTempo();

	//テンポ取得(BPM)
	unsigned long GetTempoBPM();

	//テキスト取得
	int GetText(std::string* pText);

	//ポート番号取得
	unsigned char GetPortNo();

	//拍子記号取得
	void GetTimeSignature(unsigned long* pNumerator, unsigned long* pDenominator);

private:

	SMEvent* m_pEvent;

	//代入とコピーコンストラクタの禁止
	void operator=(const SMEventMeta&);
	SMEventMeta(const SMEventMeta&);

};

} // end of namespace

