//******************************************************************************
//
// Simple MIDI Library / SMCommon
//
// 共通定義
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


//最大ポート数
#define SM_MAX_PORT_NUM  (256)

//最大チャンネル数
#define SM_MAX_CH_NUM  (16)

//最大ノート数
#define SM_MAX_NOTE_NUM  (128)

//最大コントロールチェンジ数
#define SM_MAX_CC_NUM  (128)

//デフォルトBPM (beats per minute)
//  標準MIDIファイル仕様では未指定の場合に120とみなす
#define SM_DEFAULT_BPM    (120)

//デフォルトテンポ（四分音符の時間間隔／単位：マイクロ秒）
//  BPM=120（1分間で四分音符120回）の場合 = 500msec = 500,000μsec
//  標準MIDIファイル仕様ではマイクロ秒単位で表現される
#define SM_DEFAULT_TEMPO  ((60 * 1000 / SM_DEFAULT_BPM) * 1000)

//デフォルト拍子記号
//  標準MIDIファイル仕様では未指定の場合に4/4とみなす
#define SM_DEFAULT_TIME_SIGNATURE_NUMERATOR     (4)   //分子
#define SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR   (4)   //分母

//デフォルトピッチベンド感度：2半音
#define SM_DEFAULT_PITCHBEND_SENSITIVITY  (2)

