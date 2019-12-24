//******************************************************************************
//
// MIDITrail / MTNoteDesignRing
//
// ノートデザインリングクラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
// ノートデザインクラス
//******************************************************************************
class MTNoteDesignRing : public MTNoteDesign
{
public:

	//コンストラクタ／デストラクタ
	MTNoteDesignRing(void);
	virtual ~MTNoteDesignRing(void);

	//ライブモニタモード設定
	void SetLiveMode(void);

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

	//グリッドリング基準座標取得
	void GetGridRingBasePos(
			unsigned long totalTickTime,
			D3DXVECTOR3* pBasePosStart,
			D3DXVECTOR3* pBasePosEnd
		);

	//ライブモニタ用グリッドリング基準座標取得
	void GetGridRingBasePosLive(
			D3DXVECTOR3* pBasePosStart,
			D3DXVECTOR3* pBasePosEnd
		);

	//ポート原点座標取得
	virtual float GetPortOriginY(unsigned char portNo);
	virtual float GetPortOriginZ(unsigned char portNo);

	//世界座標配置移動ベクトル取得
	virtual D3DXVECTOR3 GetWorldMoveVector();

// >>> modify access level to proteced 20191224 yossiepon begin
protected:
// <<< modify access level to proteced 20191224 yossiepon end

	bool m_isLiveMode;
	float m_NoteAngleStep;
	float m_RingRadius;

	// ノート基準座標取得
	D3DXVECTOR3 _GetNoteBasePos(
			unsigned long curTickTime,
			unsigned char portNo,
			unsigned char chNo
		);

	// ノート角度取得
	float _GetNoteAngle(
			unsigned char noteNo,
			short pitchBendValue,				//省略可：ピッチベンド
			unsigned char pitchBendSensitivity	//省略可：ピッチベンド感度
		);

	virtual int _LoadConfFile(const TCHAR* pSceneName);

};


