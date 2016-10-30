//******************************************************************************
//
// MIDITrail / MTScenePianoRoll2DLive
//
// ライブモニタ用ピアノロール2Dシーン描画クラス
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRoll2DLive.h"


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTScenePianoRoll2DLive::MTScenePianoRoll2DLive(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTScenePianoRoll2DLive::~MTScenePianoRoll2DLive(void)
{
}

//******************************************************************************
// 名称取得
//******************************************************************************
const TCHAR* MTScenePianoRoll2DLive::GetName()
{
	return _T("PianoRoll2D");
}

//******************************************************************************
// シーン生成
//******************************************************************************
int MTScenePianoRoll2DLive::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	
	//ピアノロール2Dはライトなし
	//  ノートボックスの幅をゼロにするので表と裏が同一平面状で重なる
	//  ライトを有効にすると表と裏の色が異なりZファイティングを誘発する
	m_IsEnableLight = false;
	
	result = MTScenePianoRoll3DLive::Create(hWnd, pD3DDevice, pSeqData);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

