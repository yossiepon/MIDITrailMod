//******************************************************************************
//
// MIDITrail / MTNoteDesign
//
// ノートデザインクラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ノートボックスの正面を0-1-2-3の四角とする。
// この四角の中心座標が基準点であり、ポート／チャンネル／ノートの番号
// によって決定される。
//
//           +--+
//          /  /|
//         /  / +
//        /  / /      +x
//       /  / /      /
//     0+--+1/ +y   /
//      |  |/   |  /
//     2+--+3   | /
//              |/
//   +z---------+0
//

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// ノートデザインクラス
//******************************************************************************
class MTNoteDesign
{
public:

	//コンストラクタ／デストラクタ
	MTNoteDesign(void);
	virtual ~MTNoteDesign(void);

	//初期化
// >>> modify 20161225 yossiepon begin
	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);
// <<< modify 20161225 yossiepon end

	//演奏位置取得
	float GetPlayPosX(unsigned long curTickTime);

	//ライブモニタ用ノート位置取得
	float GetLivePosX(unsigned long elapsedTime);

	//ノートボックス中心座標取得
	virtual D3DXVECTOR3 GetNoteBoxCenterPosX(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				short pitchBendValue = 0,				//省略可：ピッチベンド
				unsigned char pitchBendSensitivity = 0	//省略可：ピッチベンド感度
			);

	//ノートボックス縦横サイズ取得
	float GetNoteBoxHeight();
	float GetNoteBoxWidth();

	//ノート間隔取得
	float GetNoteStep();

	//チャンネル間隔取得
	float GetChStep();

	//ライブモニタ表示期限
	unsigned long GetLiveMonitorDisplayDuration();

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

	//グリッドボックス頂点座標取得
	void GetGridBoxVirtexPos(
				unsigned long curTickTime,
				unsigned char portNo,
				D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
				D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
				D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
				D3DXVECTOR3* pVector3 	//YZ平面+X軸方向を見て右下
			);

	//ライブモニタ用グリッドボックス頂点座標取得
	void GetGridBoxVirtexPosLive(
				unsigned long elapsedTime,	//経過時間（ミリ秒）
				unsigned char portNo,	//ポート番号
				D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
				D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
				D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
				D3DXVECTOR3* pVector3 	//YZ平面+X軸方向を見て右下
			);

	//再生面頂点座標取得
	void GetPlaybackSectionVirtexPos(
				unsigned long curTickTime,
				D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
				D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
				D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
				D3DXVECTOR3* pVector3 	//YZ平面+X軸方向を見て右下
			);

	//波紋サイズ取得：経過時間（ミリ秒）は省略可
	float GetRippleHeight(unsigned long elapsedTime = 0);
	float GetRippleWidth(unsigned long elapsedTime = 0);
	float GetRippleAlpha(unsigned long elapsedTime = 0);

	//ピクチャボード相対位置取得
	float GetPictBoardRelativePos();

	//ポート原点座標取得
	virtual float GetPortOriginY(unsigned char portNo);
	virtual float GetPortOriginZ(unsigned char portNo);

	//世界座標配置移動ベクトル取得
	virtual D3DXVECTOR3 GetWorldMoveVector();

	//ノートボックスカラー取得
	D3DXCOLOR GetNoteBoxColor(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo
			);

	//発音中ノートボックスカラー取得
	D3DXCOLOR GetActiveNoteBoxColor(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				unsigned long elapsedTime
			);

	//発音中ノートボックスエミッシブ取得（マテリアル用）
	D3DXCOLOR GetActiveNoteEmissive();

	//グリッドラインカラー取得
	D3DXCOLOR GetGridLineColor();

	//再生面カラー取得
	D3DXCOLOR GetPlaybackSectionColor();

protected:

	enum NoteColorType {
		Channel,
		Scale
	};

	unsigned long m_TimeDivision;
	float m_QuarterNoteLength;
	float m_NoteBoxHeight;
	float m_NoteBoxWidth;
	float m_NoteStep;
	float m_ChStep;
	float m_RippleHeight;
	float m_RippleWidth;
	float m_PictBoardRelativePos;
	SMPortList m_PortList;
	unsigned char m_PortIndex[256];

	NoteColorType m_NoteColorType;
	D3DXCOLOR m_NoteColor[16];
	D3DXCOLOR m_NoteColorOfScale[12];
	D3DXCOLOR m_ActiveNoteEmissive;
	D3DXCOLOR m_GridLineColor;
	D3DXCOLOR m_PlaybackSectionColor;

	int m_ActiveNoteDuration;
	float m_ActiveNoteWhiteRate;
	float m_ActiveNoteBoxSizeRatio;

	int m_RippleDuration;

	int m_LiveMonitorDisplayDuration;
	float m_LiveNoteLengthPerSecond;

// >>> modify 20120728 yossiepon begin
	virtual void _Clear();
// <<< modify 20120728 yossiepon end

	virtual int _LoadConfFile(const TCHAR* pSceneName);
	int _LoadUserConf();

};


