//******************************************************************************
//
// MIDITrail / MTPictBoardRingMod
//
// ピクチャボードリング描画Modクラス
//
// Copyright (C) 2025 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "MTPictBoardRing.h"
#include "MTNoteDesignRingMod.h"


//******************************************************************************
//  ピクチャボードリング描画クラス
//******************************************************************************
class MTPictBoardRingMod : public MTPictBoardRing
{
public:

	//コンストラクタ／デストラクタ
	MTPictBoardRingMod(void);
	virtual ~MTPictBoardRingMod(void);

	//生成
	virtual int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, SMSeqData* pSeqData, bool isReverseMode);

protected:

	MTNoteDesignRingMod m_NoteDesignMod;

	virtual int _CreateVertexOfBoard(
			MTPICTBOARD_VERTEX* pVertex,
			unsigned long* pIbIndex,
			bool isReverseMode
		);

	float MTPictBoardRingMod::GetRippleMargin();
};

