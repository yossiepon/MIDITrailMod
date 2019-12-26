//******************************************************************************
//
// MIDITrail / MTNoteBoxRingMod
//
// ノートボックスリング描画Modクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteBoxMod.h"


//******************************************************************************
// ノートボックスリング描画クラス
//******************************************************************************
class MTNoteBoxRingMod : public MTNoteBoxMod
{
public:

	//コンストラクタ／デストラクタ
	MTNoteBoxRingMod(void);
	virtual ~MTNoteBoxRingMod(void);

private:

	virtual int _CreateNoteDesign();

};


