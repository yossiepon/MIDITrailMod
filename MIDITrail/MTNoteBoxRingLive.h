//******************************************************************************
//
// MIDITrail / MTNoteBoxRingLive
//
// ライブモニタ用ノートボックスリング描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteBoxLive.h"


//******************************************************************************
// ライブモニタ用ノートボックスリング描画クラス
//******************************************************************************
class MTNoteBoxRingLive : public MTNoteBoxLive
{
public:

	//コンストラクタ／デストラクタ
	MTNoteBoxRingLive(void);
	virtual ~MTNoteBoxRingLive(void);

private:

	virtual int _CreateNoteDesign();

};


