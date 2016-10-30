//******************************************************************************
//
// Simple MIDI Library / SMEventSysMsg
//
// システムメッセージイベントクラス
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// イベントクラスから派生させる設計が理想だが、newの実施回数を激増させる
// ため、スタックで処理できるデータ解析ユーティリティクラスとして実装する。

// 本クラスの対象とするシステムメッセージ一覧
//   F1 dd     システムコモンメッセージ：クオーターフレーム(MTC)
//   F2 dl dm  システムコモンメッセージ：ソングポジションポインタ
//   F3 dd     システムコモンメッセージ：ソングセレクト
//   F4 未定義
//   F5 未定義
//   F6 システムコモンメッセージ：チューンリクエスト
//   F8 システムリアルタイムメッセージ：タイミングクロック
//   F9 未定義
//   FA システムリアルタイムメッセージ：スタート
//   FB システムリアルタイムメッセージ：コンティニュー
//   FC システムリアルタイムメッセージ：ストップ
//   FD 未定義
//   FE システムリアルタイムメッセージ：アクティブセンシング
//   FF システムリアルタイムメッセージ：システムリセット
// 下記メッセージは本クラスの対象外とする
//   F0 ... F7 システムエクスクルーシブ
//   F7 エンドオブシステムエクスクルーシブ

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMEvent.h"


namespace SMIDILib {

//******************************************************************************
// システムメッセージイベントクラス
//******************************************************************************
class SMIDILIB_API SMEventSysMsg
{
public:
	
	//コンストラクタ／デストラクタ
	SMEventSysMsg();
	virtual ~SMEventSysMsg(void);
	
	//イベントアタッチ
	void Attach(SMEvent* pEvent);
	
	//MIDI出力メッセージ取得
	int GetMIDIOutShortMsg(unsigned long* pMsg, unsigned long* pSize);
	
private:
	
	SMEvent* m_pEvent;
	
	//代入とコピーコンストラクタの禁止
	void operator=(const SMEventSysMsg&);
	SMEventSysMsg(const SMEventSysMsg&);
	
};

} // end of namespace


