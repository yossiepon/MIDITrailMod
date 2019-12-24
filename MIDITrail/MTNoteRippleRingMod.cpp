//******************************************************************************
//
// MIDITrail / MTNoteRippleRingMod
//
// ノート波紋リング描画Modクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteDesignRingMod.h"
#include "MTNoteRippleRingMod.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteRippleRingMod::MTNoteRippleRingMod(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteRippleRingMod::~MTNoteRippleRingMod(void)
{
}

//******************************************************************************
// ノートデザイン生成
//******************************************************************************
int MTNoteRippleRingMod::_CreateNoteDesign()
{
	int result = 0;

	try {
		//ノートデザインModオブジェクト生成
		m_pNoteDesignMod = new MTNoteDesignRingMod();
		m_pNoteDesign = m_pNoteDesignMod;
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}


