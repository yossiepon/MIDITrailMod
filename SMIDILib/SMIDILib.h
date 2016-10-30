//******************************************************************************
//
// Simple MIDI Library / SMIDILib
//
// シンプルMIDIライブラリヘッダ
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

//共通定義
#include "SMCommon.h"

//標準MIDIファイル読み込みクラス
#include "SMFileReader.h"

//イベントクラス系
#include "SMEvent.h"
#include "SMEventMIDI.h"
#include "SMEventSysEx.h"
#include "SMEventSysMsg.h"
#include "SMEventMeta.h"

//リストクラス系
#include "SMTrack.h"
#include "SMNoteList.h"
#include "SMBarList.h"
#include "SMPortList.h"

//デバイス制御系
#include "SMOutDevCtrl.h"
#include "SMInDevCtrl.h"

//シーケンス処理系
#include "SMSeqData.h"
#include "SMSequencer.h"
#include "SMMsgParser.h"

//モニタ系
#include "SMLiveMonitor.h"

//その他
#include "SMRcpConv.h"

