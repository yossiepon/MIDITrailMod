//******************************************************************************
//
// MIDITrail / MTNoteDesignRingMod
//
// ノートデザインリングModクラス
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteDesignMod.h"
#include "MTNoteDesignRing.h"

using namespace SMIDILib;


//******************************************************************************
// ノートデザインリングModクラス
//******************************************************************************
class MTNoteDesignRingMod : public MTNoteDesignMod, public MTNoteDesignRing
{
public:

	//コンストラクタ／デストラクタ
	MTNoteDesignRingMod(void);
	virtual ~MTNoteDesignRingMod(void);

	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	//ノートボックス中心座標取得
	virtual D3DXVECTOR3 GetNoteBoxCenterPosX(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		short pitchBendValue = 0,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity = 0	//省略可：ピッチベンド感度
	);

	//ノートボックス頂点座標取得
	virtual void GetNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
		D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
		D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
		D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
		short pitchBendValue = 0,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity = 0	//省略可：ピッチベンド感度
	);

	//発音中ノートボックス頂点座標取得
	virtual void GetActiveNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
		D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
		D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
		D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
		short pitchBendValue = 0,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity = 0,	//省略可：ピッチベンド感度
		unsigned long elapsedTime = 0            //省略可：経過時間（ミリ秒）
	);

	//発音中ノートボックス頂点座標取得
	virtual void GetActiveNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
		D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
		D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
		D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
		short pitchBendValue = 0,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity = 0,	//省略可：ピッチベンド感度
		float rate = 0.0f						//省略可：サイズ比率
	);

	//ライブモニタ用ノートボックス頂点座標取得
	virtual void GetNoteBoxVirtexPosLive(
		unsigned long elapsedTime,	//経過時間（ミリ秒）
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
		D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
		D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
		D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
		short pitchBendValue = 0,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity = 0	//省略可：ピッチベンド感度
	);

	//ポート原点座標取得
	virtual float GetPortOriginY(unsigned char portNo);
	virtual float GetPortOriginZ(unsigned char portNo);

	//世界座標配置移動ベクトル取得
	virtual D3DXVECTOR3 GetWorldMoveVector();

private:

	virtual int _LoadConfFile(const TCHAR* pSceneName);
};


