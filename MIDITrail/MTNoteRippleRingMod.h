//******************************************************************************
//
// MIDITrail / MTNoteRippleRingMod
//
// ノート波紋リング描画Modクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteRippleMod.h"


//******************************************************************************
// リング用ノート波紋描画Modクラス
//******************************************************************************
class MTNoteRippleRingMod : public MTNoteRippleMod
{
public:

	//コンストラクタ／デストラクタ
	MTNoteRippleRingMod(void);
	virtual ~MTNoteRippleRingMod(void);

private:

	virtual int _CreateNoteDesign();

};


