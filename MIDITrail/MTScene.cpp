//******************************************************************************
//
// MIDITrail / MTScene
//
// MIDITrail シーン基底クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTScene.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTScene::MTScene(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTScene::~MTScene(void)
{
	m_SceneParamDictionary.clear();
}

//******************************************************************************
// 名称取得
//******************************************************************************
const TCHAR* MTScene::GetName()
{
	return _T("NO NAME");
}

//******************************************************************************
// 生成
//******************************************************************************
int MTScene::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	return 0;
}

//******************************************************************************
// 変換
//******************************************************************************
int MTScene::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	return 0;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTScene::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	return 0;
}

//******************************************************************************
// 破棄
//******************************************************************************
void MTScene::Release()
{
	return;
}

//******************************************************************************
// ウィンドウクリックイベント受信
//******************************************************************************
int MTScene::OnWindowClicked(
		unsigned long button,
		unsigned long wParam,
		unsigned long lParam
	)
{
	return 0;
}

//******************************************************************************
// 演奏開始イベント受信
//******************************************************************************
int MTScene::OnPlayStart(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	return 0;
}

//******************************************************************************
// 演奏終了イベント受信
//******************************************************************************
int MTScene::OnPlayEnd(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	return 0;
}

//******************************************************************************
// シーケンサメッセージ受信
//******************************************************************************
int MTScene::OnRecvSequencerMsg(
		unsigned long wParam,
		unsigned long lParam
	)
{
	return 0;
}

//******************************************************************************
// 巻き戻し
//******************************************************************************
int MTScene::Rewind()
{
	return 0;
}

//******************************************************************************
// デフォルト視点取得
//******************************************************************************
void MTScene::GetDefaultViewParam(
		MTViewParamMap* pParamMap
	)
{
	return;
}

//******************************************************************************
// 視点取得
//******************************************************************************
void MTScene::GetViewParam(
		MTViewParamMap* pParamMap
	)
{
	return;
}

//******************************************************************************
// 視点登録
//******************************************************************************
void MTScene::SetViewParam(
		MTViewParamMap* pParamMap
	)
{
	return;
}

//******************************************************************************
// 視点リセット
//******************************************************************************
void MTScene::ResetViewpoint()
{
}

//******************************************************************************
// 表示効果設定
//******************************************************************************
void MTScene::SetEffect(
		MTScene::EffectType type,
		bool isEnable
	)
{
	return;
}

//******************************************************************************
// 演奏速度設定
//******************************************************************************
void MTScene::SetPlaySpeedRatio(
		unsigned long ratio
	)
{
	return;
}


//******************************************************************************
// シーンパラメータ登録
//******************************************************************************
int MTScene::SetParam(
		const char* pKey,
		const char* pValue
	)
{
	int result = 0;
	std::string key = pKey;
	std::string value = pValue;
	MTSceneParamDictionary::iterator itr;

	if ((pKey == NULL) || (pValue == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//更新の場合は既存のデータを削除
	itr = m_SceneParamDictionary.find(key);
	if (itr != m_SceneParamDictionary.end()) {
		m_SceneParamDictionary.erase(itr);
	}

	//データ登録
	m_SceneParamDictionary.insert(MTSceneParamDictionaryPair(key, value));

EXIT:;
	return result;
}

//******************************************************************************
// シーンパラメータ取得
//******************************************************************************
const char* MTScene::GetParam(
		const char* pKey
	)
{
	const char* pValue = NULL;
	std::string key = pKey;
	std::string value;
	MTSceneParamDictionary::iterator itr;

	itr = m_SceneParamDictionary.find(key);

	if (itr != m_SceneParamDictionary.end()) {
		pValue = (itr->second).c_str();
	}

	return pValue;
}


