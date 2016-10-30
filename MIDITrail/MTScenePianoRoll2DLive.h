//******************************************************************************
//
// MIDITrail / MTScenePianoRoll2DLive
//
// ライブモニタ用ピアノロール2Dシーン描画クラス
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once
#include "MTScenePianoRoll3DLive.h"


//******************************************************************************
// ピアノロール2Dシーン描画クラス
//******************************************************************************
class MTScenePianoRoll2DLive : public MTScenePianoRoll3DLive
{
public:
	
	//コンストラクタ／デストラクタ
	MTScenePianoRoll2DLive(void);
	virtual ~MTScenePianoRoll2DLive(void);
	
	//名称取得
	const TCHAR* GetName();
	
	//生成
	virtual int Create(
				HWND hWnd,
				LPDIRECT3DDEVICE9 pD3DDevice,
				SMSeqData* pSeqData
			);
	
};


