//******************************************************************************
//
// Simple MIDI Library / SMSequencer
//
// シーケンサクラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// タイマースレッドは演奏処理を行うため、MIDI出力デバイスの制御に専念
// させる。このスレッドで画面更新処理等を行ってはならない。
// 他スレッドへの通知はPostMessage等で実現する。
// _TimerCallBack()→_OnTimer()→・・・

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMSequencer.h"
#include "SMEventMIDI.h"
#include "SMEventSysEx.h"
#include "SMEventMeta.h"
#include "SMFPUCtrl.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMSequencer::SMSequencer(void)
{
	m_Status = StatusStop;
	m_PlayIndex = 0;
	m_UserRequest = RequestNone;
	m_PortNo = 0;
	m_pSeqData = NULL;
	m_TimerID = NULL;
	m_TimerResolution = 0;
	m_TimeDivision = 0;
	m_Tempo = SM_DEFAULT_TEMPO;

	m_PrevTimerTime = 0;
	m_CurPlayTime = 0;
	m_PrevEventTime = 0;
	m_NextEventTime = 0.0;
	m_NextNtcTime = 0;
	m_TotalTickTime = 0;
	m_TotalTickTimeTemp = 0;
	m_PlaybackSpeed = 1;
	m_PlaySpeedRatio = 1.0;

	//スキップ制御
	m_isSkipping = false;
	m_SkipTargetTime = 0;
	m_NotesCount = 0;
	m_MovingTimeSpanInMsec = 0;

	//小節番号制御系
	m_TickTimeOfBar = 0;
	m_CurBarNo = 1;
	m_PrevBarTickTime = 0;

	//拍子記号
	m_BeatNumerator = 0;
	m_BeatDenominator = 0;

	//ポート情報クリア
	_ClearPortInfo();
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMSequencer::~SMSequencer(void)
{
	//ポート情報クリア
	_ClearPortInfo();

	//MIDI出力デバイスを閉じる
	_CloseMIDIOutDev();

	//タイマデバイス解放
	_ReleaseTimerDev();
}

//******************************************************************************
// 初期化
//******************************************************************************
int SMSequencer::Initialize(
		HWND hTargetWnd,
		unsigned long msgId
	)
{
	int result = 0;

	if (m_Status != StatusStop) {
		result = YN_SET_ERR("Program error.", m_Status, 0);
		goto EXIT;
	}

	//MIDI出力デバイス初期化
	result = m_OutDevCtrl.Initialize();
	if (result != 0) goto EXIT;

	//ポート情報クリア
	_ClearPortInfo();

	//イベント転送オブジェクト初期化
	result = m_MsgTrans.Initialize(hTargetWnd, msgId);
	if (result != 0) goto EXIT;

	//イベントウォッチャー初期化
	result = m_EventWatcher.Initialize(&m_MsgTrans);
	if (result != 0) goto EXIT;

	//タイマデバイス初期化
	result = _InitializeTimerDev();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ポート対応デバイス登録
//******************************************************************************
int SMSequencer::SetPortDev(
		unsigned char portNo,
		const char* pProductName
	)
{
	int result = 0;
	errno_t eresult = 0;

	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}

	eresult = strcpy_s(m_PortDevName[portNo], MAXPNAMELEN, pProductName);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// シーケンスデータ登録
//******************************************************************************
int SMSequencer::SetSeqData(
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long numerator = 0;
	unsigned long denominator = 0;

	if (m_Status != StatusStop) {
		result = YN_SET_ERR("Program error.", m_Status, 0);
		goto EXIT;
	}

	m_pSeqData = pSeqData;

	//マージ済みトラック取得
	result = m_pSeqData->GetMergedTrack(&m_Track);
	if (result != 0) goto EXIT;

	//分解能取得：四分音符の長さを示す値 (ex. 48, 480, ...)
	m_TimeDivision = m_pSeqData->GetTimeDivision();
	if (m_TimeDivision == 0) {
		//データ異常：SMF読み込み時にチェックしているはず
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//テンポ取得
	m_Tempo = m_pSeqData->GetTempo();
	if (m_Tempo == 0) {
		//データ異常
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}

	//拍子記号から1小節あたりのチックタイムを算出
	numerator = m_pSeqData->GetBeatNumerator();
	denominator = m_pSeqData->GetBeatDenominator();
	if (denominator == 0) {
		//データ異常
		result = YN_SET_ERR("Invalid data found.", numerator, denominator);
		goto EXIT;
	}
	m_TickTimeOfBar = (numerator * m_TimeDivision * 4) / denominator;

	m_BeatNumerator = numerator;
	m_BeatDenominator = denominator;

EXIT:;
	return result;
}

//******************************************************************************
// 演奏開始
//******************************************************************************
int SMSequencer::Play()
{
	int result = 0;
	SMFPUCtrl fpuCtrl;

	if (m_pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//演奏中なら何もしない
	if (m_Status == StatusPlay) goto EXIT;

	//浮動小数点演算精度を倍精度に設定
	result = fpuCtrl.Start(SMFPUCtrl::FPUDouble);
	if (result != 0) goto EXIT;

	//先頭から演奏開始
	if (m_Status == StatusStop) {
		//再生開始パラメータ初期化
		result = _InitializeParamsOnPlayStart();
		if (result != 0) goto EXIT;

		//MIDI出力デバイスを開く
		result = _OpenMIDIOutDev();
		if (result != 0) goto EXIT;
	}
	//一時停止から演奏再開
	if (m_Status == StatusPause) {
		m_PrevTimerTime = _GetCurTimeInNano();
	}
	m_Status = StatusPlay;
	m_UserRequest = RequestNone;
	m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_PLAY);

	//タイマ起動
	m_TimerID = timeSetEvent(
					m_TimerResolution, //イベント遅延（ミリ秒）
					m_TimerResolution, //イベント分解能（ミリ秒）
					_TimerCallBack,    //コールバック関数
					(DWORD_PTR)this,   //ユーザーコールバックデータ
					TIME_PERIODIC      //タイマー種別：周期呼び出し
				);
	if (m_TimerID == NULL) {
		result = YN_SET_ERR("Timer device error.", m_TimerResolution, 0);
		goto EXIT;
	}

	result = fpuCtrl.End();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 演奏一時停止
//******************************************************************************
void SMSequencer::Pause()
{
	//要求を受け付けるだけ（キューイングはしない）
	//実際の処理はタイマースレッドに委任する
	m_UserRequest = RequestPause;
}

//******************************************************************************
// 演奏再開
//******************************************************************************
int SMSequencer::Resume()
{
	int result = 0;

	//現在はPlay()が再開処理も兼ねている
	result = Play();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 演奏停止
//******************************************************************************
void SMSequencer::Stop()
{

	if (m_Status == StatusPause) {
		//一時停止中の場合はタイマースレッドが停止しているため
		//ここから終了を通知する
		m_Status = StatusStop;
		m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_STOP);
	}
	else {
		//演奏中は要求を受け付けるだけ（キューイングはしない）
		//実際の処理はタイマースレッドに委任する
		m_UserRequest = RequestStop;
	}
}

//******************************************************************************
// 再生スピード設定（n倍速）
//******************************************************************************
void SMSequencer::SetPlaybackSpeed(
		unsigned long nTimes
	)
{
	m_PlaybackSpeed =  nTimes;
}

//******************************************************************************
// 再生スピード設定（パーセント）
//******************************************************************************
void SMSequencer::SetPlaySpeedRatio(
		unsigned long ratio
	)
{
	m_PlaySpeedRatio =  (double)ratio / 100.0;
}

//******************************************************************************
// リワインド／スキップ移動時間設定
//******************************************************************************
void SMSequencer::SetMovingTimeSpanInMsec(
		unsigned long timeSpan
	)
{
	m_MovingTimeSpanInMsec = timeSpan;
}

//******************************************************************************
//演奏位置スキップ
//******************************************************************************
int SMSequencer::Skip(
		int relativeTimeInMsec
	)
{
	int result = 0;
	unsigned long long diffTime = 0;

	//演奏中でなければ何もしない
	if (m_Status != StatusPlay) goto EXIT;

	//演奏位置
	if (relativeTimeInMsec < 0) {
		diffTime = (unsigned long long)(-1 * relativeTimeInMsec) * 1000000;
		if (m_CurPlayTime < diffTime) {
			m_SkipTargetTime = 0;
		}
		else {
			m_SkipTargetTime = m_CurPlayTime - diffTime;
		}
	}
	else {
		diffTime = (unsigned long long)(relativeTimeInMsec) * 1000000;
		m_SkipTargetTime = m_CurPlayTime + diffTime;
		//曲の終了時間を超える可能性がある
	}

	//演奏中は要求を受け付けるだけ（キューイングはしない）
	//実際の処理はタイマースレッドに委任する
	m_UserRequest = RequestSkip;

EXIT:;
	return result;
}

//******************************************************************************
// タイマデバイス初期化
//******************************************************************************
int SMSequencer::_InitializeTimerDev()
{
	int result = 0;
	UINT apiresult = 0;
	TIMECAPS tc;

	if (m_TimerResolution != 0) goto EXIT;

	//タイマデバイスの最小分解能を取得（通常1ms）
	apiresult = timeGetDevCaps(&tc, sizeof(TIMECAPS));
	if (apiresult != TIMERR_NOERROR) {
		result = YN_SET_ERR("Timer device error.", apiresult, 0);
		goto EXIT;
	}
	m_TimerResolution = tc.wPeriodMin;

	//最小タイマ分解能の設定
	timeBeginPeriod(m_TimerResolution);

EXIT:;
	return result;
}

//******************************************************************************
// タイマデバイス解放
//******************************************************************************
int SMSequencer::_ReleaseTimerDev()
{
	int result = 0;
	UINT apiresult = 0;

	if (m_TimerResolution != 0) {
		apiresult = timeEndPeriod(m_TimerResolution);
		if (apiresult != TIMERR_NOERROR) {
			result = YN_SET_ERR("Timer device error.", apiresult, 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ポート情報クリア
//******************************************************************************
void SMSequencer::_ClearPortInfo()
{
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortDevName[portNo][0] = '\0';
	}
}

//******************************************************************************
// MIDI出力デバイスオープン
//******************************************************************************
int SMSequencer::_OpenMIDIOutDev()
{
	int result = 0;
	unsigned char portNo = 0;

	//ポート対応デバイス名をMIDI出力デバイス制御に登録
	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		if (strlen(m_PortDevName[portNo]) > 0) {
			result = m_OutDevCtrl.SetPortDev(portNo, m_PortDevName[portNo]);
			if (result != 0) goto EXIT;
		}
	}

	//全ポートのデバイスを開く
	result = m_OutDevCtrl.OpenPortDevAll();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// MIDI出力デバイスクローズ
//******************************************************************************
int SMSequencer::_CloseMIDIOutDev()
{
	int result = 0;

	result = m_OutDevCtrl.ClosePortDevAll();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 演奏インターバル処理
//******************************************************************************
int SMSequencer::_IntervalProc(
		BOOL* pIsContinue
	)
{
	int result = 0;
	unsigned long deltaTime = 0;

	*pIsContinue = true;

	//演奏位置を更新
	result = _UpdatePlayPosition();
	if (result != 0) goto EXIT;

	//イベント処理時刻に到達していたら送信処理を行う
	if ((unsigned long long)m_NextEventTime <= m_CurPlayTime) {

		//チックタイム合計
		m_TotalTickTime += m_PrevDeltaTime;

		while (deltaTime == 0) {
			//イベント送信
			result = _OutputMIDIEvent(m_PortNo, &m_Event);
			if (result != 0) goto EXIT;

			//データ終端なら演奏終了
			m_PlayIndex++;
			if (m_PlayIndex >= m_Track.GetSize()) {
				if (!m_isSkipping) {
					_AllTrackNoteOff();
					m_MsgTrans.PostPlayTime((unsigned long)(m_CurPlayTime/1000000), m_TotalTickTime);
					m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_STOP);
					m_Status = StatusStop;
				}
				*pIsContinue = false;
				break;
			}

			//次イベント取得
			m_Track.GetDataSet(m_PlayIndex, &deltaTime, &m_Event, &m_PortNo);
		}
		//定期通知のためイベント発生時刻を記憶する
		//定期通知は厳密な精度を必要としないため1msec未満は無視する
		m_PrevEventTime = (unsigned long long)m_NextEventTime;

		//次イベント送信位置を算出
		m_NextEventTime += _ConvTick2TimeNanosec(deltaTime);
		m_PrevDeltaTime = deltaTime;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 演奏位置更新
//******************************************************************************
int SMSequencer::_UpdatePlayPosition()
{
	int result = 0;
	unsigned long long curTime = 0;
	unsigned long long diffTime = 0;
	unsigned long diffTickTime = 0;
	unsigned long nextBarTickTime = 0;
	unsigned long long ntcSpan = 0;

	curTime = _GetCurTimeInNano();

	//前回タイマーからの経過時間を利用して演奏時間を更新
	//  起動後から49日をまたぐケースでもこの計算で問題ない
	diffTime = curTime - m_PrevTimerTime;

	//前回タイマーからの経過時間を利用して演奏時間を更新
	if (m_isSkipping) {
		//スキップ中の場合は仮想的に5msec.経過させる
		diffTime = 5 * 1000000;
	}
	else {
		//スキップ中でなければ実際の経過時間を算出する
		diffTime = curTime - m_PrevTimerTime;
	}

	//再生スピードを反映（n倍速）
	if (m_PlaybackSpeed == 1) {
		diffTime = (unsigned long long)((double)diffTime * m_PlaySpeedRatio);
	}
	else {
		diffTime = diffTime * m_PlaybackSpeed;
	}

	m_CurPlayTime += diffTime;
	m_PrevTimerTime = curTime;

	//前回イベント発生からの経過時間をチックタイムに換算
	//  変換誤差が生じるが誤差を蓄積させないため問題ない
	diffTickTime = _ConvTimeNanosec2Tick(m_CurPlayTime - m_PrevEventTime);

	//曲先頭からのチックタイム合計
	//m_TotalTickTimeはイベント発生時にのみ更新するためここでは書き換えない
	m_TotalTickTimeTemp = m_TotalTickTime + diffTickTime;

	//通知時間に到達したら演奏時間を通知する
	if ((m_NextNtcTime <= m_CurPlayTime) && (!m_isSkipping)) {
		m_MsgTrans.PostPlayTime((unsigned long)(m_CurPlayTime/1000000), m_TotalTickTimeTemp);
		//通知間隔は60FPS表示を考慮して1,000,000,000/120[nanosec]×再生スピードとする
		//TODO: 外部から間隔を指定できるようにする
		ntcSpan = (unsigned long long)(1000000000.0 * m_PlaySpeedRatio / 120.0);
		m_NextNtcTime = m_CurPlayTime - (m_CurPlayTime % ntcSpan) + ntcSpan;
	}

	//小節番号更新の確認
	nextBarTickTime = m_PrevBarTickTime + m_TickTimeOfBar;
	if (nextBarTickTime <= m_TotalTickTimeTemp) {
		m_CurBarNo++;
		m_PrevBarTickTime = nextBarTickTime;
		if (!m_isSkipping) {
			m_MsgTrans.PostBar(m_CurBarNo);
		}
	}

//EXIT:;
	return result;
}

//******************************************************************************
// チックタイムから実時間への変換（ナノ秒）
//******************************************************************************
double SMSequencer::_ConvTick2TimeNanosec(
		unsigned long tickTime
	)
{
	double timeNanosec = 0;
	
	//(1) 四分音符あたりの分解能 division
	//    例：48
	//(2) トラックデータのデルタタイム delta
	//    分解能の値を用いて表現する時間差
	//    分解能が48でデルタタイムが24なら八分音符分の時間差
	//(3) テンポ設定（マイクロ秒） tempo
	//    四分音符の実時間間隔
	//
	// デルタタイムに対応する実時間間隔（ミリ秒）
	//  = (delta / division) * tempo / 1000
	//  = (delta * tempo) / (division * 1000)
	
	timeNanosec = ((double)tickTime * (double)m_Tempo) * 1000.0 / ((double)m_TimeDivision);
	
	return timeNanosec;
}

//******************************************************************************
// 実時間（ナノ秒）からチックタイムへの変換
//******************************************************************************
unsigned long SMSequencer::_ConvTimeNanosec2Tick(
		unsigned long long timeNanosec
	)
{
	unsigned long tickTime = 0;
	unsigned long long a = 0;
	unsigned long long b = 0;
	
	a = timeNanosec * m_TimeDivision / 1000;
	b = a / m_Tempo;
	tickTime = (unsigned long)b;
	
	return tickTime;
}

//******************************************************************************
// イベント送信処理
//******************************************************************************
int SMSequencer::_OutputMIDIEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;

	//MIDIイベント送信
	if (pEvent->GetType() == SMEvent::EventMIDI) {
		SMEventMIDI eventMIDI;
		eventMIDI.Attach(pEvent);
		result = _SendMIDIEvent(portNo, &eventMIDI);
		if (result != 0) goto EXIT;
	}
	//SysExイベント送信
	else if (pEvent->GetType() == SMEvent::EventSysEx) {
		SMEventSysEx eventSysEx;
		eventSysEx.Attach(pEvent);
		result = _SendSysExEvent(portNo, &eventSysEx);
		if (result != 0) goto EXIT;
	}
	//メタイベント送信
	else if (pEvent->GetType() == SMEvent::EventMeta) {
		SMEventMeta eventMeta;
		eventMeta.Attach(pEvent);
		result = _SendMetaEvent(portNo, &eventMeta);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDIイベント送信
//******************************************************************************
int SMSequencer::_SendMIDIEvent(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent
	)
{
	int result = 0;
	unsigned long msg = 0;
	bool isFiltered = false;

	//メッセージ取得
	result = pMIDIEvent->GetMIDIOutShortMsg(&msg);
	if (result != 0) goto EXIT;

	//MIDIイベントフィルタ
	result = _FilterMIDIEvent(portNo, pMIDIEvent, &isFiltered);
	if (result != 0) goto EXIT;

	//MIDIイベント送信
	if (!isFiltered) {
		//メッセージ出力：出力完了まで制御が戻らない
		result = m_OutDevCtrl.SendShortMsg(portNo, msg);
		if (result != 0) goto EXIT;

		//MIDIイベントメッセージポスト
		result =  m_EventWatcher.WatchEventMIDI(portNo, pMIDIEvent);
		if (result != 0) goto EXIT;
	}

	//ノートONをカウント
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOn) {
		m_NotesCount++;
	}

	//コントロールチェンジ監視処理
	//  ピッチベンド感度を拾うためRPNを監視する
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::ControlChange) {
		result = m_EventWatcher.WatchEventControlChange(portNo, pMIDIEvent);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// SysExイベント送信
//******************************************************************************
int SMSequencer::_SendSysExEvent(
		unsigned char portNo,
		SMEventSysEx* pSysExEvent
	)
{
	int result = 0;
	unsigned char* pVarMsg = NULL;
	unsigned long size = 0;

	//メッセージ取得
	pSysExEvent->GetMIDIOutLongMsg(&pVarMsg, &size);

	//メッセージ出力：出力完了まで制御が戻らない
	result = m_OutDevCtrl.SendLongMsg(portNo, pVarMsg, size);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// メタイベント送信
//******************************************************************************
int SMSequencer::_SendMetaEvent(
		unsigned char portNo,
		SMEventMeta* pMetaEvent
	)
{
	int result = 0;

	//メタイベントはMIDIデバイスに送信しない

	//テンポ情報
	if (pMetaEvent->GetType() == 0x51) {
		//デルタタイム計算に反映
		m_Tempo = pMetaEvent->GetTempo();
		if (m_Tempo == 0) {
			//データ異常
			result = YN_SET_ERR("Invalid data found.", 0, 0);
			goto EXIT;
		}

		//通知
		if (!m_isSkipping) {
			m_MsgTrans.PostTempo(m_Tempo);
		}
	}

	//拍子記号
	if (pMetaEvent->GetType() == 0x58) {
		//分子分母を取得
		unsigned long numerator = 0;
		unsigned long denominator = 0;
		pMetaEvent->GetTimeSignature(&numerator, &denominator);
		if (denominator == 0) {
			//データ異常
			result = YN_SET_ERR("Invalid data found.", numerator, denominator);
			goto EXIT;
		}
		m_BeatNumerator = numerator;
		m_BeatDenominator = denominator;

		//通知
		if (!m_isSkipping) {
			m_MsgTrans.PostBeat((unsigned short)numerator, (unsigned short)denominator);
		}

		//1小節あたりのチックタイムを更新
		m_TickTimeOfBar = (numerator * m_TimeDivision * 4) / denominator;

		//拍子記号更新のため1小節目開始地点として通知
		if (m_PrevBarTickTime != m_TotalTickTime) {
			m_CurBarNo++;
			m_PrevBarTickTime = m_TotalTickTime;
			if (!m_isSkipping) {
				m_MsgTrans.PostBar(m_CurBarNo);
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ユーザ要求処理
//******************************************************************************
int SMSequencer::_ProcUserRequest(
		BOOL* pIsContinue
	)
{
	int result = 0;

	if (m_UserRequest == RequestNone) goto EXIT;

	//全トラックノートオフ
	result = _AllTrackNoteOff();
	if (result != 0) goto EXIT;

	*pIsContinue = false;

	//一時停止を要求された場合
	if (m_UserRequest == RequestPause) {
		m_Status = StatusPause;
		m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_PAUSE);
	}

	//停止を要求された場合
	if (m_UserRequest == RequestStop) {
		m_Status = StatusStop;
		m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_STOP);
	}

	//スキップを要求された場合
	if (m_UserRequest == RequestSkip) {
		*pIsContinue = true;
		result = _ProcSkip(m_SkipTargetTime, pIsContinue);
		if (result != 0) goto EXIT;
	}

	m_UserRequest = RequestNone;

EXIT:;
	return result;
}

//******************************************************************************
// 全トラックノートオフ
//******************************************************************************
int SMSequencer::_AllTrackNoteOff()
{
	int result = 0;

	result = m_OutDevCtrl.NoteOffAll();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 現在時刻取得（ナノ秒）
//******************************************************************************
unsigned long long SMSequencer::_GetCurTimeInNano()
{
	return ((unsigned long long)(timeGetTime()) * 1000000);
}

//******************************************************************************
// 再生開始パラメータ初期化
//******************************************************************************
int SMSequencer::_InitializeParamsOnPlayStart()
{
	int result = 0;
	unsigned long deltaTime = 0;

	//演奏位置を曲の先頭に戻す
	m_PlayIndex = 0;
	result = m_Track.GetDataSet(m_PlayIndex, &deltaTime, &m_Event, &m_PortNo);
	if (result != 0) goto EXIT;

	m_PrevTimerTime = _GetCurTimeInNano();
	m_CurPlayTime = 0;
	m_PrevEventTime = 0;
	m_NextEventTime = _ConvTick2TimeNanosec(deltaTime);
	m_NextNtcTime = 0;
	m_PrevDeltaTime = deltaTime;
	m_TotalTickTime = 0;
	m_TotalTickTimeTemp = 0;
	m_CurBarNo = 1;
	m_PrevBarTickTime = 0;
	m_NotesCount = 0;

	//イベントウォッチャー初期化
	result = m_EventWatcher.Initialize(&m_MsgTrans);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// MIDIイベントキャッシュクリア
//******************************************************************************
void SMSequencer::_ClearMIDIEventCache()
{
	unsigned long portNo = 0;
	unsigned long chNo = 0;

	for (portNo = 0; portNo < SM_MAX_PORT_NUM; portNo++) {
		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
			m_CachePitchBend[portNo][chNo][0] = 0xFF;
			m_CachePitchBend[portNo][chNo][1] = 0xFF;
			m_CacheCC001_Modulation[portNo][chNo] = 0xFF;
			m_CacheCC007_Volume[portNo][chNo] = 0xFF;
			m_CacheCC010_Panpot[portNo][chNo] = 0xFF;
			m_CacheCC011_Expression[portNo][chNo] = 0xFF;
		}
	}

	return;
}

//******************************************************************************
// MIDIイベントフィルタ
//******************************************************************************
int SMSequencer::_FilterMIDIEvent(
		unsigned char portNo,
		SMEventMIDI* pMIDIEvent,
		bool* pIsFiltered
	)
{
	int result = 0;
	unsigned char* pData = NULL;
	unsigned long shortMsg = 0;
	unsigned long size = 0;
	unsigned char chNo = 0;
	unsigned char ccNo = 0;
	unsigned char ccValue = 0;

	*pIsFiltered = false;

	//スキップ中のみフィルタする
	if (!m_isSkipping) goto EXIT;

	chNo = pMIDIEvent->GetChNo();

	//ノートON/OFFは送信しない
	if ((pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOff) ||
		(pMIDIEvent->GetChMsg() == SMEventMIDI::NoteOn)) {
		*pIsFiltered = true;
	}

	//ピッチベンドは送信しない
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::PitchBend) {
		*pIsFiltered = true;
		result = pMIDIEvent->GetMIDIOutShortMsg(&shortMsg);
		if (result != 0) goto EXIT;
		
		//ピッチベンドの値を記憶する：En dl dm 第2,3バイト目を参照
		pData = (unsigned char*)(&shortMsg);
		m_CachePitchBend[portNo][chNo][0] = pData[1];
		m_CachePitchBend[portNo][chNo][1] = pData[2];
	}

	//コントロールチェンジの一部は送信しない
	if (pMIDIEvent->GetChMsg() == SMEventMIDI::ControlChange) {
		ccNo = pMIDIEvent->GetCCNo();
		ccValue = pMIDIEvent->GetCCValue();

		//CC#1 モジュレーション
		if (ccNo == 1) {
			*pIsFiltered = true;
			m_CacheCC001_Modulation[portNo][chNo] = ccValue;
		}
		//CC#7 ボリューム
		else if (ccNo == 7) {
			*pIsFiltered = true;
			m_CacheCC007_Volume[portNo][chNo] = ccValue;
		}
		//CC#10 パンポット
		else if (ccNo == 10) {
			*pIsFiltered = true;
			m_CacheCC010_Panpot[portNo][chNo] = ccValue;
		}
		//CC#11 エクスプレッション
		else if (ccNo == 11) {
			*pIsFiltered = true;
			m_CacheCC011_Expression[portNo][chNo] = ccValue;
		}
		//CC#121 リセットオールコントローラ
		else if (ccNo = 121) {
			//クリア対象パラメータのキャッシュを破棄する
			m_CachePitchBend[portNo][chNo][0] = 0xFF;
			m_CachePitchBend[portNo][chNo][1] = 0xFF;
			m_CacheCC001_Modulation[portNo][chNo] = 0xFF;
			//対象外 m_CacheCC007_Volume[portNo][chNo] = 0xFF;
			//対象外 m_CacheCC010_Panpot[portNo][chNo] = 0xFF;
			m_CacheCC011_Expression[portNo][chNo] = 0xFF;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDIイベントキャッシュ送信
//******************************************************************************
int SMSequencer::_SendMIDIEventCache()
{
	int result = 0;
	unsigned long index = 0;
	unsigned char portNo = 0;
	unsigned char chNo = 0;
	unsigned char pitchBend[2];
	unsigned char ccValue = 0;

	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		portNo = (unsigned char)index;
		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
			//ピッチベンド
			pitchBend[0] = m_CachePitchBend[portNo][chNo][0];
			pitchBend[1] = m_CachePitchBend[portNo][chNo][1];
			if (pitchBend[0] < 0xFF) {
				result = _SendMIDIEventPitchBend(portNo, chNo, pitchBend);
				if (result != 0) goto EXIT;
			}
			//CC#1 モジュレーション
			ccValue = m_CacheCC001_Modulation[portNo][chNo];
			if (ccValue < 0x80) {
				result = _SendMIDIEventCC(portNo, chNo, 1, ccValue);
				if (result != 0) goto EXIT;
			}
			//CC#7 ボリューム
			ccValue = m_CacheCC007_Volume[portNo][chNo];
			if (ccValue < 0x80) {
				result = _SendMIDIEventCC(portNo, chNo, 7, ccValue);
				if (result != 0) goto EXIT;
			}
			//CC#10 パンポット
			ccValue = m_CacheCC010_Panpot[portNo][chNo];
			if (ccValue < 0x80) {
				result = _SendMIDIEventCC(portNo, chNo, 10, ccValue);
				if (result != 0) goto EXIT;
			}
			//CC#11 エクスプレッション
			ccValue = m_CacheCC011_Expression[portNo][chNo];
			if (ccValue < 0x80) {
				result = _SendMIDIEventCC(portNo, chNo, 11, ccValue);
				if (result != 0) goto EXIT;
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDIイベントキャッシュ送信：ピッチベンド
//******************************************************************************
int SMSequencer::_SendMIDIEventPitchBend(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char* pPtichBend
	)
{
	int result = 0;
	SMEvent event;
	SMEventMIDI eventMIDI;

	//MIDIイベントデータ作成
	event.SetMIDIData(0xE0 | chNo, pPtichBend, 2);
	eventMIDI.Attach(&event);

	//MIDIイベント送信
	result = _SendMIDIEvent(portNo, &eventMIDI);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// MIDIイベントキャッシュ送信：コントロールチェンジ
//******************************************************************************
int SMSequencer::_SendMIDIEventCC(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char ccNo,
		unsigned char ccValue
	)
{
	int result = 0;
	unsigned char data[2];
	SMEvent event;
	SMEventMIDI eventMIDI;

	//MIDIイベントデータ作成
	data[0] = ccNo;
	data[1] = ccValue;
	event.SetMIDIData(0xB0 | chNo, data, 2);
	eventMIDI.Attach(&event);

	//MIDIイベント送信
	result = _SendMIDIEvent(portNo, &eventMIDI);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// スキップ処理
//******************************************************************************
int SMSequencer::_ProcSkip(
		unsigned long long targetTimeInNanoSec,
		BOOL* pIsContinue
	)
{
	int result = 0;
	unsigned long long startPlayTime = 0;
	unsigned long startTickTime = 0;
	unsigned long endTickTime = 0;

	if (m_Status != StatusPlay) goto EXIT;

	startPlayTime = m_CurPlayTime;
	startTickTime = m_TotalTickTimeTemp;

	//後方スキップの場合
	if (targetTimeInNanoSec < m_CurPlayTime) {
		//再生開始パラメータ初期化
		result = _InitializeParamsOnPlayStart();
		if (result != 0) goto EXIT;
		
		m_MsgTrans.PostSkipStart(SM_SKIP_BACK);
	}
	//前方スキップの場合
	else {
		m_MsgTrans.PostSkipStart(SM_SKIP_FORWARD);
	}

	//MIDIイベントキャッシュクリア
	_ClearMIDIEventCache();

	//指定時刻までMIDIイベントを処理する
	m_isSkipping = true;
	while (*pIsContinue) {
		//スレッドインターバル処理
		result = _IntervalProc(pIsContinue);
		if (result != 0) goto EXIT;
		
		//指定時刻に達したらスキップ終了とする
		if (targetTimeInNanoSec <= m_CurPlayTime) break;
	}
	m_isSkipping = false;

	//キャッシュ送信
	result = _SendMIDIEventCache();
	if (result != 0) goto EXIT;

	//再生時刻移動
	endTickTime = m_TotalTickTimeTemp;
	_SlidePlaybackTime(startPlayTime, startTickTime, endTickTime);

	//スキップ移動先の状態を通知
	m_MsgTrans.PostPlayTime((unsigned long)(m_CurPlayTime/1000000), endTickTime);
	m_MsgTrans.PostTempo(m_Tempo);
	m_MsgTrans.PostBeat((unsigned short)m_BeatNumerator, (unsigned short)m_BeatDenominator);
	m_MsgTrans.PostBar(m_CurBarNo);

	//再生開始時刻を更新
	m_PrevTimerTime = _GetCurTimeInNano();

	//スキップ終了
	m_MsgTrans.PostSkipEnd(m_NotesCount);

	//前方スキップによる再生終了
	if (!(*pIsContinue)) {
		_AllTrackNoteOff();
		m_MsgTrans.PostPlayTime((unsigned long)(m_CurPlayTime/1000000), m_TotalTickTime);
		m_MsgTrans.PostPlayStatus(SM_PLAYSTATUS_STOP);
		m_Status = StatusStop;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 再生時刻移動
//******************************************************************************
void SMSequencer::_SlidePlaybackTime(
		unsigned long long startPlayTime,
		unsigned long startTickTime,
		unsigned long endTickTime
	)
{
	unsigned long i = 0;
	unsigned long tickTime = 0;
	unsigned long tickTimeStep = 0;
	unsigned long waitTimeInMsec = 10;  //10msec.ごとに通知
	unsigned long stepNum = 0;
	bool isRewind = false;

	//再生時刻通知回数
	stepNum = m_MovingTimeSpanInMsec / waitTimeInMsec;

	//チックタイム刻み値
	if (startTickTime > endTickTime) {
		isRewind = true;
		tickTimeStep = (startTickTime - endTickTime) / stepNum;
	}
	else {
		isRewind = false;
		tickTimeStep = (endTickTime - startTickTime) / stepNum;
	}

	//再生時刻移動
	tickTime = startTickTime;
	for (i = 0; i < stepNum; i ++) {
		//再生時刻を通知：チックタイムのみ更新
		if (isRewind) {
			tickTime -= tickTimeStep;
		}
		else {
			tickTime += tickTimeStep;
		}
		m_MsgTrans.PostPlayTime((unsigned long)(startPlayTime/1000000), tickTime);
		
		//待機
		Sleep(waitTimeInMsec);
	}

	return;
}

//******************************************************************************
// タイマー呼び出し
//******************************************************************************
int SMSequencer::_OnTimer()
{
	int result = 0;
	BOOL isContinue = true;

	unsigned long deltaTime = 0;

	//浮動小数点演算精度を倍精度に設定
	//  タイマー開始直後に1回だけ実行する
	if (!(m_FPUCtrl.IsLocked())) {
		result = m_FPUCtrl.Start(SMFPUCtrl::FPUDouble);
		if (result != 0) goto EXIT;
	}

	//スレッドインターバル処理
	result = _IntervalProc(&isContinue);
	if (result != 0) goto EXIT;

	//ユーザリクエストの処理
	if (isContinue) {
		result = _ProcUserRequest(&isContinue);
		if (result != 0) goto EXIT;
	}

	if (!isContinue) {
		timeKillEvent(m_TimerID);
		m_TimerID = NULL;
		m_FPUCtrl.End();
	}

EXIT:;
	return result;
}

//******************************************************************************
// タイマーコールバック関数
//******************************************************************************
void SMSequencer::_TimerCallBack(
		UINT uTimerID,
		UINT uMsg,
		DWORD_PTR dwUser,
		DWORD_PTR dw1,
		DWORD_PTR dw2
	)
{
	int result = 0;

	SMSequencer* pSequencer = (SMSequencer*)dwUser;
	if (pSequencer == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = pSequencer->_OnTimer();
	if (result != 0) goto EXIT;

EXIT:;
	if (result != 0) YN_SHOW_ERR(NULL);
	return;
}

} // end of namespace

