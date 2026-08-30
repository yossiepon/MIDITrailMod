//******************************************************************************
//
// Simple MIDI Library / SMWavetableSynthCtrl
//
// Wavetableシンセサイザ制御クラス
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMCommon.h"
#include "SMEvent.h"
#include "SMEventMIDI.h"
#include "SMMsgQueue.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>


//#pragma warning(disable:4251)

namespace SMIDILib {

//******************************************************************************
// パラメータ定義
//******************************************************************************
//デバイスプロダクト名称
#define SM_WAVETABLE_SYNTH_PRODUCT_NAME         "Internal Wavetable Synthesizer"

//パーカッションチャンネル番号（0始まり）
#define SM_WAVETABLE_SYNTH_PERCUSSION_CH		(9)

//メッセージキューサイズ
#define SM_WAVETABLE_SYNTH_MESSAGE_QUEUE_SIZE	(2048)

//Wabetableシンセサイザパラメータ
typedef struct {
//	int sampleRate;
	int maxVoices;
	int sustain;
} SM_WAVETABLE_SYNTH_PARAM;


//******************************************************************************
// Wavetableシンセサイザ制御クラス
//******************************************************************************
class SMIDILIB_API SMWavetableSynthCtrl
{
public:
	
	//デフォルトパラメータ取得
	static SM_WAVETABLE_SYNTH_PARAM GetDefaultParam();
	
	//コンストラクタ／デストラクタ
	SMWavetableSynthCtrl();
	virtual ~SMWavetableSynthCtrl();
	
	//初期化
	int Initialize(const WCHAR* pWavetableFilePath, SM_WAVETABLE_SYNTH_PARAM synthParam);
	
	//破棄
	void Release();
	
	//デバイスプロダクト名称取得
	int GetDevProductName(std::string& name);
	
//	//デバイス表示名称取得
//	int GetDevDisplayName(std::string& name);
	
	//デバイスオープン
	int Open();
	
	//デバイスクローズ
	int Close();
	
	//オーディオセッションアクティブ設定
	int SetAudioSessionActive(BOOL isActive);
	
	//MIDI出力メッセージ送信
	int SendShortMsg(unsigned char* pMsg, unsigned int size);
	int SendLongMsg(unsigned char* pMsg, unsigned int size);
	int NoteOffAll();
	int SoundOffAll();
	
	//オーディオ出力処理
	int AudioOutputProc();

private:
	
	//Wabetableシンセサイザパラメータ
	SM_WAVETABLE_SYNTH_PARAM m_SynthParam;
	
	//WASAPI制御
	IMMDevice* m_pAudioDevice;
	IAudioClient* m_pAudioClient;
	IAudioRenderClient* m_pRenderClient;
	HANDLE m_hAudioEvent;
	int m_SampleRate;
	
	//オーディオ処理スレッド制御
	HANDLE m_hThread;
	unsigned int m_ThreadId;
	bool m_isRequestedStop;
	
	//ステータス
	bool m_isOpened;
	
	//メッセージキュー
	SMMsgQueue m_MsgQueue;
	
	//調査用パラメータ
	unsigned int m_FrameCount;
	DWORD m_PrevStartTimeMsec;
	DWORD m_ProcTimeMaxMsec;
	DWORD m_IntervalTimeMaxMsec;
	
private:
	
	void _InitializeParams();
	int _SetupChannels();
	int _CreateAuditoClient();
	void _ReleaseAudioClient();
	int _SetMIDIEventToTSF(SMEventMIDI* pEventMIDI);
	int _AudioRenderProc(UINT32 bufferFrameCount);
	
};

} // end of namespace

//#pragma warning(default:4251)

