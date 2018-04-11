//******************************************************************************
//
// MIDITrail / MTScenePianoRoll2DMod
//
// ピアノロール2Dシーン描画Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once
#include "MTScenePianoRoll3DMod.h"


//******************************************************************************
// ピアノロール2Dシーン描画Modクラス
//******************************************************************************
class MTScenePianoRoll2DMod : public MTScenePianoRoll3DMod
{
public:

	//コンストラクタ／デストラクタ
	MTScenePianoRoll2DMod(void);
	virtual ~MTScenePianoRoll2DMod(void);

	//名称取得
	const TCHAR* GetName();

	//生成
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);

};

