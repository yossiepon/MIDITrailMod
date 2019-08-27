//******************************************************************************
//
// MIDITrail / MTScenePianoRollRain2D
//
// ピアノロールレイン2Dシーン描画クラス
//
// Copyright (C) 2013-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRollRain2D.h"


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTScenePianoRollRain2D::MTScenePianoRollRain2D(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTScenePianoRollRain2D::~MTScenePianoRollRain2D(void)
{
}

//******************************************************************************
// 名称取得
//******************************************************************************
const TCHAR* MTScenePianoRollRain2D::GetName()
{
	return _T("PianoRollRain2D");
}

//******************************************************************************
// シーン生成
//******************************************************************************
int MTScenePianoRollRain2D::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	//ピアノロールレイン2Dはシングルキーボード
	m_IsSingleKeyboard = true;

	result = MTScenePianoRollRain::Create(hWnd, pD3DDevice, pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// デフォルト視点取得
//******************************************************************************
void MTScenePianoRollRain2D::GetDefaultViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector;
	float phi = 0.0f;
	float theta = 0.0f;

	viewPointVector.x = 0.0f;
	viewPointVector.y = 0.34f * 16.0f / 2.0f;
	viewPointVector.z = - 10.0f;
	phi   = 90.0f;
	theta = 90.0f;

	pParamMap->clear();
	pParamMap->insert(MTViewParamMapPair("X", viewPointVector.x));
	pParamMap->insert(MTViewParamMapPair("Y", viewPointVector.y));
	pParamMap->insert(MTViewParamMapPair("Z", viewPointVector.z));
	pParamMap->insert(MTViewParamMapPair("Phi", phi));
	pParamMap->insert(MTViewParamMapPair("Theta", theta));
	pParamMap->insert(MTViewParamMapPair("ManualRollAngle", 0.0f));
	pParamMap->insert(MTViewParamMapPair("AutoRollVelocity", 0.0f));

	return;
}

