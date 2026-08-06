//******************************************************************************
//
// MIDITrail / MTNoteLyricsRing
//
// ノート歌詞描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteLyricsRing.h"
#include "MTNoteDesignRingMod.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteLyricsRing::MTNoteLyricsRing(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteLyricsRing::~MTNoteLyricsRing(void)
{
	Release();
}

//******************************************************************************
// ノート歌詞デザイン生成
//******************************************************************************
int MTNoteLyricsRing::_CreateNoteDesign()
{
	int result = 0;

	try {
		m_pNoteDesign = new MTNoteDesignRingMod();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

