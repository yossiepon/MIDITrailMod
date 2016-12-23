//******************************************************************************
//
// MIDITrail / MTGridBoxMod
//
// グリッドボックス描画Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridBoxMod.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTGridBoxMod::MTGridBoxMod(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTGridBoxMod::~MTGridBoxMod(void)
{
}

//******************************************************************************
// 移動
//******************************************************************************
int MTGridBoxMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float rollAngle
	)
{
	int result = 0;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;

	//行列初期化
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	if(rollAngle < 0.0f) {
		rollAngle += 360.0f;
	}

	if((rollAngle > 120.0f) && (rollAngle < 300.0f)) {
		rollAngle -= 180.0f;
	}

	//回転行列
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle));

	//移動行列
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);

	//行列の合成
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//変換行列設定
	m_Primitive.Transform(worldMatrix);

	return result;
}
