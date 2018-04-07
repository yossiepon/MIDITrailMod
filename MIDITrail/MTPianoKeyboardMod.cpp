//******************************************************************************
//
// MIDITrail / MTPianoKeyboardMod
//
// ピアノキーボード描画Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboardMod.h"

using namespace YNBaseLib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
//各キーの頂点数
#define MTPIANOKEYBOARD_KEY_WHITE_1_VERTEX_NUM  (38)
#define MTPIANOKEYBOARD_KEY_WHITE_2_VERTEX_NUM  (44)
#define MTPIANOKEYBOARD_KEY_WHITE_3_VERTEX_NUM  (38)
#define MTPIANOKEYBOARD_KEY_BLACK_VERTEX_NUM    (30)
#define MTPIANOKEYBOARD_KEY_VERTEX_NUM_MAX      MTPIANOKEYBOARD_KEY_WHITE_2_VERTEX_NUM

//各キーのインデックス数
#define MTPIANOKEYBOARD_KEY_WHITE_1_INDEX_NUM   (60)
#define MTPIANOKEYBOARD_KEY_WHITE_2_INDEX_NUM   (66)
#define MTPIANOKEYBOARD_KEY_WHITE_3_INDEX_NUM   (60)
#define MTPIANOKEYBOARD_KEY_BLACK_INDEX_NUM     (48)
#define MTPIANOKEYBOARD_KEY_INDEX_NUM_MAX       MTPIANOKEYBOARD_KEY_WHITE_2_INDEX_NUM


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTPianoKeyboardMod::MTPianoKeyboardMod(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTPianoKeyboardMod::~MTPianoKeyboardMod(void)
{
}

//******************************************************************************
// 生成処理
//******************************************************************************
int MTPianoKeyboardMod::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		LPDIRECT3DTEXTURE9 pTexture
	)
{
	int result = 0;
	SMTrack track;

	//基底クラスの生成処理を呼び出す
	result = MTPianoKeyboard::Create(pD3DDevice, pSceneName, pSeqData,  pTexture);
	if (result != 0) goto EXIT;
	
	//キーボードデザイン初期化
	result = m_KeyboardDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTPianoKeyboardMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 moveVector1,
		D3DXVECTOR3 moveVector2,
		float scale,
		float z,
		float rollAngle
	)
{
	int result = 0;
	D3DXMATRIX scaleMatrix;
	D3DXMATRIX rotateMatrix1;
	D3DXMATRIX rotateMatrix2;
	D3DXMATRIX rotateMatrix3;
	D3DXMATRIX moveMatrix1;
	D3DXMATRIX moveMatrix2;
	D3DXMATRIX moveMatrix3;
	D3DXMATRIX worldMatrix;

	//行列初期化
	D3DXMatrixIdentity(&scaleMatrix);
	D3DXMatrixIdentity(&rotateMatrix1);
	D3DXMatrixIdentity(&rotateMatrix2);
	D3DXMatrixIdentity(&rotateMatrix3);
	D3DXMatrixIdentity(&moveMatrix1);
	D3DXMatrixIdentity(&moveMatrix2);
	D3DXMatrixIdentity(&worldMatrix);

	//回転行列

	if(rollAngle < 0.0f) {
		rollAngle += 360.0f;
	}

	if((rollAngle > 120.0f) && (rollAngle < 300.0f)) {
		D3DXMatrixRotationX(&rotateMatrix1, D3DX_PI / 2.0f);
		D3DXMatrixRotationZ(&rotateMatrix2, D3DX_PI / 2.0f);
	} else {
		D3DXMatrixRotationX(&rotateMatrix1, -D3DX_PI / 2.0f);
		D3DXMatrixRotationZ(&rotateMatrix2, D3DX_PI / 2.0f);
	}

	D3DXMatrixRotationX(&rotateMatrix3, D3DXToRadian(rollAngle));

	//移動行列
	D3DXMatrixTranslation(&moveMatrix1, moveVector1.x, moveVector1.y, moveVector1.z);
	D3DXMatrixTranslation(&moveMatrix2, moveVector2.x, moveVector2.y, moveVector2.z);
	D3DXMatrixTranslation(&moveMatrix3, 0.0f, 0.0f, z / scale);

	//スケール行列
	D3DXMatrixScaling(&scaleMatrix, scale, scale, scale);

	//行列の合成：ピッチベンド移動１→鍵盤向き補正回転１・２→グリッド面まで移動３→ホイール回転３→スケール→再生面追従移動２
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &moveMatrix1);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix1);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix2);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &moveMatrix3);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix3);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &scaleMatrix);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &moveMatrix2);

	//変換行列設定
	m_PrimitiveKeyboard.Transform(worldMatrix);

//EXIT:;
	return result;
}

//******************************************************************************
// キーの押し込み
//******************************************************************************
int MTPianoKeyboardMod::PushKey(
		unsigned char chNo,
		unsigned char noteNo,
		float keyDownRate,
		unsigned long elapsedTime,
		D3DXCOLOR* pNoteColor
	)
{
	int result = 0;
	float angle = 0.0f;
	D3DXCOLOR color;

	if (noteNo >= SM_MAX_NOTE_NUM) {
		result = YN_SET_ERR("Program error.", noteNo, 0);
		goto EXIT;
	}

	angle = m_KeyboardDesignMod.GetKeyRotateAngle() * keyDownRate;

	if (keyDownRate < 1.0f) {
		//キーが下降中／上昇中の場合は色を変更せず回転させる
		_RotateKey(noteNo, angle);
	}
	else {
		//キーが押下状態の場合は色を変更して回転させる
		color = m_KeyboardDesignMod.GetActiveKeyColor(chNo, noteNo, elapsedTime, pNoteColor);
		_RotateKey(noteNo, angle, &color);
	}

EXIT:;
	return result;
}

