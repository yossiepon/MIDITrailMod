//******************************************************************************
//
// MIDITrail / MTNoteBoxRingLive
//
// ライブモニタ用ノートボックスリング描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteDesignRing.h"
#include "MTNoteBoxRingLive.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteBoxRingLive::MTNoteBoxRingLive(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteBoxRingLive::~MTNoteBoxRingLive(void)
{
}

//******************************************************************************
// ノートデザイン生成
//******************************************************************************
int MTNoteBoxRingLive::_CreateNoteDesign()
{
	int result = 0;

	try {
		m_pNoteDesign = new MTNoteDesignRing();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	
	//ライブモニタモード設定
	((MTNoteDesignRing*)m_pNoteDesign)->SetLiveMode();
	
EXIT:;
	return result;
}


