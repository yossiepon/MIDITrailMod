//******************************************************************************
//
// Simple MIDI Library / SMOutDevCtrlEx
//
// 拡張MIDI出力デバイス制御クラス
//
// Copyright (C) 2026 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// memo:
// MIDI出力デバイス制御クラス(SMOutDevCtrl)とWavetableシンセサイザ制御クラス(SMWavetableSynthCtrl)を統合して、
// MIDI出力デバイス制御クラスと同等のI/Fに集約するクラス。
// SMOutDevCtrlにはSMWavetableSynthCtrlを組み込まず、MIDIデバイスの制御に集中できるようにする。

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMOutDevCtrl.h"
#include "SMWavetableSynthCtrl.h"

//#pragma warning(disable:4251)

namespace SMIDILib {

//******************************************************************************
// パラメータ定義
//******************************************************************************


//******************************************************************************
// 拡張MIDI出力デバイス制御クラス
//******************************************************************************
class SMIDILIB_API SMOutDevCtrlEx
{
public:
	
	//コンストラクタ／デストラクタ
	SMOutDevCtrlEx(void);
	virtual ~SMOutDevCtrlEx(void);
	
	//初期化
	int Initialize();
	
	//Wavetableシンセサイザパラメータ設定
	int SetWavetableSynthParam(
				const WCHAR* pWavetableFilePath,
				SM_WAVETABLE_SYNTH_PARAM synthParam
			);
	
	//デバイス数取得
	unsigned long GetDevNum();
	
	//デバイスプロダクト名称取得
	int GetDevProductName(unsigned long index, std::string& name);
	
	//ポート対応デバイス登録
	int SetPortDev(unsigned char portNo, const char* pProductName);
	
	//全デバイスのオープン／クローズ
	int OpenPortDevAll();
	int ClosePortDevAll();
	
	//ポート情報クリア
	int ClearPortInfo();
	
	//MIDI出力メッセージ送信
	int SendShortMsg(unsigned char portNo, unsigned long msg, unsigned long size);
	int SendLongMsg(unsigned char portNo, unsigned char* pMsg, unsigned long size);
	int NoteOffAll();
	int SoundOffAll();
	
private:
	
	//ポート種別
	enum SMPortType {
		PortNone,			//なし
		PortWavetableSynth,	//Wavetable シンセサイザ
		PortMIDIDevice		//MIDI デバイス
	};
	
private:
	
	//出力デバイス制御
	SMOutDevCtrl m_OutDevCtrl;
	
	//Wavetableシンセサイザ制御
	SMWavetableSynthCtrl m_WavetableSynthCtrl;
	
	//Wavetableシンセサイザ設定値
	WCHAR m_WavetableFilePath[MAX_PATH];
	SM_WAVETABLE_SYNTH_PARAM m_SynthParam;
	
	//ポート情報
	SMPortType m_PortType[SM_MIDIOUT_PORT_NUM_MAX];

};

} // end of namespace

//#pragma warning(default:4251)

