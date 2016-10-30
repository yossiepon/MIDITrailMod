//******************************************************************************
//
// MIDITrail / MTScenePianoRoll2D
//
// ピアノロール2Dシーン描画クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once
#include "mtscenepianoroll3d.h"


//******************************************************************************
// ピアノロール2Dシーン描画クラス
//******************************************************************************
class MTScenePianoRoll2D : public MTScenePianoRoll3D
{
public:

	//コンストラクタ／デストラクタ
	MTScenePianoRoll2D(void);
	virtual ~MTScenePianoRoll2D(void);

	//名称取得
	const TCHAR* GetName();

	//生成
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);

};

