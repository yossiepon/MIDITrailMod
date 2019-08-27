//******************************************************************************
//
// MIDITrail / MTScene
//
// MIDITrail シーン基底クラス
//
// Copyright (C) 2010-2018 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXScene.h"
#include "SMIDILib.h"
#include <string>
#include <map>

using namespace SMIDILib;

#pragma warning(disable:4251)


//******************************************************************************
// MIDITrail シーン基底クラス
//******************************************************************************
class MTScene : public DXScene
{
public:

	enum EffectType {
		EffectPianoKeyboard,
		EffectRipple,
		EffectPitchBend,
		EffectStars,
		EffectCounter,
		EffectBackgroundImage,
		EffectFileName
	};

	typedef std::map<std::string, float>  MTViewParamMap;
	typedef std::pair<std::string, float> MTViewParamMapPair;

public:

	//コンストラクタ／デストラクタ
	MTScene(void);
	virtual ~MTScene(void);

	//名称取得
	virtual const TCHAR* GetName();

	//生成
	virtual int Create(
					HWND hWnd,
					LPDIRECT3DDEVICE9 pD3DDevice,
					SMSeqData* pSeqData
				);

	//変換
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice);

	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//破棄
	virtual void Release();

	//ウィンドウクリックイベント受信
	virtual int OnWindowClicked(
					UINT button,
					WPARAM wParam,
					LPARAM lParam
				);

	//演奏開始イベント受信
	virtual int OnPlayStart(LPDIRECT3DDEVICE9 pD3DDevice);

	//演奏終了イベント受信
	virtual int OnPlayEnd(LPDIRECT3DDEVICE9 pD3DDevice);

	//シーケンサメッセージ受信
	virtual int OnRecvSequencerMsg(
					unsigned long param1,
					unsigned long param2
				);

	//巻き戻し
	virtual int Rewind();

	//視点取得／登録
	virtual void GetDefaultViewParam(MTViewParamMap* pParamMap);
	virtual void GetViewParam(MTViewParamMap* pParamMap);
	virtual void SetViewParam(MTViewParamMap* pParamMap);
	virtual void MoveToStaticViewpoint(unsigned long viewpointNo);

	//視点リセット
	virtual void ResetViewpoint();

	//表示効果設定
	virtual void SetEffect(EffectType type, bool isEnable);

	//演奏速度設定
	virtual void SetPlaySpeedRatio(unsigned long ratio);

	//パラメータ登録／取得
	int SetParam(const char* pKey, const char* pValue);
	const char* GetParam(const char* pKey);

protected:

	typedef std::map<std::string, std::string> MTSceneParamDictionary;
	typedef std::pair<std::string, std::string> MTSceneParamDictionaryPair;

	//シーンパラメータ
	MTSceneParamDictionary m_SceneParamDictionary;

};

#pragma warning(default:4251)


