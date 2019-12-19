//******************************************************************************
//
// MIDITrail / MTNoteDesignRing
//
// ノートデザインリングクラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTNoteDesignRing.h"
#include "DXH.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteDesignRing::MTNoteDesignRing(void)
{
	m_NoteAngleStep = 360.0f / (float)SM_MAX_NOTE_NUM;
	m_RingRadius = 0.0f;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteDesignRing::~MTNoteDesignRing(void)
{
}

//******************************************************************************
// ノートボックス中心座標取得
//******************************************************************************
D3DXVECTOR3 MTNoteDesignRing::GetNoteBoxCenterPosX(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		short pitchBendValue,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity	//省略可：ピッチベンド感度
	)
{
	D3DXVECTOR3 basePos;
	D3DXVECTOR3 notePos;
	float angle = 0.0f;

	//ノート基準座標
	basePos = _GetNoteBasePos(curTickTime, portNo, chNo);

	//ノート番号で角度を決定
	angle = _GetNoteAngle(noteNo, pitchBendValue, pitchBendSensitivity);

	//X軸回転
	notePos = DXH::RotateYZ(0.0f, 0.0f, basePos, angle);

	return notePos;
}

//******************************************************************************
// ノートボックス頂点座標取得
//******************************************************************************
void MTNoteDesignRing::GetNoteBoxVirtexPos(
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
	D3DXVECTOR3 basePos0;
	D3DXVECTOR3 basePos1;
	D3DXVECTOR3 basePos2;
	float angle0 = 0.0f;
	float angle1 = 0.0f;
	float angle2 = 0.0f;

	//ノート基準座標
	basePos0 = _GetNoteBasePos(curTickTime, portNo, chNo);
	basePos1 = basePos0;
	basePos1.y -= GetNoteBoxWidth() / 2.0f;
	basePos2 = basePos0;
	basePos2.y += GetNoteBoxWidth() / 2.0f;

	//ノート番号で角度を決定
	angle0 = _GetNoteAngle(noteNo, pitchBendValue, pitchBendSensitivity);
	angle1 = angle0 - (m_NoteAngleStep / 2.0f);
	angle2 = angle0 + (m_NoteAngleStep / 2.0f);

	*pVector0 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle1);
	*pVector1 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle2);
	*pVector2 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle1);
	*pVector3 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle2);
}

//******************************************************************************
// 発音中ノートボックス頂点座標取得
//******************************************************************************
void MTNoteDesignRing::GetActiveNoteBoxVirtexPos(
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
	D3DXVECTOR3 basePos0;
	D3DXVECTOR3 basePos1;
	D3DXVECTOR3 basePos2;
	float angle0 = 0.0f;
	float angle1 = 0.0f;
	float angle2 = 0.0f;
	float curSizeRatio = 1.0f;

	if (elapsedTime < (unsigned long)m_ActiveNoteDuration) {
		curSizeRatio = 1.0f + (m_ActiveNoteBoxSizeRatio - 1.0f) * (1.0f - (float)elapsedTime / (float)m_ActiveNoteDuration);
	}

	//ノート基準座標
	basePos0 = _GetNoteBasePos(curTickTime, portNo, chNo);
	basePos1 = basePos0;
	basePos1.y -= GetNoteBoxWidth() * curSizeRatio / 2.0f;
	basePos2 = basePos0;
	basePos2.y += GetNoteBoxWidth() * curSizeRatio / 2.0f;

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
void MTNoteDesignRing::GetNoteBoxVirtexPosLive(
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
	D3DXVECTOR3 basePos0;
	D3DXVECTOR3 basePos1;
	D3DXVECTOR3 basePos2;
	float angle0 = 0.0f;
	float angle1 = 0.0f;
	float angle2 = 0.0f;
	float x = 0.0f;
	unsigned long tickTimeDummy = 0;

	x = -(GetLivePosX(elapsedTime));

	//ノート基準座標
	basePos0 = _GetNoteBasePos(tickTimeDummy, portNo, chNo);
	basePos0.x = x;
	basePos1 = basePos0;
	basePos1.y -= GetNoteBoxWidth() / 2.0f;
	basePos2 = basePos0;
	basePos2.y += GetNoteBoxWidth() / 2.0f;

	//ノート番号で角度を決定
	angle0 = _GetNoteAngle(noteNo, pitchBendValue, pitchBendSensitivity);
	angle1 = angle0 - (m_NoteAngleStep / 2.0f);
	angle2 = angle0 + (m_NoteAngleStep / 2.0f);

	*pVector0 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle1);
	*pVector1 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle2);
	*pVector2 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle1);
	*pVector3 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle2);
}

//******************************************************************************
// グリッドリング基準座標取得
//******************************************************************************
void MTNoteDesignRing::GetGridRingBasePos(
		unsigned long totalTickTime,
		D3DXVECTOR3* pBasePosStart,
		D3DXVECTOR3* pBasePosEnd
	)
{
	*pBasePosStart = D3DXVECTOR3(
							GetPlayPosX(0),
							GetPortOriginY(0),
							GetPortOriginZ(0));
	*pBasePosEnd   = D3DXVECTOR3(
							GetPlayPosX(totalTickTime),
							GetPortOriginY(0),
							GetPortOriginZ(0));
}

//******************************************************************************
// ライブモニタ用グリッドリング基準座標取得
//******************************************************************************
void MTNoteDesignRing::GetGridRingBasePosLive(
		D3DXVECTOR3* pBasePosStart,
		D3DXVECTOR3* pBasePosEnd
	)
{
	unsigned long elapsedTime = 0;

	elapsedTime = GetLiveMonitorDisplayDuration();
	*pBasePosStart = D3DXVECTOR3(
							GetPlayPosX(0),
							GetPortOriginY(0),
							GetPortOriginZ(0));
	*pBasePosEnd   = D3DXVECTOR3(
							-(GetLivePosX(elapsedTime)),
							GetPortOriginY(0),
							GetPortOriginZ(0));
}

//******************************************************************************
// ポート原点Y座標取得
//******************************************************************************
float MTNoteDesignRing::GetPortOriginY(
		unsigned char portNo
	)
{
	float portIndex = 0.0f;
	float portWidth = 0.0f;

	portIndex = (float)(m_PortIndex[portNo]);
	portWidth = GetChStep() * (float)SM_MAX_CH_NUM;

	//   +y
	//    |
	//    @-- Note#0,127 @:Origin(for portB)
	//    |
	//    @-- Note#0,127 @:Origin(for portA)
	//    | |
	//    | | Radius
	//    | |
	// ---0----->+x(time)
	//    |
	//    |
	//    |
	//    *-- PortA
	//    |
	//    *-- PortB
	//    |
	//   -y

	return (m_RingRadius + (portWidth * portIndex));
}

//******************************************************************************
// ポート原点Z座標取得
//******************************************************************************
float MTNoteDesignRing::GetPortOriginZ(
		unsigned char portNo
	)
{
	//               +y
	//                |
	//           *****@*****      @:Origin(for portB)
	//         **     |     **
	//        *    ***@***    *   @:Origin(for portA)
	//       *   **   |   **   *
	//      *   *     |     *   *
	//      *  *      |      *  *
	// +z<--*--*------0------*--*-->-z
	//      *  *      |      *  *
	//      *   *     |     *   *
	//       *   **   |   **   *
	//        *    *******    *
	//         **     |     **
	//           ***********
	//                |
	//               -y

	return (0.0f);
}

//******************************************************************************
// 世界座標配置移動ベクトル取得
//******************************************************************************
D3DXVECTOR3 MTNoteDesignRing::GetWorldMoveVector()
{
	D3DXVECTOR3 vector;

	vector.x = 0.0f;
	vector.y = 0.0f;
	vector.z = 0.0f;

	return vector;
}

//******************************************************************************
// ノート基準座標取得
//******************************************************************************
D3DXVECTOR3 MTNoteDesignRing::_GetNoteBasePos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo
	)
{
	D3DXVECTOR3 vector;

	//演奏位置
	vector.x = GetPlayPosX(curTickTime);

	//ポート番号・チャンネル番号で原点を決定
	vector.y = GetPortOriginY(portNo) + (GetChStep() * chNo);
	vector.z = GetPortOriginZ(portNo);

	return vector;
}

//******************************************************************************
// ノート角度取得
//******************************************************************************
float MTNoteDesignRing::_GetNoteAngle(
		unsigned char noteNo,
		short pitchBendValue,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity	//省略可：ピッチベンド感度
	)
{
	float angle = 0.0f;
	float pb = 0.0f;

	//ピッチベンドによる角度の増減
	if (pitchBendValue < 0) {
		pb = m_NoteAngleStep * pitchBendSensitivity * ((float)pitchBendValue / 8192.0f);
	}
	else {
		pb = m_NoteAngleStep * pitchBendSensitivity * ((float)pitchBendValue / 8191.0f);
	}

	//ノート番号で角度を決定
	angle = (m_NoteAngleStep * noteNo) + (m_NoteAngleStep / 2.0f) + pb;

	return angle;
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTNoteDesignRing::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	MTConfFile confFile;

	//設定ファイル読み込み
	result = MTNoteDesign::_LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//----------------------------------
	//スケール情報
	//----------------------------------
	result = confFile.SetCurSection(_T("Scale"));
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("RingRadius"), &m_RingRadius, 5.0f);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


