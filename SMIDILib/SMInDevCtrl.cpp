//******************************************************************************
//
// Simple MIDI Library / SMInDevCtrl
//
// MIDI入力デバイス制御クラス
//
// Copyright (C) 2012-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMInDevCtrl.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMInDevCtrl::SMInDevCtrl(void)
{
	//ポート情報
	m_PortInfo.isExist = false;
	m_PortInfo.devId = 0;
	m_PortInfo.hMidiIn = NULL;
	memset((void*)&(m_PortInfo.midiHdr), 0, sizeof(MIDIHDR));
	
	//コールバック関数
	m_pInReadCallBack = NULL;
	m_pCallBackUserParam = NULL;
	
	//パケット解析系
	m_isContinueSysEx = false;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMInDevCtrl::~SMInDevCtrl()
{
	m_InDevList.clear();
	ClosePortDev();
}

//******************************************************************************
// 初期化
//******************************************************************************
int SMInDevCtrl::Initialize()
{
	int result = 0;
	
	//ポート情報クリア
	result = ClearPortInfo();
	if (result != 0) goto EXIT;
	
	//MIDI入力デバイス一覧を作成
	result = _InitDevList();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// デバイスリスト初期化
//******************************************************************************
int SMInDevCtrl::_InitDevList()
{
	int result = 0;
	MMRESULT apiresult = 0;
	unsigned long devId = 0;
	unsigned long devNum = 0;
	MIDIINCAPS mic;
	SMInDevInfo devInfo;

	m_InDevList.clear();

	//MIDI出力デバイスの数
	devNum = midiInGetNumDevs();

	//MIDI出力デバイスの情報を取得する
	for (devId = 0; devId < devNum; devId++) {

		ZeroMemory(&mic, sizeof(MIDIINCAPS));
		ZeroMemory(&devInfo, sizeof(SMInDevInfo));

		apiresult= midiInGetDevCaps(devId, &mic, sizeof(MIDIINCAPS));
		if (apiresult != MMSYSERR_NOERROR) {
			result = YN_SET_ERR("MIDI In device access error.", apiresult, 0);
			goto EXIT;
		}
		devInfo.devId = devId;
		memcpy(devInfo.productName, mic.szPname, MAXPNAMELEN);

		//取得した情報をリストに登録
		m_InDevList.push_back(devInfo);
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// デバイス数取得
//******************************************************************************
unsigned long SMInDevCtrl::GetDevNum()
{
	return (unsigned long)m_InDevList.size();
}

//******************************************************************************
// デバイスプロダクト名称取得
//******************************************************************************
int SMInDevCtrl::GetDevProductName(
		unsigned long index,
		std::string& name
	)
{
	int result = 0;
	SMInDevListItr itr;
	
	if (index >= m_InDevList.size()) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	itr = m_InDevList.begin();
	advance(itr, index);
	
	name = itr->productName;
	
EXIT:;
	return result;
}

//******************************************************************************
// ポートに対応するデバイスを設定
//******************************************************************************
int SMInDevCtrl::SetPortDev(
		const char* pProductName
	)
{
	int result = 0;
	bool isFound = false;
	SMInDevListItr itr;
	
	if (pProductName == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	for (itr = m_InDevList.begin(); itr != m_InDevList.end(); itr++) {
		if (strcmp(itr->productName, pProductName) == 0) {
			m_PortInfo.isExist = true;
			m_PortInfo.devId = itr->devId;
			//m_PortInfo.hMidiIn = NULL;
			isFound = true;
			break;
		}
	}
	if (!isFound) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDIイベント読み込みコールバック関数登録
//******************************************************************************
void SMInDevCtrl::SetInReadCallBack(
		SMInReadCallBack pCallBack,
		void* pUserParam
	)
{
	m_pInReadCallBack = pCallBack;
	m_pCallBackUserParam = pUserParam;
}

//******************************************************************************
// ポートに対応するデバイスを開く
//******************************************************************************
int SMInDevCtrl::OpenPortDev()
{
	int result = 0;
	MMRESULT apiresult = 0;
	HMIDIIN hMidiIn = NULL;
	unsigned char* pBuf = NULL;
	
	result = ClosePortDev();
	if (result != 0) goto EXIT;
	
	//ポートが存在しなければスキップ
	if (!m_PortInfo.isExist) goto EXIT;;
	
	m_isContinueSysEx = false;
	
	//デバイスを開く
	apiresult = midiInOpen(
					&hMidiIn,			//ハンドルのアドレス
					m_PortInfo.devId,	//デバイス識別子
					(DWORD_PTR)_InReadCallBack,	//コールバック関数
					(DWORD_PTR)this,	//コールバック関数に渡すユーザーインスタンスデータ
					CALLBACK_FUNCTION	//コールバックフラグ：コールバック関数
				);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device open error.", apiresult, 0);
		goto EXIT;
	}
	m_PortInfo.hMidiIn = hMidiIn;
	
	//MIDI入力バッファ作成
	try {
		pBuf = new unsigned char[SM_MIDIIN_BUF_SIZE];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	
	//ヘッダ作成
	memset((void*)&(m_PortInfo.midiHdr), 0, sizeof(MIDIHDR));
	m_PortInfo.midiHdr.lpData         = (LPSTR)pBuf;
	m_PortInfo.midiHdr.dwBufferLength = SM_MIDIIN_BUF_SIZE;
	m_PortInfo.midiHdr.dwFlags        = 0;
	pBuf = NULL;
	
	//MIDI入力バッファ準備
	apiresult = midiInPrepareHeader(hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}
	
	//MIDI入力バッファ登録
	apiresult = midiInAddBuffer(hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}
	
	//MIDI入力開始
	apiresult = midiInStart(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device open error.", apiresult, 0);
		goto EXIT;
	}
	
EXIT:;
	delete [] pBuf;
	return result;
}

//******************************************************************************
// ポートに対応するデバイスを閉じる
//******************************************************************************
int SMInDevCtrl::ClosePortDev()
{
	int result = 0;
	UINT apiresult = 0;
	
	//ポートが存在しなければスキップ
	if (!m_PortInfo.isExist) goto EXIT;
	
	//ポートを開いてなければスキップ
	if (m_PortInfo.hMidiIn == NULL) goto EXIT;
	
	//MIDI入力停止
	//  キューにバッファが存在する場合は現在のバッファは処理済みにされる
	//  MIDIHDRのdwBytesRecordedメンバにはデータの実際の長さが入る
	//  ただしキューにある空のバッファは残され処理済みとはされない
	apiresult = midiInStop(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", apiresult, 0);
		goto EXIT;
	}
	
	//MIDI入力停止
	//  未処理の入力バッファをコールバック関数に返す
	//  MIDIHDRのdwFlagsメンバにMHDR_DONEフラグをセットする
	apiresult = midiInReset(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", apiresult, 0);
		goto EXIT;
	}
	
	//MIDI入力バッファ解除
	apiresult = midiInUnprepareHeader(m_PortInfo.hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", apiresult, 0);
		goto EXIT;
	}
	
	//バッファ破棄
	delete [] (unsigned char*)(m_PortInfo.midiHdr.lpData);
	m_PortInfo.midiHdr.lpData = NULL;
	
	//デバイスを閉じる
	apiresult = midiInClose(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", 0, 0);
		goto EXIT;
	}
	m_PortInfo.hMidiIn = NULL;
	
EXIT:;
	return result;
}


//******************************************************************************
// ポート情報クリア
//******************************************************************************
int SMInDevCtrl::ClearPortInfo()
{
	int result = 0;
	
	result = ClosePortDev();
	if (result != 0) goto EXIT;
	
	m_PortInfo.isExist = false;
	m_PortInfo.devId = 0;
	m_PortInfo.hMidiIn = NULL;
	memset((void*)&(m_PortInfo.midiHdr), 0, sizeof(MIDIHDR));
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN 読み込みコールバック関数
//******************************************************************************
void SMInDevCtrl::_InReadCallBack(
		HMIDIIN hMidiIn,
		UINT wMsg,
		DWORD_PTR dwInstance,
		DWORD_PTR dwParam1,
		DWORD_PTR dwParam2
	)
{
	SMInDevCtrl* pInDevCtrl = NULL;
	
	pInDevCtrl = (SMInDevCtrl*)dwInstance;
	pInDevCtrl->_InReadProc(hMidiIn, wMsg, dwParam1, dwParam2);
	
	return;
}

//******************************************************************************
// MIDI IN 読み込み処理
//******************************************************************************
void SMInDevCtrl::_InReadProc(
		HMIDIIN hMidiIn,
		UINT wMsg,
		DWORD_PTR dwParam1,
		DWORD_PTR dwParam2
	)
{
	int result = 0;
	SMEvent event;
	
	switch (wMsg) {
		case MIM_OPEN:
			//MIDI入力デバイスオープン
			break;
		case MIM_CLOSE:
			//MIDI入力デバイスクローズ
			break;
		case MIM_DATA:
			//MIDIメッセージ受信
			//  dwParam1 MIDIメッセージ
			//  dwParam2 タイムスタンプ
			m_isContinueSysEx = false;
			result = _InReadProcMIDI(dwParam1, dwParam2, &event);
			if (result != 0) goto EXIT;
			break;
		case MIM_LONGDATA:
			//システムエクスクルーシブ受信
			//  dwParam1 MIDIHDR構造体へのポインタ
			//  dwParam2 タイムスタンプ
			result = _InReadProcSysEx((MIDIHDR*)dwParam1, dwParam2, &m_isContinueSysEx, &event);
			if (result != 0) goto EXIT;
			break;
		case MIM_ERROR:
			//無効なMIDIメッセージ受信
			break;
		case MIM_LONGERROR:
			//無効なエクスクルーシブメッセージ受信
			break;
		case MIM_MOREDATA:
			//未処理のMIDIメッセージ
			//midiInOpenでMIDI_IO_STATUSを指定した場合のみ発生する
			break;
		default:
			break;
	}
	
	//コールバック呼び出し
	if ((m_pInReadCallBack != NULL) &&
		(event.GetType() != SMEvent::EventNone)) {
		result = m_pInReadCallBack(&event, m_pCallBackUserParam);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	if (result != 0) {
		YN_SHOW_ERR(NULL);
	}
	return;
}

//******************************************************************************
// MIDIメッセージ読み込み処理
//******************************************************************************
int SMInDevCtrl::_InReadProcMIDI(
		DWORD_PTR midiMessage,
		DWORD_PTR timestamp,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned char status = 0;
	unsigned char data[2] = { 0, 0 };
	unsigned long dataLength = 0;
	
	status  = (unsigned char)((midiMessage      ) & 0x000000FF);
	data[0] = (unsigned char)((midiMessage >>  8) & 0x000000FF);
	data[1] = (unsigned char)((midiMessage >> 16) & 0x000000FF);
	
	if ((status & 0xF0) != 0xF0) {
		//MIDIメッセージ
		dataLength = _GetMIDIMsgSize(status) - 1;
		result = pEvent->SetMIDIData(status, data, dataLength);
		if (result != 0) goto EXIT;
	}
	else if (status == 0xF0) {
		//システムエクスクルーシブメッセージ
		//ありえないAPIの挙動
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	else {
		//システムコモンメッセージまたはシステムリアルタイムメッセージ
		dataLength = _GetSysMsgSize(status) - 1;
		result = pEvent->SetSysMsgData(status, data, dataLength);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// システムエクスクルーシブ読み込み処理
//******************************************************************************
int SMInDevCtrl::_InReadProcSysEx(
		MIDIHDR* pMIDIHDR,
		DWORD_PTR timestamp,
		bool* pIsContinueSysEx,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned char* pData = NULL;
	UINT apiresult = 0;
	
	//受信データサイズがゼロなら何もしない
	if (pMIDIHDR->dwBytesRecorded == 0) goto EXIT;
	
	//システムエクスクルーシブ初回読み込み
	if (!(*pIsContinueSysEx)) {
		pData = (unsigned char*)(pMIDIHDR->lpData);
		result = pEvent->SetSysExData(0xF0, pData + 1, pMIDIHDR->dwBytesRecorded - 1);
		if (result != 0) goto EXIT;
	}
	//2番目以降のパケット
	else {
		pData = (unsigned char*)(pMIDIHDR->lpData);
		result = pEvent->SetSysExData(0xF7, pData, pMIDIHDR->dwBytesRecorded);
		if (result != 0) goto EXIT;
	}
	
	//システムエクスクルーシブの終端を確認
	if (pData[(pMIDIHDR->dwBytesRecorded)-1] == 0xF7) {
		//システムエクスクルーシブが閉じる
		*pIsContinueSysEx = false;
	}
	else {
		//末尾が0xF7でなければ次にデータがまたがる
		*pIsContinueSysEx = true;
	}
	
	//MIDI入力バッファ準備
	apiresult = midiInPrepareHeader(m_PortInfo.hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}
	
	//MIDI入力バッファ登録
	apiresult = midiInAddBuffer(m_PortInfo.hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDIメッセージサイズ取得
//******************************************************************************
unsigned long SMInDevCtrl::_GetMIDIMsgSize(unsigned char status)
{
	unsigned long size = 0;

	switch (status & 0xF0) {
		case 0x80: size = 3; break;  //ノートオフ
		case 0x90: size = 3; break;  //ノートオン
		case 0xA0: size = 3; break;  //ポリフォニックキープレッシャー
		case 0xB0: size = 3; break;  //コントロールチェンジ
		case 0xC0: size = 2; break;  //プログラムチェンジ
		case 0xD0: size = 2; break;  //チャンネルプレッシャー
		case 0xE0: size = 3; break;  //ピッチベンド
		case 0xF0:
			size = _GetSysMsgSize(status);
			break;
	}
	
	return size;
}

//******************************************************************************
// システムメッセージサイズ取得
//******************************************************************************
unsigned long SMInDevCtrl::_GetSysMsgSize(unsigned char status)
{
	unsigned long size = 0;
	
	switch (status) {
		case 0xF0: size = 0; break;  // F0 ... F7 システムエクスクルーシブ
		case 0xF1: size = 2; break;  // F1 dd     システムコモンメッセージ：クオーターフレーム(MTC)
		case 0xF2: size = 3; break;  // F2 dl dm  システムコモンメッセージ：ソングポジションポインタ
		case 0xF3: size = 2; break;  // F3 dd     システムコモンメッセージ：ソングセレクト
		case 0xF4: size = 1; break;  // F4 未定義
		case 0xF5: size = 1; break;  // F5 未定義
		case 0xF6: size = 1; break;  // F6 システムコモンメッセージ：チューンリクエスト
		case 0xF7: size = 1; break;  // F7 エンドオブシステムエクスクルーシブ
		case 0xF8: size = 1; break;  // F8 システムリアルタイムメッセージ：タイミングクロック
		case 0xF9: size = 1; break;  // F9 未定義
		case 0xFA: size = 1; break;  // FA システムリアルタイムメッセージ：スタート
		case 0xFB: size = 1; break;  // FB システムリアルタイムメッセージ：コンティニュー
		case 0xFC: size = 1; break;  // FC システムリアルタイムメッセージ：ストップ
		case 0xFD: size = 1; break;  // FD 未定義
		case 0xFE: size = 1; break;  // FE システムリアルタイムメッセージ：アクティブセンシング
		case 0xFF: size = 1; break;  // FF システムリアルタイムメッセージ：システムリセット
	}
	
	return size;
}

} // end of namespace


