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
#define PLANE_INDEX_UP				(0)
#define PLANE_INDEX_FRONT			(1)
#define PLANE_INDEX_RIGHT			(2)
#define PLANE_INDEX_RIGHT_FRONT		(2)
#define PLANE_INDEX_MIDDLE_RIGHT	(3)
#define PLANE_INDEX_RIGHT_BACK		(4)
#define PLANE_INDEX_BACK			(5)
#define PLANE_INDEX_LEFT_BACK		(6)
#define PLANE_INDEX_MIDDLE_LEFT		(7)
#define PLANE_INDEX_LEFT_FRONT		(8)
#define PLANE_INDEX_LEFT			(8)
#define PLANE_INDEX_DOWN			(9)
#define PLANE_INDEX_WHITEKEY1_MAX	(8)
#define PLANE_INDEX_WHITEKEY2_MAX	(10)
#define PLANE_INDEX_WHITEKEY3_MAX	(8)
#define PLANE_INDEX_BLACKKEY_MAX	(6)

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTPianoKeyboardMod::MTPianoKeyboardMod(void)
{
	//逆順インデックス情報
	for (int i = 0; i < MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX; i++) {
		m_pRenderingIndexBuffer[i] = NULL;
		m_RenderingIndexNum[i] = 0;
		m_IsRenderingIndexLocked[i] = false;
	}

	//キーボード描画範囲
	m_noteNoLow = -1;
	m_noteNoHigh = -1;
	m_camDirLR = 0;
	m_camPosIdx = 2;
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

	//描画インデックス生成
	_CreateRenderingIndex(pD3DDevice);

EXIT:;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTPianoKeyboardMod::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	//テクスチャステージ設定
	//  カラー演算：乗算  引数1：テクスチャ  引数2：ポリゴン
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	// アルファ演算：乗算  引数1：テクスチャ  引数2：ポリゴン
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	//テクスチャフィルタ
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//視線方向による描画順の変更
	if (m_camDirLR == -1) {
		//低音側キーボードの描画
		if (m_noteNoLow != -1) {
			result = m_PrimitiveKeyboard.Draw(pD3DDevice, m_pRenderingIndexBuffer[m_camPosIdx + 1], m_pTexture, m_BufInfo[m_noteNoLow].indexTotal / 3);
			if (result != 0) goto EXIT;
		}

		//高音側キーボードの描画
		if (m_noteNoHigh != -1) {
			result = m_PrimitiveKeyboard.Draw(pD3DDevice, m_pRenderingIndexBuffer[m_camPosIdx], m_pTexture, m_BufInfo[m_noteNoHigh].revIndexTotal / 3);
			if (result != 0) goto EXIT;
		}
	}
	else {
		//高音側キーボードの描画
		if (m_noteNoHigh != -1) {
			result = m_PrimitiveKeyboard.Draw(pD3DDevice, m_pRenderingIndexBuffer[m_camPosIdx], m_pTexture, m_BufInfo[m_noteNoHigh].revIndexTotal / 3);
			if (result != 0) goto EXIT;
		}

		//低音側キーボードの描画
		if (m_noteNoLow != -1) {
			result = m_PrimitiveKeyboard.Draw(pD3DDevice, m_pRenderingIndexBuffer[m_camPosIdx + 1], m_pTexture, m_BufInfo[m_noteNoLow].indexTotal / 3);
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTPianoKeyboardMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 basePosVector,
		D3DXVECTOR3 playbackPosVector,
		D3DXVECTOR3 camVector,
		D3DXVECTOR3 lookVector,
		float rollAngle
	)
{
	int result = 0;
	D3DXMATRIX scaleMatrix;
	D3DXMATRIX rotateMatrix1;
	D3DXMATRIX rotateMatrix2;
	D3DXMATRIX rotateMatrix3;
	D3DXMATRIX basePosMatrix;
	D3DXMATRIX playbackPosMatrix;
	D3DXMATRIX worldMatrix;

	//行列初期化
	D3DXMatrixIdentity(&scaleMatrix);
	D3DXMatrixIdentity(&rotateMatrix1);
	D3DXMatrixIdentity(&rotateMatrix2);
	D3DXMatrixIdentity(&basePosMatrix);
	D3DXMatrixIdentity(&playbackPosMatrix);
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
	D3DXMatrixTranslation(&basePosMatrix, basePosVector.x, basePosVector.y, basePosVector.z);
	D3DXMatrixTranslation(&playbackPosMatrix, playbackPosVector.x, playbackPosVector.y, playbackPosVector.z);

	//スケール行列
	float scale = m_KeyboardDesignMod.GetKeyboardResizeRatio();
	D3DXMatrixScaling(&scaleMatrix, scale, scale, scale);

	//行列の合成：スケール→原点移動→回転１・２（鍵盤向き補正）→回転３（ホイール角度）→再生位置追従移動
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &scaleMatrix);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &basePosMatrix);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix1);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix2);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix3);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &playbackPosMatrix);

	//変換行列設定
	m_PrimitiveKeyboard.Transform(worldMatrix);

	//キーボード描画情報の生成
	_MakeRenderingInfo(basePosVector, playbackPosVector, camVector, lookVector, rollAngle);

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

//******************************************************************************
// 描画情報の作成
//******************************************************************************
int MTPianoKeyboardMod::_MakeRenderingInfo(
		D3DXVECTOR3 basePosVector,
		D3DXVECTOR3 playbackPosVector,
		D3DXVECTOR3 camVector,
		D3DXVECTOR3 lookVector,
		float rollAngle
	)
{
	//ピッチベンド幅を含んだキーボード原点
	float originXF = basePosVector.z;
	float originXB = basePosVector.z + m_KeyboardDesignMod.GetWhiteKeyLen() * m_KeyboardDesignMod.GetKeyboardResizeRatio();
	float originY = basePosVector.y;
	float originZ = basePosVector.x;

	//カメラ位置
	float camPosX = camVector.x - playbackPosVector.x;
	float camPosY = camVector.y;
	float camPosZ = -camVector.z;

	//カメラ視線方向
	float camDir = -lookVector.z;

	m_noteNoLow = (int)::floor((camPosZ - originZ) / m_KeyboardDesignMod.GetNoteStep() + SM_MAX_NOTE_NUM / 4);
	m_noteNoHigh = m_noteNoLow + 2;

	if (m_noteNoLow < 0) {
		m_noteNoLow = -1;
	}

	if (m_noteNoLow >= SM_MAX_NOTE_NUM) {
		m_noteNoLow = SM_MAX_NOTE_NUM - 1;
	}


	if (m_noteNoHigh < 0) {
		m_noteNoHigh = 0;
	}

	if (m_noteNoHigh >= SM_MAX_NOTE_NUM) {
		m_noteNoHigh = -1;
	}

	m_camDirLR = camDir < 0 ? -1 : 1;

	if (camPosX < originXF) {
		m_camPosIdx = 2;
	}
	else if (camPosX > originXB) {
		m_camPosIdx = 4;
	}
	else {
		m_camPosIdx = 0;
	}

	char buf[256];
	//::sprintf(buf, "O:%f, P:%f, L:%d, H:%d\n", origin, camPos, m_noteNoLow, m_noteNoHigh);
	//::sprintf(buf, "D:%f\n", camDir);
	//::sprintf(buf, "O:%f, P:%f\n", originY, camPosY);
	::sprintf(buf, "X:%f, Y:%f, Z:%f, D:%d, I:%d, L:%d, H:%d\n", (camPosX - originXF), (camPosY - originY), (camPosZ - originZ), m_camDirLR, m_camPosIdx, m_noteNoLow, m_noteNoHigh);
	::OutputDebugStringA(buf);

	return 0;
}

//******************************************************************************
// 描画インデックス生成
//******************************************************************************
int MTPianoKeyboardMod::_CreateRenderingIndex(LPDIRECT3DDEVICE9 pD3DDevice)
{
	int result = 0;

	//インデックスの総数
	unsigned long indexTotal = 0;

	for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		indexTotal += m_BufInfo[noteNo].indexNum;
		m_BufInfo[noteNo].indexTotal = indexTotal;
	}

	//逆順インデックスのオフセット情報を生成
	unsigned long revIndexNum = indexTotal;
	unsigned long revIndexTotal = 0;

	for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		unsigned long size = m_BufInfo[noteNo].indexNum;
		revIndexNum -= size;

		m_BufInfo[noteNo].revIndexPos = revIndexNum;
		m_BufInfo[noteNo].revIndexTotal = indexTotal- revIndexTotal;

		revIndexTotal += size;
	}

	unsigned long* pIndex = NULL;
	unsigned long size = indexTotal * sizeof(unsigned long);

	//インデックスバッファのロック
	result = m_PrimitiveKeyboard.LockIndex(&pIndex, 0, size);
	if (result != 0) goto EXIT;

	//描画インデックスバッファ生成
	for (int bufferIdx = 0; bufferIdx < MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX; bufferIdx++) {

		result = _CreateRenderingIndexBuffer(pD3DDevice, bufferIdx, indexTotal);
		if (result != 0) goto EXIT;

		unsigned long* pRenderingIndex = NULL;

		//描画インデックスバッファのロック
		result = _LockRenderingIndex(&pRenderingIndex, bufferIdx, 0, size);
		if (result != 0) goto EXIT;

		ZeroMemory(pRenderingIndex, size);

		//バッファに描画インデックスを書き込む
		for (unsigned char noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
			result = _CreateRenderingIndexOfKey(noteNo, bufferIdx, pIndex, pRenderingIndex);
			if (result != 0) goto EXIT;
		}

		//描画インデックスバッファのロック解除
		result = _UnlockRenderingIndex(bufferIdx);
		if (result != 0) goto EXIT;

	}

	//インデックスバッファのロック解除
	result = m_PrimitiveKeyboard.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// キー単位の描画インデックス生成
//******************************************************************************
int MTPianoKeyboardMod::_CreateRenderingIndexOfKey(
		unsigned char noteNo,
		int bufferIdx,
		unsigned long* pIndex,
		unsigned long* pRenderingIndex
   )
{
	int result = 0;

	if ((bufferIdx < 0) || (bufferIdx >= MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//カメラ位置分類コード： UP(0)/DOWN(1) - MID(0)/BACK(1) - MID(0)/FRONT(1) - LEFT(0)/RIGHT(1)

	//白鍵1 キー平面描画順
	static int idxWhiteKey1[][PLANE_INDEX_WHITEKEY1_MAX] =
	{
		//0000
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_UP
		},
		//0001
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_UP
		},
		//0010
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_UP
		},
		//0011
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_BACK,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_UP
		},
		//0100
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_UP
		},
		//0101
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_UP
		},

		//1000
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_DOWN
		},
		//1001
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_DOWN
		},
		//1010
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_DOWN
		},
		//1011
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_BACK,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_DOWN
		},
		//1100
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_DOWN
		},
		//1101
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_DOWN
		},
	};

	//白鍵2 キー平面描画順
	static int idxWhiteKey2[][PLANE_INDEX_WHITEKEY2_MAX] =
	{
		//0000
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_UP
		},
		//0001
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_UP
		},
		//0010
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_UP
		},
		//0011
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_BACK,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_UP
		},
		//0100
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_UP
		},
		//0101
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_UP
		},

		//1000
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_DOWN
		},
		//1001
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_DOWN
		},
		//1010
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_DOWN
		},
		//1011
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_BACK,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_RIGHT_BACK,		PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_FRONT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_DOWN
		},
		//1100
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_DOWN
		},
		//1101
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_RIGHT_FRONT,	PLANE_INDEX_MIDDLE_RIGHT,	PLANE_INDEX_RIGHT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_DOWN
		},
	};

	//白鍵3 キー平面描画順
	static int idxWhiteKey3[][PLANE_INDEX_WHITEKEY3_MAX] =
	{
		//0000
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_UP
		},
		//0001
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_UP
		},
		//0010
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_UP
		},
		//0011
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_BACK,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_UP
		},
		//0100
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_UP
		},
		//0101
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_UP
		},

		//1000
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_DOWN
		},
		//1001
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_DOWN
		},
		//1010
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_DOWN
		},
		//1011
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_BACK,
			PLANE_INDEX_LEFT_BACK,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_FRONT,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_DOWN
		},
		//1100
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_BACK,
			PLANE_INDEX_DOWN
		},
		//1101
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT_FRONT,		PLANE_INDEX_MIDDLE_LEFT,	PLANE_INDEX_LEFT_BACK,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_DOWN
		},
	};

	//黒鍵 キー平面描画順
	static int idxBlackKey[][PLANE_INDEX_BLACKKEY_MAX] =
	{
		//0000
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_UP
		},
		//0001
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_UP
		},
		//0010
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_UP
		},
		//0011
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_BACK,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_UP
		},
		//0100
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_UP
		},
		//0101
		{
			PLANE_INDEX_DOWN,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_UP
		},

		//1000
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_DOWN
		},
		//1001
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_DOWN
		},
		//1010
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_BACK,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_DOWN
		},
		//1011
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_BACK,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_DOWN
		},
		//1100
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_DOWN
		},
		//1101
		{
			PLANE_INDEX_UP,
			PLANE_INDEX_FRONT,
			PLANE_INDEX_LEFT,
			PLANE_INDEX_RIGHT,
			PLANE_INDEX_BACK,
			PLANE_INDEX_DOWN
		},
	};

	unsigned long srcOffset = m_BufInfo[noteNo].indexPos;
	// カメラ視点より右のキー（カメラ位置分類コードがLEFT）は高音側から描画する
	unsigned long dstOffset = bufferIdx % 2 == 0 ? m_BufInfo[noteNo].revIndexPos : srcOffset;
	unsigned long size = m_BufInfo[noteNo].indexNum;

	int *pPlaneIndex;
	int planeIndexSize;

	switch (m_BufInfo[noteNo].keyType)
	{
		case MTPianoKeyboardMod::KeyWhite1:
			pPlaneIndex = idxWhiteKey1[bufferIdx];
			planeIndexSize = PLANE_INDEX_WHITEKEY1_MAX;
		break;
		case MTPianoKeyboardMod::KeyWhite2:
			pPlaneIndex = idxWhiteKey2[bufferIdx];
			planeIndexSize = PLANE_INDEX_WHITEKEY2_MAX;
		break;
		case MTPianoKeyboardMod::KeyWhite3:
			pPlaneIndex = idxWhiteKey3[bufferIdx];
			planeIndexSize = PLANE_INDEX_WHITEKEY3_MAX;
		break;
		case MTPianoKeyboardMod::KeyBlack:
			pPlaneIndex = idxBlackKey[bufferIdx];
			planeIndexSize = PLANE_INDEX_BLACKKEY_MAX;
		break;
		default:
			pPlaneIndex = NULL;
			planeIndexSize = 0;
		break;
	}

	for (int i = 0; i < planeIndexSize; i++) {
		int planeIndex = pPlaneIndex[i];
		int planeOffset = m_BufInfo[noteNo].planeIndexPos[planeIndex];
		int planeSize = m_BufInfo[noteNo].planeIndexNum[planeIndex];
		::memcpy(pRenderingIndex + dstOffset, pIndex + srcOffset + planeOffset, sizeof(unsigned long) * planeSize);
		dstOffset += planeSize;
	}

EXIT:
	return result;
}

//******************************************************************************
// 描画インデックスバッファ生成
//******************************************************************************
int MTPianoKeyboardMod::_CreateRenderingIndexBuffer(
		LPDIRECT3DDEVICE9 pD3DDevice,
		int bufferIdx,
		unsigned long indexNum
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	
	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if ((bufferIdx < 0) || (bufferIdx >= MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (m_pRenderingIndexBuffer[bufferIdx] != NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	//インデックスバッファ生成
	if (indexNum > 0) {
		hresult = pD3DDevice->CreateIndexBuffer(
						sizeof(unsigned long) * indexNum,		//インデックスバッファの全体サイズ(byte)
						D3DUSAGE_WRITEONLY,						//使用方法
						D3DFMT_INDEX32,							//インデックスバッファのフォーマット
						D3DPOOL_MANAGED,						//リソース配置場所となるメモリクラス
						&m_pRenderingIndexBuffer[bufferIdx],	//作成されたインデックスバッファ
						NULL									//予約パラメータ
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, indexNum);
			goto EXIT;
		}
	}

	m_RenderingIndexNum[bufferIdx] = indexNum;

EXIT:;
	return result;
}

//******************************************************************************
// 描画インデックスバッファロック
//******************************************************************************
int MTPianoKeyboardMod::_LockRenderingIndex(
		unsigned long** pPtrIndex,
		int bufferIdx,
		unsigned long offset,	//省略時はゼロ
		unsigned long size		//省略時はゼロ
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	if ((bufferIdx < 0) || (bufferIdx >= MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (m_IsRenderingIndexLocked[bufferIdx]) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if ((sizeof(unsigned long) * m_RenderingIndexNum[bufferIdx]) < (offset + size)) {
		result = YN_SET_ERR("Program error.", offset, size);
		goto EXIT;
	}

	//インデックスバッファのロックとバッファメモリポインタ取得
	if (m_pRenderingIndexBuffer[bufferIdx] != NULL) {
		hresult = m_pRenderingIndexBuffer[bufferIdx]->Lock(
						offset,		//ロックするインデックスのオフセット(byte)
						size,		//ロックするインデックスのサイズ(byte)
						(void**)pPtrIndex,	//バッファメモリポインタ
						0			//ロッキングフラグ
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, (DWORD64)pPtrIndex);
			goto EXIT;
		}
	}

	m_IsRenderingIndexLocked[bufferIdx] = true;

EXIT:;
	return result;
}

//******************************************************************************
// 描画インデックスバッファロック解除
//******************************************************************************
int MTPianoKeyboardMod::_UnlockRenderingIndex(
		int bufferIdx
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	if (m_IsRenderingIndexLocked[bufferIdx]) {
		if (m_pRenderingIndexBuffer[bufferIdx] != NULL) {
			hresult = m_pRenderingIndexBuffer[bufferIdx]->Unlock();
			if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
				goto EXIT;
			}
		}
		m_IsRenderingIndexLocked[bufferIdx] = false;
	}

EXIT:;
	return result;
}

//******************************************************************************
// キーボード頂点生成：白鍵A
//******************************************************************************
int MTPianoKeyboardMod::_CreateVertexOfKeyWhite1(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	//基底クラスの頂点生成処理を呼び出す
	int result = MTPianoKeyboard::_CreateVertexOfKeyWhite1(
			noteNo,
			pVertex,
			pIndex,
			pColor
		);

	//キー種別
	m_BufInfo[noteNo].keyType = MTPianoKeyboardMod::KeyWhite1;

	//描画順の再生成
	int idx = 0;

	//----------------------------------------------------------------
	//上の面
	//----------------------------------------------------------------
	// 6+--+5
	//  |  |
	//  |  |
	//  |  |4
	// 3+--+--+2
	//  |     |   +z
	//  |     |    |
	//  |     |    |
	// 0+-----+1   +---> +x
	//     |
	//    posX

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_UP] = idx;

	//インデックス
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 3, 5, 4, 3, 6, 5 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_UP] = sizeof(indexUP) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 0-1
	//----------------------------------------------------------------
	// 0      1
	// 7+----+8
	//  |    |
	// 9+----+10

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_FRONT] = idx;

	//インデックス
	unsigned long index01[] = { 7, 8, 9, 8, 10, 9 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_FRONT] = sizeof(index01) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 1-2
	//----------------------------------------------------------------
	// 2 12+--+14
	//     |  |
	//     |  |
	// 1 11+--+13

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_RIGHT_FRONT] = idx;

	//インデックス
	unsigned long index12[] = { 11, 12, 13, 12, 14, 13 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index12[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_RIGHT_FRONT] = sizeof(index12) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 2-4
	//----------------------------------------------------------------
	//   18+--+17
	//     |  |
	// 4 16+--+15 2

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_MIDDLE_RIGHT] = idx;

	//インデックス
	unsigned long index24[] = { 15, 16, 17, 16, 18, 17 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index24[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_MIDDLE_RIGHT] = sizeof(index24) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 4-5
	//----------------------------------------------------------------
	// 5 20+--+22
	//     |  |
	//     |  |
	// 4 19+--+21

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_RIGHT_BACK] = idx;

	//インデックス
	unsigned long index45[] = { 19, 20, 21, 20, 22, 21 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_RIGHT_BACK] = sizeof(index45) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 5-6
	//----------------------------------------------------------------
	//   26+--+25
	//     |  |
	// 6 24+--+23 5

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_BACK] = idx;

	//インデックス
	unsigned long index56[] = { 23, 24, 25, 24, 26, 25 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_BACK] = sizeof(index56) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 6-0
	//----------------------------------------------------------------
	// 29+--+27 6
	//   |  |
	//   |  |
	// 30+--+28 0

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_LEFT] = idx;

	//インデックス
	unsigned long index60[] = { 27, 28, 29, 28, 30, 29 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index60[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_LEFT] = sizeof(index60) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//下の面
	//----------------------------------------------------------------
	//  37 6+--+5 36
	//      |  |
	//      |  |
	//      |  |4 35
	//  34 3+--+--+2 33
	//      |     |     +z
	//      |     |      |
	//      |     |      |
	//  31 0+-----+1 32  +---> +x
	//         |
	//        posX

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_DOWN] = idx;

	//下面インデックス
	unsigned long indexDW[] = { 31, 32, 33, 31, 33, 34, 34, 35, 36, 34, 36, 37 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_DOWN] = sizeof(indexDW) / sizeof(unsigned long);

	return result;
}

//******************************************************************************
// キーボード頂点生成：白鍵B
//******************************************************************************
int MTPianoKeyboardMod::_CreateVertexOfKeyWhite2(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	//基底クラスの頂点生成処理を呼び出す
	int result = MTPianoKeyboard::_CreateVertexOfKeyWhite2(
			noteNo,
			pVertex,
			pIndex,
			pColor
		);

	D3DXCOLOR keyColor;
	D3DXVECTOR2 tsc;

	//白鍵カラー
	if (pColor == NULL) {
		keyColor = m_KeyboardDesign.GetWhiteKeyColor();
	}
	else {
		keyColor = *pColor;
	}

	//キー種別
	m_BufInfo[noteNo].keyType = MTPianoKeyboardMod::KeyWhite2;

	//描画順の再生成
	int idx = 0;

	//----------------------------------------------------------------
	//上の面
	//----------------------------------------------------------------
	//   6+-+5
	//    | |
	//    | |
	//   7| |4
	// 3+-+-+-+2
	//  |     |   +z
	//  |     |    |
	//  |     |    |
	// 0+-----+1   +---> +x
	//     |
	//    posX

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_UP] = idx;

	//インデックス
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 7, 5, 4, 7, 6, 5 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_UP] = sizeof(indexUP) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 0-1
	//----------------------------------------------------------------
	//  0      1
	//  8+----+9
	//   |    |
	// 10+----+11

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_FRONT] = idx;

	//インデックス
	unsigned long index01[] = { 8, 9, 10, 9, 11, 10 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_FRONT] = sizeof(index01) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 1-2
	//----------------------------------------------------------------
	// 2 13+--+15
	//     |  |
	//     |  |
	// 1 12+--+14

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_RIGHT_FRONT] = idx;

	//インデックス
	unsigned long index12[] = { 12, 13, 14, 13, 15, 14 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index12[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_RIGHT_FRONT] = sizeof(index12) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 2-4
	//----------------------------------------------------------------
	//   19+--+18
	//     |  |
	// 4 17+--+16 2

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_MIDDLE_RIGHT] = idx;

	//基底クラスでは2-3で生成しているが、透過すると4-7間が仕切りとして
	//見えてしまうため、2-4、7-3に分割する

	//頂点
	pVertex[17].p = pVertex[4].p;
	pVertex[19].p = D3DXVECTOR3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'

	//インデックス
	unsigned long index24[] = { 16, 17, 18, 17, 19, 18 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index24[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_MIDDLE_RIGHT] = sizeof(index24) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 4-5
	//----------------------------------------------------------------
	// 5 21+--+23
	//     |  |
	//     |  |
	// 4 20+--+22

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_RIGHT_BACK] = idx;

	//インデックス
	unsigned long index45[] = { 20, 21, 22, 21, 23, 22 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_RIGHT_BACK] = sizeof(index45) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 5-6
	//----------------------------------------------------------------
	//   27+--+26
	//     |  |
	// 6 25+--+24 5

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_BACK] = idx;

	//インデックス
	unsigned long index56[] = { 24, 25, 26, 25, 27, 26 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_BACK] = sizeof(index56) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 6-7
	//----------------------------------------------------------------
	// 30+--+28 6
	//   |  |
	//   |  |
	// 31+--+29 7

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_LEFT_BACK] = idx;

	//インデックス
	unsigned long index67[] = { 28, 29, 30, 29, 31, 30 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index67[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_LEFT_BACK] = sizeof(index67) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 7-3
	//----------------------------------------------------------------
	//   47+--+46
	//     |  |
	// 3 45+--+44 7

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_MIDDLE_LEFT] = idx;

	//頂点
	pVertex[44].p = pVertex[7].p;
	pVertex[45].p = pVertex[3].p;
	pVertex[46].p = D3DXVECTOR3(pVertex[7].p.x, 0.0f, pVertex[7].p.z);  // 7'
	pVertex[47].p = D3DXVECTOR3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'

	//法線／色
	for (int i = 44; i < 48; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor;
	}

	//インデックス
	unsigned long index73[] = { 44, 45, 46, 45, 47, 46 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index73[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_MIDDLE_LEFT] = sizeof(index73) / sizeof(unsigned long);

	//単一色のテクスチャ座標
	m_KeyboardDesign.GetWhiteKeyTexturePosSingleColor(noteNo, &tsc);
	for (int i = 44; i < 48; i++) {
		pVertex[i].t = tsc;
	}

	//----------------------------------------------------------------
	//側面 3-0
	//----------------------------------------------------------------
	// 34+--+32 3
	//   |  |
	//   |  |
	// 35+--+33 0

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_LEFT_FRONT] = idx;

	//インデックス
	unsigned long index30[] = { 32, 33, 34, 33, 35, 34 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index30[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_LEFT_FRONT] = sizeof(index30) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//下の面
	//----------------------------------------------------------------
	//   42 6+-+5 41
	//       | |
	//       | |
	//   43 7| |4 40
	// 39 3+-+-+-+2 38
	//     |     |     +z
	//     |     |      |
	//     |     |      |
	// 36 0+-----+1 37  +---> +x
	//        |
	//       posX

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_DOWN] = idx;

	//インデックス
	unsigned long indexDW[] = { 36, 37, 38, 36, 38, 39, 43, 40, 41, 43, 41, 42 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_DOWN] = sizeof(indexDW) / sizeof(unsigned long);

	return result;
}

//******************************************************************************
// キーボード頂点生成：白鍵C
//******************************************************************************
int MTPianoKeyboardMod::_CreateVertexOfKeyWhite3(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	//基底クラスの頂点生成処理を呼び出す
	int result = MTPianoKeyboard::_CreateVertexOfKeyWhite3(
			noteNo,
			pVertex,
			pIndex,
			pColor
		);

	//キー種別
	m_BufInfo[noteNo].keyType = MTPianoKeyboardMod::KeyWhite3;

	//描画順の再生成
	int idx = 0;

	//----------------------------------------------------------------
	//上の面
	//----------------------------------------------------------------
	//    5+--+4
	//     |  |
	//     |  |
	//    6|  |
	// 3+--+--+2
	//  |     |   +z
	//  |     |    |
	//  |     |    |
	// 0+-----+1   +---> +x
	//     |
	//    posX

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_UP] = idx;

	//インデックス
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 2, 6, 4, 6, 5, 4 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_UP] = sizeof(indexUP) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 0-1
	//----------------------------------------------------------------
	// 0      1
	// 7+----+8
	//  |    |
	// 9+----+10

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_FRONT] = idx;

	//インデックス
	unsigned long index01[] = { 7, 8, 9, 8, 10, 9 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_FRONT] = sizeof(index01) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 1-4
	//----------------------------------------------------------------
	// 4 12+--+14
	//     |  |
	//     |  |
	// 1 11+--+13

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_RIGHT] = idx;

	//インデックス
	unsigned long index14[] = { 11, 12, 13, 12, 14, 13 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index14[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_RIGHT] = sizeof(index14) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 4-5
	//----------------------------------------------------------------
	//   18+--+17
	//     |  |
	// 5 16+--+15 4

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_BACK] = idx;

	//インデックス
	unsigned long index45[] = { 15, 16, 17, 16, 18, 17 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_BACK] = sizeof(index45) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 5-6
	//----------------------------------------------------------------
	// 21+--+19 5
	//   |  |
	//   |  |
	// 22+--+20 6

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_LEFT_BACK] = idx;

	//インデックス
	unsigned long index56[] = { 19, 20, 21, 20, 22, 21 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_LEFT_BACK] = sizeof(index56) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 6-3
	//----------------------------------------------------------------
	//   26+--+25
	//     |  |
	// 3 24+--+23 6

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_MIDDLE_LEFT] = idx;

	//インデックス
	unsigned long index63[] = { 23, 24, 25, 24, 26, 25 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index63[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_MIDDLE_LEFT] = sizeof(index63) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 3-0
	//----------------------------------------------------------------
	// 29+--+27 3
	//   |  |
	//   |  |
	// 30+--+28 0

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_LEFT_FRONT] = idx;

	//インデックス
	unsigned long index30[] = { 27, 28, 29, 28, 30, 29 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index30[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_LEFT_FRONT] = sizeof(index30) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//下の面
	//----------------------------------------------------------------
	//    36 5+--+4 35
	//        |  |
	//        |  |
	//    37 6|  |
	// 34 3+--+--+2 33
	//     |     |     +z
	//     |     |      |
	//     |     |      |
	// 31 0+-----+1 32  +---> +x
	//        |
	//       posX

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_DOWN] = idx;

	//インデックス
	unsigned long indexDW[] = { 31, 32, 33, 31, 33, 34, 33, 35, 37, 37, 35, 36 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_DOWN] = sizeof(indexDW) / sizeof(unsigned long);

	return result;
}

//******************************************************************************
// キーボード頂点生成：黒鍵
//******************************************************************************
int MTPianoKeyboardMod::_CreateVertexOfKeyBlack(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	//基底クラスの頂点生成処理を呼び出す
	int result = MTPianoKeyboard::_CreateVertexOfKeyBlack(
			noteNo,
			pVertex,
			pIndex,
			pColor
		);

	//キー種別
	m_BufInfo[noteNo].keyType = MTPianoKeyboardMod::KeyBlack;

	//描画順の修正
	int idx = 0;

	//----------------------------------------------------------------
	//上の面
	//----------------------------------------------------------------
	//   6+-+5
	//    | |
	//    | |
	// 7 3+-+2 4
	//   0+-+1
	//     |   +z
	//     |    |
	//     |    |
	//   --+--  +---> +x
	//     |
	//    posX

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_UP] = idx;

	//インデックス
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 4, 7, 5, 7, 6, 5 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_UP] = sizeof(indexUP) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 0-1
	//----------------------------------------------------------------
	//  0      1
	//  8+----+9
	//   |    |
	// 10+----+11

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_FRONT] = idx;

	//インデックス
	unsigned long index01[] = { 8, 9, 10, 9, 11, 10 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_FRONT] = sizeof(index01) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 1-2-5
	//----------------------------------------------------------------
	// 5 14+--+16
	//     |  |
	//     |  |
	// 2 13+  |
	//      \ |
	// 1 12 +-+15

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_RIGHT] = idx;

	//インデックス
	unsigned long index125[] = { 12, 13, 15, 13, 16, 15, 13, 14, 16 };
	for (int i = 0; i < 9; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index125[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_RIGHT] = sizeof(index125) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 5-6
	//----------------------------------------------------------------
	//   20+--+19
	//     |  |
	// 6 18+--+17 5

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_BACK] = idx;

	//インデックス
	unsigned long index56[] = { 17, 18, 19, 18, 20, 19 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_BACK] = sizeof(index56) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//側面 6-3-0
	//----------------------------------------------------------------
	// 24+--+21 6
	//   |  |
	//   |  |
	//   |  +22 3
	//   | /
	// 25+-+23  0

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_LEFT] = idx;

	//インデックス
	unsigned long index630[] = { 21, 22, 24, 22, 25, 24, 22, 23, 25 };
	for (int i = 0; i < 9; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index630[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_LEFT] = sizeof(index630) / sizeof(unsigned long);

	//----------------------------------------------------------------
	//下の面
	//----------------------------------------------------------------
	//   29 6+-+5 28
	//       | |
	//       | |
	//       | |
	//   26 0+-+1 27
	//        |      +z
	//        |       |
	//        |       |
	//      --+--     +---> +x
	//        |
	//       posX

	m_BufInfo[noteNo].planeIndexPos[PLANE_INDEX_DOWN] = idx;

	//インデックス
	unsigned long indexDW[] = { 26, 27, 28, 26, 28, 29 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	m_BufInfo[noteNo].planeIndexNum[PLANE_INDEX_DOWN] = sizeof(indexDW) / sizeof(unsigned long);

	return result;
}

