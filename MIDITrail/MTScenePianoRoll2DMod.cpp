//******************************************************************************
//
// MIDITrail / MTScenePianoRoll2DMod
//
// ピアノロール2Dシーン描画Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRoll2DMod.h"


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTScenePianoRoll2DMod::MTScenePianoRoll2DMod(void)
{
	m_IsSingleKeyboard = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTScenePianoRoll2DMod::~MTScenePianoRoll2DMod(void)
{
}

//******************************************************************************
// 名称取得
//******************************************************************************
const TCHAR* MTScenePianoRoll2DMod::GetName()
{
	return _T("PianoRoll2D");
}

//******************************************************************************
// シーン生成
//******************************************************************************
int MTScenePianoRoll2DMod::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	//ピアノロール2Dはライトなし
	//  ノートボックスの幅をゼロにするので表と裏が同一平面状で重なる
	//  ライトを有効にすると表と裏の色が異なりZファイティングを誘発する
	m_IsEnableLight = FALSE;

	result = MTScenePianoRoll3DMod::Create(hWnd, pD3DDevice, pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

