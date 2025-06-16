//******************************************************************************
//
// MIDITrail / MTNoteRippleRingLive
//
// ライブモニタ用ノート波紋リング描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteDesignRing.h"
#include "MTNoteRippleRingLive.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteRippleRingLive::MTNoteRippleRingLive(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteRippleRingLive::~MTNoteRippleRingLive(void)
{
}

//******************************************************************************
// ノートデザイン生成
//******************************************************************************
int MTNoteRippleRingLive::_CreateNoteDesign()
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


