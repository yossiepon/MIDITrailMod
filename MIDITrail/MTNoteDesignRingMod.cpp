//******************************************************************************
//
// MIDITrail / MTNoteDesignRingMod
//
// ノートデザインリングModクラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTNoteDesignRingMod.h"
#include "DXH.h"


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteDesignRingMod::MTNoteDesignRingMod(void)
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteDesignRingMod::~MTNoteDesignRingMod(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int MTNoteDesignRingMod::Initialize(
	const TCHAR* pSceneName,
	SMSeqData* pSeqData
)
{
	int result = 0;

	result = MTNoteDesignRing::Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = MTNoteDesignMod::Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ノートボックス中心座標取得
//******************************************************************************
D3DXVECTOR3 MTNoteDesignRingMod::GetNoteBoxCenterPosX(
	unsigned long curTickTime,
	unsigned char portNo,
	unsigned char chNo,
	unsigned char noteNo,
	short pitchBendValue,				//省略可：ピッチベンド
	unsigned char pitchBendSensitivity	//省略可：ピッチベンド感度
)
{
	return MTNoteDesignRing::GetNoteBoxCenterPosX(curTickTime, portNo, chNo, noteNo, pitchBendValue, pitchBendSensitivity);
}

//******************************************************************************
// ノートボックス頂点座標取得
//******************************************************************************
void MTNoteDesignRingMod::GetNoteBoxVirtexPos(
	unsigned long curTickTime,
	unsigned char portNo,
	unsigned char chNo,
	unsigned char noteNo,
	D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
	D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
	D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
	D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
	short pitchBendValue,				//省略可：ピッチベンド
	unsigned char pitchBendSensitivity	//省略可：ピッチベンド感度

)
{
	MTNoteDesignRing::GetNoteBoxVirtexPos(
			curTickTime, portNo, chNo, noteNo, pVector0, pVector1, pVector2, pVector3, pitchBendValue, pitchBendSensitivity);
}

//******************************************************************************
// 発音中ノートボックス頂点座標取得
//******************************************************************************
void MTNoteDesignRingMod::GetActiveNoteBoxVirtexPos(
	unsigned long curTickTime,
	unsigned char portNo,
	unsigned char chNo,
	unsigned char noteNo,
	D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
	D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
	D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
	D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
	short pitchBendValue,				//省略可：ピッチベンド
	unsigned char pitchBendSensitivity,	//省略可：ピッチベンド感度
	unsigned long elapsedTime			//省略可：経過時間（ミリ秒）
)
{
	MTNoteDesignRing::GetActiveNoteBoxVirtexPos(
			curTickTime, portNo, chNo, noteNo, pVector0, pVector1, pVector2, pVector3, pitchBendValue, pitchBendSensitivity, elapsedTime);
}

//******************************************************************************
// 発音中ノートボックス頂点座標取得
//******************************************************************************
void MTNoteDesignRingMod::GetActiveNoteBoxVirtexPos(
	unsigned long curTickTime,
	unsigned char portNo,
	unsigned char chNo,
	unsigned char noteNo,
	D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
	D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
	D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
	D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
	short pitchBendValue,				//省略可：ピッチベンド
	unsigned char pitchBendSensitivity,	//省略可：ピッチベンド感度
	float rate							//省略可：サイズ比率
)
{
	D3DXVECTOR3 basePos0;
	D3DXVECTOR3 basePos1;
	D3DXVECTOR3 basePos2;
	float angle0 = 0.0f;
	float angle1 = 0.0f;
	float angle2 = 0.0f;
	float curSizeRatio = 1.0f;

	if (rate > 0.0f) {
		curSizeRatio = 1.0f + (MTNoteDesignMod::m_ActiveNoteBoxSizeRatio - 1.0f) * GetDecayCoefficient(rate, 30.0f);
	}

	//ノート基準座標
	basePos0 = _GetNoteBasePos(curTickTime, portNo, chNo);
	basePos1 = basePos0;
	basePos1.y -= MTNoteDesignMod::GetNoteBoxWidth() * curSizeRatio / 2.0f;
	basePos2 = basePos0;
	basePos2.y += MTNoteDesignMod::GetNoteBoxWidth() * curSizeRatio / 2.0f;

	//ノート番号で角度を決定
	angle0 = _GetNoteAngle(noteNo, pitchBendValue, pitchBendSensitivity);
	angle1 = angle0 - (m_NoteAngleStep * curSizeRatio / 2.0f);
	angle2 = angle0 + (m_NoteAngleStep * curSizeRatio / 2.0f);

	*pVector0 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle1);
	*pVector1 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle2);
	*pVector2 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle1);
	*pVector3 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle2);
}

//******************************************************************************
// ライブモニタ用ノートボックス頂点座標取得
//******************************************************************************
void MTNoteDesignRingMod::GetNoteBoxVirtexPosLive(
	unsigned long elapsedTime,
	unsigned char portNo,
	unsigned char chNo,
	unsigned char noteNo,
	D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
	D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
	D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
	D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
	short pitchBendValue,				//省略可：ピッチベンド
	unsigned char pitchBendSensitivity	//省略可：ピッチベンド感度
)
{
	MTNoteDesignRing::GetNoteBoxVirtexPosLive(
		elapsedTime, portNo, chNo, noteNo, pVector0, pVector1, pVector2, pVector3, pitchBendValue, pitchBendSensitivity);
}

//******************************************************************************
// ポート原点Y座標取得
//******************************************************************************
float MTNoteDesignRingMod::GetPortOriginY(
	unsigned char portNo
)
{
	return MTNoteDesignRing::GetPortOriginY(portNo);
}

//******************************************************************************
// ポート原点Z座標取得
//******************************************************************************
float MTNoteDesignRingMod::GetPortOriginZ(
	unsigned char portNo
)
{
	return MTNoteDesignRing::GetPortOriginZ(portNo);
}

//******************************************************************************
// 世界座標配置移動ベクトル取得
//******************************************************************************
D3DXVECTOR3 MTNoteDesignRingMod::GetWorldMoveVector()
{
	return MTNoteDesignRing::GetWorldMoveVector();
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTNoteDesignRingMod::_LoadConfFile(
	const TCHAR* pSceneName
)
{
	int result = 0;

	result = MTNoteDesignRing::_LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	result = MTNoteDesignMod::_LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


