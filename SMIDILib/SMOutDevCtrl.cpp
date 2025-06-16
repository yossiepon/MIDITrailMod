//******************************************************************************
//
// Simple MIDI Library / SMOutDevCtrl
//
// MIDI出力デバイス制御クラス
//
// Copyright (C) 2010-2021 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMOutDevCtrl.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMOutDevCtrl::SMOutDevCtrl(void)
{
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortInfo[portNo].isExist = false;
		m_PortInfo[portNo].devId = 0xFFFFFFFF;
		m_PortInfo[portNo].hMIDIOut = NULL;
	}
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMOutDevCtrl::~SMOutDevCtrl(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int SMOutDevCtrl::Initialize()
{
	int result = 0;

	//ポート情報クリア
	result = ClearPortInfo();
	if (result != 0) goto EXIT;

	//MIDI出力デバイス一覧を作成
	result = _InitDevList();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// デバイスリスト初期化
//******************************************************************************
int SMOutDevCtrl::_InitDevList()
{
	int result = 0;
	MMRESULT apiresult = 0;
	unsigned long devId = 0;
	unsigned long devNum = 0;
	MIDIOUTCAPS moc;
	SMOutDevInfo devInfo;

	m_OutDevList.clear();

	//MIDI出力デバイスの数
	devNum = midiOutGetNumDevs();

	//MIDI出力デバイスの情報を取得する
	for (devId = 0; devId < devNum; devId++) {

		ZeroMemory(&moc, sizeof(MIDIOUTCAPS));
		ZeroMemory(&devInfo, sizeof(SMOutDevInfo));

		apiresult= midiOutGetDevCaps(devId, &moc, sizeof(MIDIOUTCAPS));
		if (apiresult != MMSYSERR_NOERROR) {
			result = YN_SET_ERR("MIDI OUT device access error.", apiresult, 0);
			goto EXIT;
		}
		devInfo.devId = devId;
		memcpy(devInfo.productName, moc.szPname, MAXPNAMELEN);

		//取得した情報をリストに登録
		m_OutDevList.push_back(devInfo);
	}

EXIT:;
	return result;
}

//******************************************************************************
// デバイス数取得
//******************************************************************************
unsigned long SMOutDevCtrl::GetDevNum()
{
	return (unsigned long)m_OutDevList.size();
}

//******************************************************************************
// デバイスプロダクト名称取得
//******************************************************************************
int SMOutDevCtrl::GetDevProductName(
		unsigned long index,
		std::string& name
	)
{
	int result = 0;
	SMOutDevListItr itr;

	if (index >= m_OutDevList.size()) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	itr = m_OutDevList.begin();
	advance(itr, index);

	name = itr->productName;

EXIT:;
	return result;
}

//******************************************************************************
// ポートに対応するデバイスを設定
//******************************************************************************
int SMOutDevCtrl::SetPortDev(
		unsigned char portNo,
		const char* pProductName
	)
{
	int result = 0;
	bool isFound = false;
	SMOutDevListItr itr;

	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (pProductName == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	for (itr = m_OutDevList.begin(); itr != m_OutDevList.end(); itr++) {
		if (strcmp(itr->productName, pProductName) == 0) {
			m_PortInfo[portNo].isExist = true;
			m_PortInfo[portNo].devId = itr->devId;
			//m_PortInfo[portNo].hMIDIOut = NULL;
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
// ポートに対応するデバイスIDを取得
//******************************************************************************
int SMOutDevCtrl::GetPortDevId(
		unsigned char portNo,
		unsigned long* pDevId
	)
{
	int result = 0;

	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (pDevId == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (!m_PortInfo[portNo].isExist) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	*pDevId = m_PortInfo[portNo].devId;

EXIT:;
	return result;
}

//******************************************************************************
// 全ポートに対応するデバイスを開く
//******************************************************************************
int SMOutDevCtrl::OpenPortDevAll()
{
	int result = 0;
	UINT apiresult = 0;
	unsigned char portNo = 0;
	unsigned char prevPortNo = 0;
	unsigned long devId;
	HMIDIOUT hMIDIOut = NULL;
	bool isOpen = false;

	result = ClosePortDevAll();
	if (result != 0) goto EXIT;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		//ポートが存在しなければスキップ
		if (!m_PortInfo[portNo].isExist) continue;

		//ポートに対応するデバイスIDを取得
		devId = m_PortInfo[portNo].devId;

		//別のポートで同じデバイスをすでに開いている場合の対処
		isOpen = false;
		for (prevPortNo = 0; prevPortNo < portNo; prevPortNo++) {
			if (devId == m_PortInfo[prevPortNo].devId) {
				m_PortInfo[portNo].hMIDIOut = m_PortInfo[prevPortNo].hMIDIOut;
				isOpen = true;
				break;
			}
		}

		//新規にデバイスを開く
		if (!isOpen) {
			apiresult = midiOutOpen(
							&hMIDIOut,      //ハンドル
							devId,          //MIDI出力デバイス識別子
							NULL,           //再生進捗状況コールバック関数
							NULL,           //コールバック関数に渡すユーザーインスタンスデータ
							CALLBACK_NULL   //コールバックフラグ：コールバックなし
						);
			if (apiresult != MMSYSERR_NOERROR) {
				result = YN_SET_ERR("MIDI OUT device open error.", apiresult, 0);
				goto EXIT;
			}
			m_PortInfo[portNo].hMIDIOut = hMIDIOut;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 全ポートに対応するデバイスを閉じる
//******************************************************************************
int SMOutDevCtrl::ClosePortDevAll()
{
	int result = 0;
	UINT apiresult = 0;
	unsigned char portNo = 0;
	unsigned char nextPortNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		//ポートが存在しなければスキップ
		if (!m_PortInfo[portNo].isExist) continue;

		//デバイスを開いてなければスキップ
		if (m_PortInfo[portNo].hMIDIOut == NULL) continue;

		//デバイスを閉じる
		apiresult = midiOutClose(m_PortInfo[portNo].hMIDIOut);
		if (apiresult != MMSYSERR_NOERROR) {
			result = YN_SET_ERR("MIDI OUT device close error.", 0, 0);
			goto EXIT;
		}
		m_PortInfo[portNo].hMIDIOut = NULL;

		//別のポートで同じデバイスを開いている場合の対処
		for (nextPortNo = portNo+1; nextPortNo < SM_MIDIOUT_PORT_NUM_MAX; nextPortNo++) {
			if (m_PortInfo[portNo].devId == m_PortInfo[nextPortNo].devId) {
				m_PortInfo[nextPortNo].hMIDIOut = NULL;
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ポート情報クリア
//******************************************************************************
int SMOutDevCtrl::ClearPortInfo()
{
	int result = 0;
	unsigned char portNo = 0;

	result = ClosePortDevAll();
	if (result != 0) goto EXIT;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortInfo[portNo].isExist = false;
		m_PortInfo[portNo].devId = 0xFFFFFFFF;
		m_PortInfo[portNo].hMIDIOut = NULL;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDIデータ送信（ショートメッセージ）
//******************************************************************************
int SMOutDevCtrl::SendShortMsg(
		unsigned char portNo,
		unsigned long msg
	)
{
	int result = 0;
	UINT apiresult = 0;
	HMIDIOUT hMIDIOut = NULL;

	//サポート範囲外のポートが指定された場合は何もしない
	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) goto EXIT;

	//ポートが存在しなければ何もしない
	if (!m_PortInfo[portNo].isExist) goto EXIT;

	//デバイスが開かれていなければエラー
	if (m_PortInfo[portNo].hMIDIOut == NULL) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}
	hMIDIOut = m_PortInfo[portNo].hMIDIOut;

	//メッセージ出力：MIDI仕様により約0.3msec掛かる
	apiresult = midiOutShortMsg(hMIDIOut, msg);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, msg);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDIデータ送信（ロングメッセージ）
//******************************************************************************
int SMOutDevCtrl::SendLongMsg(
		unsigned char portNo,
		unsigned char* pMsg,
		unsigned long size
	)
{
	int result = 0;
	UINT apiresult = 0;
	HMIDIOUT hMIDIOut = NULL;
	MIDIHDR mh;

	//パラメータチェック
	if (pMsg == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//サポート範囲外のポートが指定された場合は何もしない
	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) goto EXIT;

	//ポートが存在しなければ何もしない
	if (!m_PortInfo[portNo].isExist) goto EXIT;

	//デバイスが開かれていなければエラー
	if (m_PortInfo[portNo].hMIDIOut == NULL) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}
	hMIDIOut = m_PortInfo[portNo].hMIDIOut;

	//ヘッダ作成
	memset((void*)&mh, 0, sizeof(MIDIHDR));
	mh.lpData         = (LPSTR)pMsg;
	mh.dwBufferLength = size;
	mh.dwFlags        = 0;

	//出力バッファ準備
	apiresult = midiOutPrepareHeader(hMIDIOut, &mh, sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, size);
		goto EXIT;
	}
	//メッセージ出力：MIDI仕様により約0.3msec以上掛かる
	apiresult = midiOutLongMsg(hMIDIOut, &mh, sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, size);
		goto EXIT;
	}

	//出力完了まで待ち合わせ
	while ((mh.dwFlags & MHDR_DONE) == 0) {
		//コールバックI/Fがないのでこうするしか・・・
	}

	//出力バッファ解放
	apiresult = midiOutUnprepareHeader(hMIDIOut, &mh, sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, size);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 全ポートノートオフ
//******************************************************************************
int SMOutDevCtrl::NoteOffAll()
{
	int result = 0;
	int i = 0;
	UINT apiresult = 0;
	unsigned long msg = 0;
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		//ポートとデバイスが存在しなければスキップ
		if (!m_PortInfo[portNo].isExist) continue;
		if (m_PortInfo[portNo].hMIDIOut == NULL) continue;

		//全トラックノートオフ
		for (i = 0; i < 16; i++) {
			msg = (0x7B << 8) | (0xB0 | i);
			apiresult = midiOutShortMsg(m_PortInfo[portNo].hMIDIOut, msg);
			if (apiresult != MMSYSERR_NOERROR) {
				result = YN_SET_ERR("MIDI OUT device output error.", apiresult, portNo);
				goto EXIT;
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 全ポートサウンドオフ
//******************************************************************************
int SMOutDevCtrl::SoundOffAll()
{
	int result = 0;
	int i = 0;
	UINT apiresult = 0;
	unsigned long msg = 0;
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		//ポートとデバイスが存在しなければスキップ
		if (!m_PortInfo[portNo].isExist) continue;
		if (m_PortInfo[portNo].hMIDIOut == NULL) continue;

		//全トラックサウンドオフ
		for (i = 0; i < 16; i++) {
			msg = (0x78 << 8) | (0xB0 | i);
			apiresult = midiOutShortMsg(m_PortInfo[portNo].hMIDIOut, msg);
			if (apiresult != MMSYSERR_NOERROR) {
				result = YN_SET_ERR("MIDI OUT device output error.", apiresult, portNo);
				goto EXIT;
			}
		}
	}

EXIT:;
	return result;
}

} // end of namespace

