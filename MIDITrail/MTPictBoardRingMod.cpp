//******************************************************************************
//
// MIDITrail / MTPictBoardRingMod
//
// ピクチャボードリング描画クラス
//
// Copyright (C) 2025 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTPictBoardRingMod.h"
#include "DXH.h"


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTPictBoardRingMod::MTPictBoardRingMod(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTPictBoardRingMod::~MTPictBoardRingMod(void)
{
	Release();
}

//******************************************************************************
// ピクチャボード生成
//******************************************************************************
int MTPictBoardRingMod::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		bool isReverseMode
	)
{
	int result = 0;

	// 基底クラスの生成処理を呼び出す
	result = MTPictBoardRing::Create(pD3DDevice, pSceneName, pSeqData, isReverseMode);
	if (result != 0) goto EXIT;

	//ノートデザインModオブジェクト初期化
	result = m_NoteDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


//******************************************************************************
// ピクチャボード頂点生成
//******************************************************************************
int MTPictBoardRingMod::_CreateVertexOfBoard(
		MTPICTBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		bool isReverseMode
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long virtexIndex = 0;
	unsigned long virtexIndexStart = 0;
	D3DXVECTOR3 basePos;
	D3DXVECTOR3 rotatedPos;
	float boardHeight = 0.0f;
	float boardWidth = 0.0f;
	float chStep = 0.0f;
	float angle = 0.0f;
	float direction = 0.0f;
	D3DXVECTOR2 clipAreaP1;
	D3DXVECTOR2 clipAreaP2;
	D3DXVECTOR2 textureP1;
	D3DXVECTOR2 textureP2;

	//テクスチャクリップ領域の座標
	clipAreaP1 = D3DXVECTOR2(0.0f, 0.0f);  //左上
	clipAreaP2 = D3DXVECTOR2(1.0f, 1.0f);  //右下

	//テスクチャX座標
	if (isReverseMode) {
		textureP1.x = clipAreaP1.x;
		textureP1.y = clipAreaP2.y;
		textureP2.x = clipAreaP2.x;
		textureP2.y = clipAreaP1.y;
		direction = -1.0f;
	}
	else {
		textureP1.x = clipAreaP2.x;
		textureP1.y = clipAreaP1.y;
		textureP2.x = clipAreaP1.x;
		textureP2.y = clipAreaP2.y;
		direction = 1.0f;
	}

	//基準座標
	chStep = m_NoteDesignMod.MTNoteDesignRing::GetChStep();
	basePos = D3DXVECTOR3(
				m_NoteDesignMod.MTNoteDesignRing::GetPlayPosX(0),
				m_NoteDesignMod.MTNoteDesignRing::GetPortOriginY(0) + (chStep * (float)SM_MAX_CH_NUM) + chStep + 0.01f,
				m_NoteDesignMod.MTNoteDesignRing::GetPortOriginZ(0));
	boardHeight = 2.0f * 3.1415926f * basePos.y;
	boardWidth = boardHeight * ((float)m_ImgInfo.Width / (float)m_ImgInfo.Height);
	basePos.x -= (boardWidth * m_NoteDesignMod.MTNoteDesignRing::GetPictBoardRelativePos());

	//頂点作成：X軸回りの円筒
	virtexIndexStart = virtexIndex;
	pVertex[virtexIndex].p = basePos;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP1.x, textureP1.y);
	virtexIndex++;
	pVertex[virtexIndex].p = basePos;
	pVertex[virtexIndex].p.x += boardWidth;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP2.x, textureP1.y);
	for (i = 1; i < SM_MAX_NOTE_NUM; i++) {
		virtexIndex++;
		
		//回転後の頂点
		angle = (360.0f / (float)SM_MAX_NOTE_NUM) * (float)i;
		rotatedPos = DXH::RotateYZ(0.0f, 0.0f, basePos, angle);
		pVertex[virtexIndex].p = rotatedPos;
		pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVertex[virtexIndex].t = D3DXVECTOR2(textureP1.x, textureP1.y + (direction * (float)i / (float)SM_MAX_NOTE_NUM));
		virtexIndex++;
		pVertex[virtexIndex].p = rotatedPos;
		pVertex[virtexIndex].p.x += boardWidth;
		pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVertex[virtexIndex].t = D3DXVECTOR2(textureP2.x, textureP1.y + (direction * (float)i / (float)SM_MAX_NOTE_NUM));
		
		//インデックスバッファ
		//  直前の頂点0,1と追加した頂点2,3で三角形0-1-3と0-3-2を追加
		//  1+--+3-+--+--+--
		//   | /| /| /| /|  ..
		//   |/ |/ |/ |/ |  ..
		//  0+--+2-+--+--+--
		pIndex[(i-1)*6 + 0] = (i-1)*2 + 0;  //0 1つ目の三角形
		pIndex[(i-1)*6 + 1] = (i-1)*2 + 1;  //1 1つ目の三角形
		pIndex[(i-1)*6 + 2] = (i-1)*2 + 3;  //3 1つ目の三角形
		pIndex[(i-1)*6 + 3] = (i-1)*2 + 0;  //0 2つ目の三角形
		pIndex[(i-1)*6 + 4] = (i-1)*2 + 3;  //3 2つ目の三角形
		pIndex[(i-1)*6 + 5] = (i-1)*2 + 2;  //2 2つ目の三角形
	}
	//最後の頂点2,3は最初0,1の頂点と同じ（リングを閉じる）
	virtexIndex++;
	pVertex[virtexIndex] =pVertex[0];
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP1.x, textureP2.y);
	virtexIndex++;
	pVertex[virtexIndex] =pVertex[1];
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP2.x, textureP2.y);

	//インデックスバッファ（リングを閉じる）
	pIndex[(i-1)*6 + 0] = (i-1)*2 + 0;  //0 1つ目の三角形
	pIndex[(i-1)*6 + 1] = (i-1)*2 + 1;  //1 1つ目の三角形
	pIndex[(i-1)*6 + 2] = (i-1)*2 + 3;  //3 1つ目の三角形
	pIndex[(i-1)*6 + 3] = (i-1)*2 + 0;  //0 2つ目の三角形
	pIndex[(i-1)*6 + 4] = (i-1)*2 + 3;  //3 2つ目の三角形
	pIndex[(i-1)*6 + 5] = (i-1)*2 + 2;  //2 2つ目の三角形

	return result;
}
