//******************************************************************************
//
// MIDITrail / MTNoteBoxRing
//
// ノートボックスリング描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteDesignRing.h"
#include "MTNoteBoxRing.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteBoxRing::MTNoteBoxRing(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteBoxRing::~MTNoteBoxRing(void)
{
}

//******************************************************************************
// ノートデザイン生成
//******************************************************************************
int MTNoteBoxRing::_CreateNoteDesign()
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


