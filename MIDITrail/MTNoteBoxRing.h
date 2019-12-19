//******************************************************************************
//
// MIDITrail / MTNoteBoxRing
//
// ノートボックスリング描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteBox.h"


//******************************************************************************
// ノートボックスリング描画クラス
//******************************************************************************
class MTNoteBoxRing : public MTNoteBox
{
public:

	//コンストラクタ／デストラクタ
	MTNoteBoxRing(void);
	virtual ~MTNoteBoxRing(void);

private:

	virtual int _CreateNoteDesign();

};


