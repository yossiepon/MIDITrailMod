//******************************************************************************
//
// MIDITrail / MTNoteLyricsRing
//
// ノート歌詞リング描画クラス
//
// Copyright (C) 2025 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteLyrics.h"


//******************************************************************************
// ノート歌詞リング描画クラス
//******************************************************************************
class MTNoteLyricsRing : public MTNoteLyrics
{
public:

	//コンストラクタ／デストラクタ
	MTNoteLyricsRing(void);
	virtual ~MTNoteLyricsRing(void);

protected:

	virtual int _CreateNoteDesign();
};
