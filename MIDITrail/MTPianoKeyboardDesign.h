//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesign
//
// ピアノキーボードデザインクラス
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// キーボードの基本配置座標
//
//  +y   +z
//  |    /
//  |   / +-#-#-+-#-#-#-+------
//  |  / / # # / # # # / ...
//  | / / / / / / / / / ...
//  |/ +-+-+-+-+-+-+-+------
// 0+------------------------ +x

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// ピアノキーボードデザインクラス
//******************************************************************************
class MTPianoKeyboardDesign
{
public:

	//キー種別
	//  黒鍵は白鍵と白鍵の中心から微妙にずれて配置されている
	//  このため白鍵の形はCからBまですべて異なる
	enum KeyType {
		KeyWhiteC,	//白鍵C
		KeyWhiteD,	//白鍵D
		KeyWhiteE,	//白鍵E
		KeyWhiteF,	//白鍵F
		KeyWhiteG,	//白鍵G
		KeyWhiteA,	//白鍵A
		KeyWhiteB,	//白鍵B
		KeyBlack	//黒鍵
	};

	//発音中キー色種別
	enum ActiveKeyColorType {
		DefaultColor,	//デフォルト色
		NoteColor		//ノート色
	};

public:

	MTPianoKeyboardDesign(void);
	virtual ~MTPianoKeyboardDesign(void);

	//初期化
// >>> modify 20120728 yossiepon begin
	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);
// <<< modify 20120728 yossiepon end

	//ポート原点座標取得
	float GetPortOriginX(unsigned char portNo);
// >>> modify 20120728 yossiepon begin
	virtual float GetPortOriginY(unsigned char portNo);
	virtual float GetPortOriginZ(unsigned char portNo);
// <<< modify 20120728 yossiepon end

	//キー種別取得
	KeyType GetKeyType(unsigned char noteNo);

	//キー中心X座標取得
	float GetKeyCenterPosX(unsigned char noteNo);

	//白鍵配置間隔取得
	float GetWhiteKeyStep();

	//白鍵横サイズ取得
	float GetWhiteKeyWidth();

	//白鍵高さ取得
	float GetWhiteKeyHeight();

	//白鍵長さ取得
	float GetWhiteKeyLen();

	//黒鍵横サイズ取得
	float GetBlackKeyWidth();

	//黒鍵高さ取得
	float GetBlackKeyHeight();

	//黒鍵傾斜長さ取得
	float GetBlackKeySlopeLen();

	//黒鍵長さ取得
	float GetBlackKeyLen();

	//キー間隔サイズ取得
	float GetKeySpaceSize();

	//キー押下回転中心Y軸座標取得
	float GetKeyRotateAxisXPos();

	//キー押下回転角度
	float GetKeyRotateAngle();

	//キー下降時間取得(msec)
	unsigned long GetKeyDownDuration();

	//キー上昇時間取得(msec)
	unsigned long GetKeyUpDuration();

	//ピッチベンドキーボードシフト量取得
	float GetPitchBendShift(short pitchBendValue, unsigned char pitchBendSensitivity);

	//ノートドロップ座標取得
	float GetNoteDropPosZ(unsigned char noteNo);

	//白鍵カラー取得
	D3DXCOLOR GetWhiteKeyColor();

	//黒鍵カラー取得
	D3DXCOLOR GetBlackKeyColor();

	//発音中キーカラー取得
	D3DXCOLOR GetActiveKeyColor(
			unsigned char noteNo,
			unsigned long elapsedTime,
			D3DXCOLOR* pNoteColor = NULL
		);

	//白鍵テクスチャ座標取得
	void GetWhiteKeyTexturePosTop(
			unsigned char noteNo,
			D3DXVECTOR2* pTexPos0,
			D3DXVECTOR2* pTexPos1,
			D3DXVECTOR2* pTexPos2,
			D3DXVECTOR2* pTexPos3,
			D3DXVECTOR2* pTexPos4,
			D3DXVECTOR2* pTexPos5,
			D3DXVECTOR2* pTexPos6,
			D3DXVECTOR2* pTexPos7
		);
	void GetWhiteKeyTexturePosFront(
			unsigned char noteNo,
			D3DXVECTOR2* pTexPos0,
			D3DXVECTOR2* pTexPos1,
			D3DXVECTOR2* pTexPos2,
			D3DXVECTOR2* pTexPos3
		);
	void GetWhiteKeyTexturePosSingleColor(
			unsigned char noteNo,
			D3DXVECTOR2* pTexPos
		);

	//黒鍵テクスチャ座標取得
	void GetBlackKeyTexturePos(
			unsigned char noteNo,
			D3DXVECTOR2* pTexPos0,
			D3DXVECTOR2* pTexPos1,
			D3DXVECTOR2* pTexPos2,
			D3DXVECTOR2* pTexPos3,
			D3DXVECTOR2* pTexPos4,
			D3DXVECTOR2* pTexPos5,
			D3DXVECTOR2* pTexPos6,
			D3DXVECTOR2* pTexPos7,
			D3DXVECTOR2* pTexPos8,
			D3DXVECTOR2* pTexPos9,
			bool isColored = false
		);
	void GetBlackKeyTexturePosSingleColor(
			unsigned char noteNo,
			D3DXVECTOR2* pTexPos,
			bool isColored = false
		);

	//キーボード基準座標取得
// >>> modify 20120728 yossiepon begin
	virtual D3DXVECTOR3 GetKeyboardBasePos(unsigned char portNo, unsigned char chNo);
// <<< modify 20120728 yossiepon end

	//キーボード最大表示数取得
	unsigned long GetKeyboardMaxDispNum();

	//キー表示範囲取得
	unsigned char GetKeyDispRangeStart();
	unsigned char GetKeyDispRangeEnd();
	bool IsKeyDisp(unsigned char noteNo);

// >>> modify 20120728 yossiepon begin
protected:

	//ポート情報
	SMPortList m_PortList;
	unsigned char m_PortIndex[SM_MAX_PORT_NUM];

	//キーボード配置情報
	float m_KeyboardStepY;

	//発音中キー色情報
	int m_ActiveKeyColorDuration;
	float m_ActiveKeyColorTailRate;

	virtual void _Initialize();
	virtual int _LoadConfFile(const TCHAR* pSceneName);

// <<< modify 20120728 yossiepon end

// >>> modify 20140920 yossiepon begin
	ActiveKeyColorType m_ActiveKeyColorType;
// <<< modify 20120920 yossiepon end

private:

	//キー情報
	typedef struct {
		KeyType keyType;
		float keyCenterPosX;
	} MTKeyInfo;

private:

// >>> modify 20120728 yossiepon begin
	//キー情報配列
	MTKeyInfo m_KeyInfo[SM_MAX_NOTE_NUM];

// <<< modify 20120728 yossiepon end

	//スケール情報
	float m_WhiteKeyStep;
	float m_WhiteKeyWidth;
	float m_WhiteKeyHeight;
	float m_WhiteKeyLen;
	float m_BlackKeyWidth;
	float m_BlackKeyHeight;
	float m_BlackKeySlopeLen;
	float m_BlackKeyLen;
	float m_KeySpaceSize;
	float m_NoteDropPosZ4WhiteKey;
	float m_NoteDropPosZ4BlackKey;
	float m_BlackKeyShiftCDE;
	float m_BlackKeyShiftFGAB;

	//キー回転情報
	float m_KeyRotateAxisXPos;
	float m_KeyRotateAngle;
	int m_KeyDownDuration;
	int m_KeyUpDuration;

	//キーボード配置情報
// >>> modify 20120728 yossiepon begin
	float m_KeyboardStepZ;
	int m_KeyboardMaxDispNum;
// <<< modify 20120728 yossiepon end

	//キー色情報
	D3DXCOLOR m_WhiteKeyColor;
	D3DXCOLOR m_BlackKeyColor;

	//発音中キー色情報
// >>> modify 20120728 yossiepon begin
	D3DXCOLOR m_ActiveKeyColor;
// <<< modify 20120728 yossiepon end

// >>> modify 20120728 yossiepon begin
	//キー表示範囲
	int m_KeyDispRangeStart;
	int m_KeyDispRangeEnd;

	void _InitKeyType();
	void _InitKeyPos();
// <<< modify 20120728 yossiepon end

};


