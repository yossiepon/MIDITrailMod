//******************************************************************************
//
// MIDITrail / MTNoteRippleRing
//
// リング用ノート波紋描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteDesignRing.h"
#include "MTNoteRippleRing.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteRippleRing::MTNoteRippleRing(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteRippleRing::~MTNoteRippleRing(void)
{
}

//******************************************************************************
// ノートデザイン生成
//******************************************************************************
int MTNoteRippleRing::_CreateNoteDesign()
{
	int result = 0;

	try {
		m_pNoteDesign = new MTNoteDesignRing();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}


