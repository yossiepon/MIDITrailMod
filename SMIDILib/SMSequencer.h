//******************************************************************************
//
// Simple MIDI Library / SMSequencer
//
// シーケンサクラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "mmsystem.h"
#include "SMEventMIDI.h"
#include "SMEventSysEx.h"
#include "SMEventMeta.h"
#include "SMSeqData.h"
#include "SMMsgTransmitter.h"
#include "SMOutDevCtrl.h"
#include "SMCommon.h"
#include "SMFPUCtrl.h"
#include "SMEventWatcher.h"

namespace SMIDILib {


//******************************************************************************
// シーケンサクラス
//******************************************************************************
class SMIDILIB_API SMSequencer
{
public:

	//演奏状態
	enum Status {
		StatusPlay,
		StatusPause,
		StatusStop
	};

	//ユーザ要求
	enum UserRequest {
		RequestNone,
		RequestPause,
		RequestStop,
		RequestSkip
	};

	//コンストラクタ／デストラクタ
	SMSequencer(void);
	virtual ~SMSequencer(void);

	//初期化
	int Initialize(HWND hTargetWnd, unsigned long msgId);

	//ポート対応デバイス登録
	int SetPortDev(unsigned char portNo, const char* pProductName);

	//シーケンスデータ登録
	int SetSeqData(SMSeqData* pSeqData);

	//演奏開始
	int Play();

	//演奏一時停止
	void Pause();

	//演奏再開
	int Resume();

	//演奏停止
	void Stop();

	//再生スピード設定
	void SetPlaybackSpeed(unsigned long nTimes); //n倍速
	void SetPlaySpeedRatio(unsigned long ratio); //パーセント

	//リワインド／スキップ移動時間設定
	void SetMovingTimeSpanInMsec(unsigned long timeSpan);

	//演奏位置変更
	int Skip(int relativeTimeInMsec);

private:

	//演奏状態
	Status m_Status;
	unsigned long m_PlayIndex;
	UserRequest m_UserRequest;
	SMMsgTransmitter m_MsgTrans;
	SMEventWatcher m_EventWatcher;

	//MIDIデバイス系
	SMOutDevCtrl m_OutDevCtrl;
	unsigned char m_PortNo;
	char m_PortDevName[SM_MIDIOUT_PORT_NUM_MAX][MAXPNAMELEN];

	//MIDIデータ系
	SMSeqData* m_pSeqData;
	SMTrack m_Track;
	SMEvent m_Event;

	//タイマー制御系
	UINT m_TimerID;
	unsigned long m_TimerResolution;
	unsigned long m_TimeDivision;
	unsigned long m_Tempo;
	unsigned long long m_PrevTimerTime;
	unsigned long long m_CurPlayTime;
	unsigned long long m_PrevEventTime;
	double m_NextEventTime;
	unsigned long long m_NextNtcTime;
	unsigned long m_PrevDeltaTime;
	unsigned long m_TotalTickTime;
	unsigned long m_TotalTickTimeTemp;
	unsigned long m_PlaybackSpeed;
	double m_PlaySpeedRatio;

	//浮動小数点演算制御
	SMFPUCtrl m_FPUCtrl;

	//小節番号制御系
	unsigned long m_TickTimeOfBar;
	unsigned long m_CurBarNo;
	unsigned long m_PrevBarTickTime;

	//拍子記号
	unsigned long m_BeatNumerator;
	unsigned long m_BeatDenominator;

	//スキップ制御
	bool m_isSkipping;
	unsigned long long m_SkipTargetTime;
	unsigned long m_NotesCount;
	unsigned long m_MovingTimeSpanInMsec;
	unsigned char m_CachePitchBend[SM_MAX_PORT_NUM][SM_MAX_CH_NUM][2];
	unsigned char m_CacheCC001_Modulation[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_CacheCC007_Volume[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_CacheCC010_Panpot[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];
	unsigned char m_CacheCC011_Expression[SM_MAX_PORT_NUM][SM_MAX_CH_NUM];

	//タイマーデバイス処理
	int _InitializeTimerDev();
	int _ReleaseTimerDev();

	//ポート制御
	void _ClearPortInfo();
	int _OpenMIDIOutDev();
	int _CloseMIDIOutDev();

	//再生制御
	int _InitializeParamsOnPlayStart();

	//演奏スレッドインターバル処理
	int _IntervalProc(BOOL* pIsContinue);

	//時間制御
	int _UpdatePlayPosition();
	double _ConvTick2TimeNanosec(unsigned long tickTime);
	unsigned long _ConvTimeNanosec2Tick(unsigned long long timeMsec);
	unsigned long long _GetCurTimeInNano();

	//MIDI出力処理
	int _OutputMIDIEvent(unsigned char portNo, SMEvent* pEvent);
	int _SendMIDIEvent(unsigned char portNo, SMEventMIDI* pMIDIEvent);
	int _SendSysExEvent(unsigned char portNo, SMEventSysEx* pSysExEvent);
	int _SendMetaEvent(unsigned char portNo, SMEventMeta* pMetaEvent);
	int _AllTrackNoteOff();

	//その他
	int _ProcUserRequest(BOOL* pIsContinue);

	//スキップ制御
	void _ClearMIDIEventCache();
	int _FilterMIDIEvent(unsigned char portNo, SMEventMIDI* pMIDIEvent, bool* pIsFiltered);
	int _SendMIDIEventCache();
	int _SendMIDIEventPitchBend(unsigned char portNo, unsigned char chNo, unsigned char* pPtichBend);
	int _SendMIDIEventCC(unsigned char portNo, unsigned char chNo, unsigned char ccNo, unsigned char ccValue);
	int _ProcSkip(unsigned long long targetTimeInNanoSec, BOOL* pIsContinue);
	void _SlidePlaybackTime(unsigned long long startPlayTime, unsigned long startTickTime, unsigned long endTickTime);

	//タイマー処理
	static void CALLBACK _TimerCallBack(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

protected:

	int _OnTimer();

private:

	//代入とコピーコンストラクタの禁止
	void operator=(const SMSequencer&);
	SMSequencer(const SMSequencer&);

};

} // end of namespace

