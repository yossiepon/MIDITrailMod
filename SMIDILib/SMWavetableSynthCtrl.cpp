//******************************************************************************
//
// Simple MIDI Library / SMWavetableSynthCtrl
//
// Wavetableシンセサイザ制御クラス
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

//MEMO:
//  出力用オーディオクライアントの制御と、TinySoundFontへのMIDIメッセージ登録処理を実装する。
//  TinySoundFontにて、SoundFontファイル(.sf2)の読み込みと、波形レンダリングを実行する。

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMWavetableSynthCtrl.h"
#include "SMCommon.h"
#include <process.h>
#include <mmsystem.h>
#include <Functiondiscoverykeys_devpkey.h>

using namespace YNBaseLib;


//******************************************************************************
// TinySoundFont 定義
//******************************************************************************
//TinySoundFont実装
#define TSF_IMPLEMENTATION
#include "../SoundLib/TinySoundFont/tsf.h"

//TinySoundFontポインタ
static tsf* g_pTinySoundFont;


namespace SMIDILib {

//******************************************************************************
// オーディオ出力スレッド関数
//******************************************************************************
unsigned __stdcall AudioOutputThread(void* pArguments);

//******************************************************************************
// デフォルトパラメータ取得
//******************************************************************************
SM_WAVETABLE_SYNTH_PARAM SMWavetableSynthCtrl::GetDefaultParam()
{
	SM_WAVETABLE_SYNTH_PARAM param;
	
	//パラメータデフォルト値
	param.maxVoices     = 512;
	param.sustain       = 1; //有効
	
	return param;
}

//******************************************************************************
// コンストラクタ
//******************************************************************************
SMWavetableSynthCtrl::SMWavetableSynthCtrl()
{
	_InitializeParams();
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMWavetableSynthCtrl::~SMWavetableSynthCtrl()
{
	Release();
}

//******************************************************************************
// パラメータ初期化
//******************************************************************************
void SMWavetableSynthCtrl::_InitializeParams()
{
	//TinySoundFont制御
	g_pTinySoundFont = NULL;
	m_isOpened = false;
	m_FrameCount = 0;
	m_PrevStartTimeMsec = 0;
	m_ProcTimeMaxMsec = 0;
	m_IntervalTimeMaxMsec = 0;
	memset(&m_SynthParam, 0, sizeof(SM_WAVETABLE_SYNTH_PARAM));
	
	//WASAPI制御
	m_pAudioDevice = NULL;
	m_pAudioClient = NULL;
	m_pRenderClient = NULL;
	m_hAudioEvent = NULL;
	m_SampleRate = 0;
	
	//スレッド制御
	m_hThread = NULL;
	m_ThreadId = 0;
	m_isRequestedStop = false;
}

//******************************************************************************
// 初期化
//******************************************************************************
int SMWavetableSynthCtrl::Initialize(
		const WCHAR* pWavetableFilePath,
		SM_WAVETABLE_SYNTH_PARAM synthParam
	)
{
	int result = 0;
	int tsfresult = 0;
	char logbuf[256];
	
	OutputDebugString(_T("SMWavetableSynthCtrl::Initialize\n"));
	
	Release();
	
	m_SynthParam = synthParam;
	
	//オーディオクライアント生成
	result = _CreateAuditoClient();
	if (result != 0) goto EXIT;
	
	//メッセージキュー初期化
	result = m_MsgQueue.Initialize(SM_WAVETABLE_SYNTH_MESSAGE_QUEUE_SIZE);
	if (result != 0) goto EXIT;
	
	//SoundFontファイル読み込みと出力設定
	if (pWavetableFilePath != NULL) {
		OutputDebugStringW(L"WavetableFilePath: ");
		OutputDebugStringW(pWavetableFilePath);
		OutputDebugStringW(L"\n");
		
		//TinySoundFontにSoundFontファイル読み込みを依頼
		g_pTinySoundFont = tsf_load_filename(pWavetableFilePath);
		if (g_pTinySoundFont == NULL) {
			OutputDebugString(_T("tsf_load_filename failed.\n"));
			//ログ出力のみで処理は続行
		}
	}
	
	//TinySoundFont初期設定
	if (g_pTinySoundFont != NULL) {
		//TinySoundFont 出力設定：戻り値なし
		tsf_set_output(
				g_pTinySoundFont,				//TinySoundFontポインタ
				TSF_STEREO_INTERLEAVED,			//出力モード：ステレオ
				m_SampleRate,					//サンプル周波数
				0								//ボリュームゲイン
			);
		
		//TinySoundFont 最大同時発音数登録
		if (m_SynthParam.maxVoices > 0) {
			tsfresult = tsf_set_max_voices(g_pTinySoundFont, m_SynthParam.maxVoices);
			if (tsfresult == 0) {
				sprintf_s(logbuf, 256, "tsf_set_max_voices failed. %d\n", m_SynthParam.maxVoices);
				OutputDebugString(logbuf);
				//ログ出力のみで処理は続行
			}
		}
		
		//TinySoundFont チャンネルセットアップ
		result = _SetupChannels();
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// チャンネルセットアップ
//******************************************************************************
int SMWavetableSynthCtrl::_SetupChannels()
{
	int result = 0;
	int tsfresult = 0;
	int chNo = 0;
	int presetNumber = 0;
	int flagMIDIDrums = 0;
	char logbuf[256];
	
	//チャンネル別にプリセット番号を設定
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		presetNumber = 0;
		flagMIDIDrums = 0;
		if (chNo == SM_WAVETABLE_SYNTH_PERCUSSION_CH) {
			flagMIDIDrums = 1;
		}
		tsfresult = tsf_channel_set_presetnumber(g_pTinySoundFont, chNo, presetNumber, flagMIDIDrums);
		if (tsfresult == 0) {
			sprintf_s(logbuf, 256, "tsf_channel_set_presetnumber failed. ChNo:%d\n", chNo);
			OutputDebugString(logbuf);
			//ログ出力のみで処理は続行
		}
	}
	
	return result;
}

//******************************************************************************
// 破棄
//******************************************************************************
void SMWavetableSynthCtrl::Release()
{
	//デバイスクローズ：オーディオ出力停止
	Close();
	
	//TyinySoundFont破棄
	if (g_pTinySoundFont != NULL) {
		tsf_close(g_pTinySoundFont);
		g_pTinySoundFont = NULL;
	}
	
	//オーディオクライアント破棄
	_ReleaseAudioClient();
	
	//パラメータ初期化
	_InitializeParams();
	
	return;
}

//******************************************************************************
// デバイスプロダクト名取得
//******************************************************************************
int SMWavetableSynthCtrl::GetDevProductName(
		std::string& name
	)
{
	int result = 0;
	
	name = SM_WAVETABLE_SYNTH_PRODUCT_NAME;
	
	return result;
}

//******************************************************************************
// デバイスオープン
//******************************************************************************
int SMWavetableSynthCtrl::Open()
{
	int result = 0;
	int errNo = 0;
	
	OutputDebugString(_T("SMWavetableSynthCtrl::Open\n"));
	
	if (m_isOpened) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	m_MsgQueue.Clear();
	m_isRequestedStop = false;
	
	//オーディオ出力スレッド起動
	m_hThread = (HANDLE)_beginthreadex(
							NULL,				//セキュリティ属性
							0,					//スタックサイズ：メインスレッドと同じ
							AudioOutputThread,	//開始アドレス
							this,				//引数リスト
							0,					//スレッド初期状態：即時実行
							&m_ThreadId			//スレッドID
						);
	if (m_hThread == 0) {
		errNo = errno;
		result = YN_SET_ERR("Windows API error.", errNo, 0);
		goto EXIT;
	}
	
	m_isOpened = true;
	
EXIT:;
	return result;
}

//******************************************************************************
// デバイスクローズ
//******************************************************************************
int SMWavetableSynthCtrl::Close()
{
	int result = 0;
	DWORD waitResult = 0;
	
	OutputDebugString(_T("SMWavetableSynthCtrl::Close\n"));
	
	if (!m_isOpened) goto EXIT;
	
	//オーディオ出力スレッド終了フラグON
	m_isRequestedStop = true;

	//オーディオ出力スレッド終了待ち合わせ（タイムアウト10秒）
	waitResult = WaitForSingleObject(m_hThread, 1000 * 10);
	if (waitResult == WAIT_OBJECT_0) {
		//シグナル状態：スレッド終了待ち合わせ完了
	}
	else if (waitResult == WAIT_TIMEOUT) {
		//タイムアウト発生：異常のため処理終了
		result = YN_SET_ERR("Program error.", waitResult, 0);
		goto EXIT;
	}
	else {
		//失敗：WAIT_FAILEDまたはWAIT_ABANDONED
		result = YN_SET_ERR("Windows API error.", waitResult, 0);
		goto EXIT;
	}

EXIT:;
	if (m_hThread != NULL) {
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	m_isOpened = false;
	m_isRequestedStop = false;
	return result;
}

//******************************************************************************
// オーディオセッションアクティブ設定
//******************************************************************************
int SMWavetableSynthCtrl::SetAudioSessionActive(BOOL isActive)
{
	int result = 0;
	char logbuf[256];
	
	sprintf_s(logbuf, 256, "SMWavetableSynthCtrl::SetAudioSessionActive %d\n", (int)isActive);
	OutputDebugString(logbuf);
	
	//アクティブ化
	if (isActive) {
		//すでにアクティブ化されている場合は何もしない
		if (m_isOpened) goto EXIT;
		
		//デバイスオープン：オーディオ出力開始
		result = Open();
		if (result != 0) goto EXIT;
	}
	//非アクティブ化
	else {
		//すでに非アクティブ化されている場合は何もしない
		if (!m_isOpened) goto EXIT;
		
		//デバイスクローズ：：オーディオ出力停止
		result = Close();
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDIメッセージ送信
//******************************************************************************
int SMWavetableSynthCtrl::SendShortMsg(
		unsigned char* pMsg,
		unsigned int size
	)
{
	int result = 0;
	unsigned int param1 = 0;
	unsigned int param2 = 0;
	
	if (size > 3) {
		result = YN_SET_ERR("Program error.", size, 0);
		goto EXIT;
	}
	
	memcpy((unsigned char*)&param1, pMsg, size);
	param2 = size;
	
	//メッセージをポスト：コールバック関数への受け渡し
	result = m_MsgQueue.PostMessage(param1, param2);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// システムエクスクルーシブ送信
//******************************************************************************
int SMWavetableSynthCtrl::SendLongMsg(
		unsigned char* pMsg,
		unsigned int size
	)
{
	//システムエクスクルーシブには対応しない
	return 0;
}

//******************************************************************************
// 全ノートオフ
//******************************************************************************
int SMWavetableSynthCtrl::NoteOffAll()
{
	int result = 0;
	unsigned char i = 0;
	unsigned char msg[3];
	
	//全トラックノートオフ
	for (i = 0; i < SM_MAX_CH_NUM; i++) {
		msg[0] = 0xB0 | i;
		msg[1] = 0x7B;
		msg[2] = 0x00;
		result = SendShortMsg(msg, 3);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// 全サウンドオフ
//******************************************************************************
int SMWavetableSynthCtrl::SoundOffAll()
{
	int result = 0;
	unsigned char i = 0;
	unsigned char msg[3];
	
	//全トラックサウンドオフ
	for (i = 0; i < SM_MAX_CH_NUM; i++) {
		msg[0] = 0xB0 | i;
		msg[1] = 0x78;
		msg[2] = 0x00;
		result = SendShortMsg(msg, 3);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// オーディオクライアント生成
//******************************************************************************
int SMWavetableSynthCtrl::_CreateAuditoClient()
{
	int result = 0;
	HRESULT hresult = S_OK;
	IMMDeviceEnumerator* pDeviceEnumerator = NULL;
	WAVEFORMATEX wf;
	WAVEFORMATEX* pWaveFormat = NULL;
	WAVEFORMATEX* pClosestMatchWaveFormat = NULL;
	WAVEFORMATEXTENSIBLE* pWaveFormatEx = NULL;
	IPropertyStore* pPropertyStore = NULL;
	PROPVARIANT propVariant;
	REFERENCE_TIME hnsDefaultPeriod = 0;
	REFERENCE_TIME hnsMinimumPeriod = 0;
	char logbuf[256];
	wchar_t wlogbuf[256];

	//COMライブラリ初期化
	hresult = CoInitializeEx(
					NULL,					//予約
					COINIT_MULTITHREADED	//初期化オプション
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}

	//オーディオデバイス一覧取得
	hresult = CoCreateInstance(
					__uuidof(MMDeviceEnumerator),	//CLSID：オーディオデバイス列挙
					NULL,							//集合オブジェクトインターフェース
					CLSCTX_INPROC_SERVER,			//実行コンテキスト：同じプロセスで実行されるDLL
					__uuidof(IMMDeviceEnumerator),	//オブジェクト通信に使用するインターフェースの識別子
					(void**)&pDeviceEnumerator		//インターフェースポインタ受取先
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}

	//デフォルトオーディオエンドポイント取得
	hresult = pDeviceEnumerator->GetDefaultAudioEndpoint(
					eRender,		//データフロー方向：レンダリングデバイス
					eMultimedia,	//デバイスロール：オーディオ コンテンツ再生 #その他：eConsole, eCommunications
					&m_pAudioDevice	//エンドポイントオブジェクト
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}

	//参考情報ログ出力：デバイスフレンドリーネーム
	//プロパティストア取得
	hresult = m_pAudioDevice->OpenPropertyStore(
						STGM_READ,			//ストレージアクセスモード：読取り専用
						&pPropertyStore		//プロパティストアインターフェース
					);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}
	hresult = pPropertyStore->GetValue(
						PKEY_Device_FriendlyName,	//キー：デバイスフレンドリーネーム
						&propVariant				//プロパティデータ
					);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}
	if (propVariant.vt != VT_EMPTY) {
		swprintf_s(wlogbuf, 256, L"AudioEndpoint: %lS\n", propVariant.pwszVal);
		OutputDebugStringW(wlogbuf);
	}

	//オーディオクライアント生成
	hresult = m_pAudioDevice->Activate(
					__uuidof(IAudioClient),	//インターフェース識別子：オーディオクライアント
					CLSCTX_INPROC_SERVER,	//実行コンテキスト：同じプロセスで実行されるDLL
					NULL,					//アクティブ化パラメータ
					(void**)&m_pAudioClient	//インターフェースポインタ受取先
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}

	//希望するストリーム形式
	memset((void*)&wf, 0, sizeof(WAVEFORMATEX));
	wf.wFormatTag      = WAVE_FORMAT_IEEE_FLOAT;	//波形オーディオ形式   WAVE_FORMAT_PCM
	wf.nChannels       = 2;							//チャンネル数：ステレオ 2チャンネル(L/R)
	wf.nSamplesPerSec  = 44100;						//サンプルレート：44.1 kHz
	wf.wBitsPerSample  = 32;						//サンプルあたりのビット数
	wf.nBlockAlign     = (wf.wBitsPerSample / 8) * wf.nChannels;	//ブロック配置 float 4 bytes * 2チャンネル
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;		//平均データ転送速度(bytes/sec)
	wf.cbSize          = 0;							//追加情報サイズ

	//ストリーム形式サポート確認
	hresult = m_pAudioClient->IsFormatSupported(
					AUDCLNT_SHAREMODE_SHARED,	//共有モード：共有
					&wf,						//ストリーム形式
					&pClosestMatchWaveFormat	//指定したストリーム形式に最も近いサポートされている形式の受取先
				);
	if (hresult == S_OK) {
		//成功：指定したストリーム形式をサポートする
		pWaveFormat = &wf;
	}
	else if (hresult == S_FALSE) {
		//成功：指定した形式に最も近い一致
		// -> 許容できるストリーム形式であることを確認
		
		//浮動小数点形式PCMデータの確認
		if (pClosestMatchWaveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
			//拡張情報を参照
			pWaveFormatEx = (WAVEFORMATEXTENSIBLE*)pClosestMatchWaveFormat;
			if (!IsEqualGUID(pWaveFormatEx->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
				//IEEE浮動小数点形式PCMデータをサポートしていない
				result = YN_SET_ERR("The audio device does not support WAVE_FORMAT_IEEE_FLOAT.", 0, 0);
				goto EXIT;
			}
		}
		else if (pClosestMatchWaveFormat->wFormatTag != WAVE_FORMAT_IEEE_FLOAT) {
			//IEEE浮動小数点形式PCMデータをサポートしていない
			result = YN_SET_ERR("The audio device does not support WAVE_FORMAT_IEEE_FLOAT.", pClosestMatchWaveFormat->wFormatTag, 0);
			goto EXIT;
		}
		
		//チャンネル数の確認
		if (pClosestMatchWaveFormat->nChannels != 2) {
			//ステレオ2チャンネルをサポートしていない
			result = YN_SET_ERR("The audio device does not support 2 channels.", pClosestMatchWaveFormat->nChannels, 0);
			goto EXIT;
		}
		
		//サンプル数の確認
		if (pClosestMatchWaveFormat->wBitsPerSample != 32) {
			//IEEE浮動小数点形式PCMデータであるにも関わらず32bitでなければ異常
			result = YN_SET_ERR("Invalid data found.", pClosestMatchWaveFormat->wBitsPerSample, 0);
			goto EXIT;
		}
		
		//提案されたストリーム形式を採用する
		pWaveFormat = pClosestMatchWaveFormat;
	}
	else if (hresult == AUDCLNT_E_UNSUPPORTED_FORMAT) {
		//成功：指定した形式は排他モードではサポートされない
		// -> 共有モードでは受け入れられると判断して処理を続行する
		pWaveFormat = &wf;
	}
	else {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}

	//サンプルレート
	m_SampleRate = pWaveFormat->nSamplesPerSec;

	//参考情報ログ出力：デバイス固有の周期
	hresult = m_pAudioClient->GetDevicePeriod(&hnsDefaultPeriod, &hnsMinimumPeriod);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}
	sprintf_s(logbuf, 256, "AudioDevice / DefaultPeriod: %.2f msec MinimumPeriod: %.2f msec\n",
								(float)hnsDefaultPeriod / 10000, (float)hnsMinimumPeriod / 10000);
	OutputDebugString(logbuf);

	//オーディオクライアント初期化
	hresult = m_pAudioClient->Initialize(
					AUDCLNT_SHAREMODE_SHARED,			//共有モード：共有
					AUDCLNT_STREAMFLAGS_EVENTCALLBACK,	//ストリームフラグ：イベント駆動モード
					0,									//バッファ容量：自動割り当て（通常は10ms程度）
														//  100ナノ秒単位で指定する（10msの場合は 10000 * 10）
					0,									//デバイス期間：共有モードでは0
					pWaveFormat,						//ストリーム形式
					NULL								//セッションGUID
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}

	//オーディオ出力制御用イベントハンドル生成
	m_hAudioEvent = CreateEvent(
							NULL,		//セキュリティ属性
							FALSE,		//手動リセット：オフ
							FALSE,		//初期状態：オフ
							NULL		//イベントオブジェクト名
						);
	if (m_hAudioEvent == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//オーディオクライアントにイベントハンドルを登録
	hresult = m_pAudioClient->SetEventHandle(m_hAudioEvent);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}

	//レンダークライアント取得
	hresult = m_pAudioClient->GetService(
					__uuidof(IAudioRenderClient),	//インターフェースID：オーディオレンダークライアント
					(void**)&m_pRenderClient		//インターフェースポインタ受け取り先
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	if (pClosestMatchWaveFormat != NULL) {
		CoTaskMemFree(pClosestMatchWaveFormat);
	}
	if (pPropertyStore != NULL) {
		pPropertyStore->Release();
	}
	if (pDeviceEnumerator != NULL) {
		pDeviceEnumerator->Release();
	}
	return result;
}

//******************************************************************************
// オーディオクライアント破棄
//******************************************************************************
void SMWavetableSynthCtrl::_ReleaseAudioClient()
{
    if (m_hAudioEvent != NULL) {
		CloseHandle(m_hAudioEvent);
		m_hAudioEvent = NULL;
	}
	if (m_pRenderClient != NULL) {
		m_pRenderClient->Release();
		m_pRenderClient = NULL;
	}
	if (m_pAudioClient != NULL) {
		m_pAudioClient->Release();
		m_pAudioClient = NULL;
	}
	if (m_pAudioDevice != NULL) {
		m_pAudioDevice->Release();
		m_pAudioDevice = NULL;
	}
	CoUninitialize();
	
	return;
}

//******************************************************************************
// オーディオ出力処理
//******************************************************************************
int SMWavetableSynthCtrl::AudioOutputProc()
{
	int result = 0;
	HRESULT hresult = 0;
	UINT32 bufferFrameCount = 0;
	DWORD waitResult = 0;
	DWORD startTimeMsec = 0;
	DWORD endTimeMsec = 0;
	DWORD procTimeMsec = 0;
	DWORD intervalTimeMsec = 0;
	char logbuf[256];

	//オーディオクライアントからバッファーサイズ取得
	hresult = m_pAudioClient->GetBufferSize(&bufferFrameCount);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Program error.", hresult, 0);
		goto EXIT;
	}
	
	//オーディオ出力開始
	hresult = m_pAudioClient->Start();
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}

	//オーディオ出力ループ
	while (true) {

		//OSから指示あるまで待機（タイムアウト10秒）
		//  オーディオデバイスのデフォルト設定に依存するが通常の間隔は10msec程度
		waitResult = WaitForSingleObject(m_hAudioEvent, 1000 * 10);
		if (waitResult == WAIT_OBJECT_0) {
			//シグナル状態になったので処理を開始
		}
		else if (waitResult == WAIT_TIMEOUT) {
			//タイムアウト発生：異常のため処理終了
			result = YN_SET_ERR("Program error.", waitResult, 0);
			goto EXIT;
		}
		else {
			//失敗：WAIT_FAILEDまたはWAIT_ABANDONED
			result = YN_SET_ERR("Windows API error.", waitResult, 0);
			goto EXIT;
		}
		
		//オーディオ出力停止を要求された場合はスレッド終了
		if (m_isRequestedStop) {
			break;
		}
		
		//調査用：オーディオ出力処理呼び出し時間間隔が変化したときだけログ出力する
		startTimeMsec = timeGetTime();
		if (m_PrevStartTimeMsec != 0) {
			intervalTimeMsec = startTimeMsec - m_PrevStartTimeMsec;
			if (m_IntervalTimeMaxMsec < intervalTimeMsec) {
				m_IntervalTimeMaxMsec = intervalTimeMsec;
				sprintf_s(logbuf, 256, "AudioOutputProc / interval time max: %d msec\n", m_IntervalTimeMaxMsec);
				OutputDebugString(logbuf);
			}
		}
		m_PrevStartTimeMsec = startTimeMsec;
		
		//オーディオレンダリング処理
		result = _AudioRenderProc(bufferFrameCount);
		if (result != 0) goto EXIT;
		
		//調査用：レンダリング処理時間が最大値を更新したときだけログ出力する
		endTimeMsec = timeGetTime();
		procTimeMsec = endTimeMsec - startTimeMsec;
		if (procTimeMsec > m_ProcTimeMaxMsec) {
			m_ProcTimeMaxMsec = procTimeMsec;
			sprintf_s(logbuf, 256, "AudioOutputProc / proc time max: %d msec\n", m_ProcTimeMaxMsec);
			OutputDebugString(logbuf);
		}
	}
	
	//オーディオ出力終了
	hresult = m_pAudioClient->Stop();
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Windows API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// オーディオレンダリング処理
//******************************************************************************
int SMWavetableSynthCtrl::_AudioRenderProc(UINT32 bufferFrameCount)
{
	int result = 0;
	HRESULT hresult = 0;
	float* pStream = NULL;
	unsigned long sampleBlock = 0;
	unsigned long frameRemainingCount = 0;
	bool isExist = false;
	unsigned long param1 = 0;
	unsigned long param2 = 0;
	unsigned char* pData = NULL;
	unsigned long size = 0;
	SMEvent event;
	SMEventMIDI eventMIDI;
	UINT32 numFramesPadding = 0;
	UINT32 numFramesAvailable = 0;
	BYTE* pBuf = NULL;
	char logbuf[256];

	//バッファサイズ取得
	hresult = m_pAudioClient->GetCurrentPadding(&numFramesPadding);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Program error.", hresult, 0);
		goto EXIT;
	}
	
	//要求フレーム数
	numFramesAvailable = bufferFrameCount - numFramesPadding;
	
	//バッファ位置取得
	hresult = m_pRenderClient->GetBuffer(
						numFramesAvailable,	//要求フレーム数
						&pBuf				//書き込み先アドレス
					);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Program error.", hresult, 0);
		goto EXIT;
	}
	
	//出力バッファにブロック単位で波形を書き込む
	pStream = (float*)pBuf;
	frameRemainingCount = numFramesAvailable;
	for (sampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK;
		 frameRemainingCount;
		 frameRemainingCount -= sampleBlock, pStream += sampleBlock * 2) {

		//ブロックサイズ調整
		if (sampleBlock > frameRemainingCount) {
			sampleBlock = frameRemainingCount;
		}
		
		//メッセージキューに登録されているMIDIイベントをすべてTinySoundFontに登録
		while(true) {
			//メッセージ取り出し
			result = m_MsgQueue.GetMessage(&isExist, &param1, &param2);
			if (result != 0) goto EXIT;
			
			//取り出し終わったら波形レンダリングに進む
			if (!isExist) break;
			
			//パラメータ確認
			pData = (unsigned char*)&param1;
			size = param2;
			if ((size == 0) || (size > 3)) {
				result = YN_SET_ERR("Program error.", param1, param2);
				goto EXIT;
			}
			
			//MIDIイベントデータ作成
			result = event.SetMIDIData(pData[0], &(pData[1]), size - 1);
			if (result != 0) goto EXIT;
			eventMIDI.Attach(&event);
			
			//TinySoundFontにMIDIイベントを登録
			result = _SetMIDIEventToTSF(&eventMIDI);
			if (result != 0) goto EXIT;
		}
		
		//TinySoundFontに波形レンダリングを依頼：戻り値なし
		if (g_pTinySoundFont != NULL) {
			tsf_render_float(
					 g_pTinySoundFont,	//TinySoundFontポインタ
					 pStream,			//出力先バッファ
					 (int)sampleBlock,	//サンプル数
					 0					//ミキシングなし（バッファをクリアする）
				 );
		}
	}

	//調査用：フレームカウントが最大値を更新したときログ出力する
	if (numFramesAvailable > m_FrameCount) {
		m_FrameCount = numFramesAvailable;
		sprintf_s(logbuf, 256, "AudioRenderProc / frame count max: %d\n", m_FrameCount);
		OutputDebugString(logbuf);
	}

	//バッファ領域解放：OS側にデータを渡す
	hresult = m_pRenderClient->ReleaseBuffer(
						numFramesAvailable,	//書き込みフレーム数
						0					//バッファー構成フラグ
					);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("Program error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// TinySoundFontにMIDIイベントを登録
//******************************************************************************
int SMWavetableSynthCtrl::_SetMIDIEventToTSF(SMEventMIDI* pEventMIDI)
{
	int result = 0;
	int tsfresult = 0;
	char logbuf[256];
	float vel = 0.0f;
	
	if (g_pTinySoundFont == NULL) goto EXIT;
	
	//TinySoundFontにMIDIメッセージを登録
	switch(pEventMIDI->GetChMsg()) {
		case SMEventMIDI::None:
			break;
		//ノートオフ
		case SMEventMIDI::NoteOff:
			tsf_channel_note_off(
					g_pTinySoundFont,
					pEventMIDI->GetChNo(),
					pEventMIDI->GetNoteNo());
			//戻り値なし
			break;
		//ノートオン
		case SMEventMIDI::NoteOn:
			//ベロシティの設置値については以下ページを参照
			//https://github.com/schellingb/TinySoundFont/issues/53
			vel = powf(pEventMIDI->GetVelocity() / 127.0f, 2.0f);
			tsfresult = tsf_channel_note_on(
							g_pTinySoundFont,
							pEventMIDI->GetChNo(),
							pEventMIDI->GetNoteNo(),
							vel);
			if (tsfresult == 0) {
				//新しい発音の割り当てができなかった場合
				sprintf_s(logbuf, 256, "tsf_channel_note_on failed. Ch:%d Note:%d\n", pEventMIDI->GetChNo(), pEventMIDI->GetNoteNo());
				OutputDebugString(logbuf);
			}
			break;
		//ポリフォニックキープレッシャー
		case SMEventMIDI::PolyphonicKeyPressure:
			break;
		//コントロールチェンジ
		case SMEventMIDI::ControlChange:
			if ((pEventMIDI->GetCCNo() == 64) && (m_SynthParam.sustain == 0)) {
				//サスティン無功の場合はサスティンをスキップ
			}
			else {
				tsfresult = tsf_channel_midi_control(
								g_pTinySoundFont,
								pEventMIDI->GetChNo(),
								pEventMIDI->GetCCNo(),
								pEventMIDI->GetCCValue());
				if (tsfresult == 0) {
					//新しいチャンネルの割り当てができなかった場合
					sprintf_s(logbuf, 256, "tsf_channel_midi_control failed. Ch:%d CC:%d Val:%d\n", pEventMIDI->GetChNo(), pEventMIDI->GetCCNo(), pEventMIDI->GetCCValue());
					OutputDebugString(logbuf);
				}
			}
			break;
		//プログラムチェンジ
		case SMEventMIDI::ProgramChange:
			tsfresult = tsf_channel_set_presetnumber(
								g_pTinySoundFont,
								pEventMIDI->GetChNo(),
								pEventMIDI->GetProgramNo(),
								(pEventMIDI->GetChNo() == SM_WAVETABLE_SYNTH_PERCUSSION_CH));
			if (tsfresult == 0) {
				//新しいチャンネルの割り当てができなかった場合
				sprintf_s(logbuf, 256, "tsf_channel_midi_control failed. Ch:%d Pg:%d\n", pEventMIDI->GetChNo(), pEventMIDI->GetProgramNo());
				OutputDebugString(logbuf);
			}
			break;
		//チャンネルプレッシャー
		case SMEventMIDI::ChannelPressure:
			break;
		//ピッチベンド
		case SMEventMIDI::PitchBend:
			tsfresult = tsf_channel_set_pitchwheel(
							g_pTinySoundFont,
							pEventMIDI->GetChNo(),
							pEventMIDI->GetPitchBendValue() + 8192);
			if (tsfresult == 0) {
				//新しいチャンネルの割り当てができなかった場合
				sprintf_s(logbuf, 256, "tsf_channel_midi_control failed. Ch:%d Pg:%d\n", pEventMIDI->GetChNo(), pEventMIDI->GetProgramNo());
				OutputDebugString(logbuf);
			}
			break;
		default:
			break;
	};
	
EXIT:;
	return result;
}

//******************************************************************************
// オーディオ出力スレッド
//******************************************************************************
unsigned __stdcall AudioOutputThread(void* pArguments)
{
	int result = 0;

	SMWavetableSynthCtrl* pWavetableSynthCtrl = NULL;
	
	OutputDebugString("AudioOutputThread start\n");
	
	if (pArguments == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	pWavetableSynthCtrl = (SMWavetableSynthCtrl*)pArguments;
	
	//オーディオ出力処理
	result = pWavetableSynthCtrl->AudioOutputProc();
	if (result != 0) goto EXIT;
	
EXIT:;
	OutputDebugString("AudioOutputThread end\n");
	_endthreadex(0);
	return 0;
}

} // end of namespace

