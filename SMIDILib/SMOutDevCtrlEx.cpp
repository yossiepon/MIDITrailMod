//******************************************************************************
//
// Simple MIDI Library / SMOutDevCtrlEx
//
// 拡張MIDI出力デバイス制御クラス
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMOutDevCtrlEx.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMOutDevCtrlEx::SMOutDevCtrlEx()
{
	unsigned char portNo = 0;
	
	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortType[portNo] = PortNone;
	}
	
	m_WavetableFilePath[0] = L'\0';
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMOutDevCtrlEx::~SMOutDevCtrlEx()
{
}

//******************************************************************************
// Wavetableシンセサイザパラメータ登録
//******************************************************************************
int SMOutDevCtrlEx::SetWavetableSynthParam(
		const WCHAR* pWavetableFilePath,
		SM_WAVETABLE_SYNTH_PARAM synthParam
	)
{
	int result = 0;
	errno_t eresult = 0;
	
	if (pWavetableFilePath == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	eresult = wcscpy_s(m_WavetableFilePath, MAX_PATH, pWavetableFilePath);
	if (eresult != 0) {
		result = YN_SET_ERR("The file path is too long.", 0, 0);
		goto EXIT;
	}
	
	m_SynthParam = synthParam;
	
EXIT:;
	return result;
}

//******************************************************************************
// 初期化
//******************************************************************************
int SMOutDevCtrlEx::Initialize()
{
	int result = 0;
	
	result = ClearPortInfo();
	if (result != 0) goto EXIT;
	
	result = m_WavetableSynthCtrl.Initialize(
						m_WavetableFilePath,
						m_SynthParam
					);
	if (result != 0) goto EXIT;
	
	result = m_OutDevCtrl.Initialize();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// デバイス数取得
//******************************************************************************
unsigned long SMOutDevCtrlEx::GetDevNum()
{
	unsigned int devNum = 0;
	
	//Wavetableシンセサイザをカウントする
	devNum = 1;
	
	//MIDI出力デバイス数を加算する
	devNum += m_OutDevCtrl.GetDevNum();
	
	return devNum;
}

//******************************************************************************
// デバイスプロダクト名称取得
//******************************************************************************
int SMOutDevCtrlEx::GetDevProductName(
		unsigned long index,
		std::string& name
	)
{
	int result = 0;

	if (index == 0) {
		result = m_WavetableSynthCtrl.GetDevProductName(name);
		if (result != 0) goto EXIT;
	}
	else {
		result = m_OutDevCtrl.GetDevProductName(index - 1, name);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ポートに対応するデバイスを設定
//******************************************************************************
int SMOutDevCtrlEx::SetPortDev(
		unsigned char portNo,
		const char* pProductName
	)
{
	int result = 0;
	std::string synthProductName;

	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}
	
	result = m_WavetableSynthCtrl.GetDevProductName(synthProductName);
	if (result != 0) goto EXIT;
	
	if (strcmp(synthProductName.c_str(), pProductName) == 0) {
		m_PortType[portNo] = PortWavetableSynth;
	}
	else {
		m_PortType[portNo] = PortMIDIDevice;
		result = m_OutDevCtrl.SetPortDev(portNo, pProductName);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 全デバイスのオープン
//******************************************************************************
int SMOutDevCtrlEx::OpenPortDevAll()
{
	int result = 0;
	unsigned char portNo = 0;
	bool isActiveWavetableSynth = false;
	
	//Wavetableシンセサイザ
	//Wavetable読み込みで負荷が発生するため選択されているときだけオープン
	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		if (m_PortType[portNo] == PortWavetableSynth) {
			isActiveWavetableSynth = true;
		}
	}
	if (isActiveWavetableSynth) {
		result = m_WavetableSynthCtrl.Open();
		if (result != 0) goto EXIT;
	}
	
	//MIDIデバイス
	result = m_OutDevCtrl.OpenPortDevAll();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// 全デバイスのクローズ
//******************************************************************************
int SMOutDevCtrlEx::ClosePortDevAll()
{
	int result = 0;
	
	result = m_WavetableSynthCtrl.Close();
	if (result != 0) goto EXIT;
	
	result = m_OutDevCtrl.ClosePortDevAll();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// ポート情報クリア
//******************************************************************************
int SMOutDevCtrlEx::ClearPortInfo()
{
	int result = 0;
	unsigned char portNo = 0;
	
	result = m_OutDevCtrl.ClearPortInfo();
	if (result != 0) goto EXIT;
	
	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortType[portNo] = PortNone;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDIデータ送信（ショートメッセージ）
//******************************************************************************
int SMOutDevCtrlEx::SendShortMsg(
		unsigned char portNo,
		unsigned long msg,
		unsigned long size
	)
{
	int result = 0;
	
	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}
	
	if (m_PortType[portNo] == PortWavetableSynth) {
		result = m_WavetableSynthCtrl.SendShortMsg((unsigned char*)&msg, size);
		if (result != 0) goto EXIT;
	}
	else if (m_PortType[portNo] == PortMIDIDevice) {
		result = m_OutDevCtrl.SendShortMsg(portNo, msg, size);
		if (result != 0) goto EXIT;
	}
	else {
		//出力先が指定されていないポートに対するデータ送信のため無視する
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDIデータ送信（ロングメッセージ）
//******************************************************************************
int SMOutDevCtrlEx::SendLongMsg(
		unsigned char portNo,
		unsigned char* pMsg,
		unsigned long size
	)
{
	int result = 0;
	
	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}
	
	if (m_PortType[portNo] == PortWavetableSynth) {
		result = m_WavetableSynthCtrl.SendLongMsg(pMsg, size);
		if (result != 0) goto EXIT;
	}
	else if (m_PortType[portNo] == PortMIDIDevice) {
		result = m_OutDevCtrl.SendLongMsg(portNo, pMsg, size);
		if (result != 0) goto EXIT;
	}
	else {
		//出力先が指定されていないポートに対するデータ送信のため無視する
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// 全ポートノートオフ
//******************************************************************************
int SMOutDevCtrlEx::NoteOffAll()
{
	int result = 0;
	
	result = m_WavetableSynthCtrl.NoteOffAll();
	if (result != 0) goto EXIT;
	
	result = m_OutDevCtrl.NoteOffAll();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// 全ポートサウンドオフ
//******************************************************************************
int SMOutDevCtrlEx::SoundOffAll()
{
	int result = 0;
	
	result = m_WavetableSynthCtrl.SoundOffAll();
	if (result != 0) goto EXIT;
	
	result = m_OutDevCtrl.SoundOffAll();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

} // end of namespace

