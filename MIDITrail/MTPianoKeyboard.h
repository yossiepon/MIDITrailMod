//******************************************************************************
//
// MIDITrail / MTPianoKeyboard
//
// ピアノキーボード描画クラス
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ピアノキーボード(1ch分)の描画を制御するクラス。

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXPrimitive.h"
#include "SMIDILib.h"
#include "MTPianoKeyboardDesign.h"
#include "MTNotePitchBend.h"

using namespace SMIDILib;


//******************************************************************************
// ピアノキーボード描画クラス
//******************************************************************************
class MTPianoKeyboard
{
public:

	//コンストラクタ／デストラクタ
	MTPianoKeyboard(void);
	virtual ~MTPianoKeyboard(void);

	//生成
// >>> modify 20120728 yossiepon begin
	virtual int Create(
// <<< modify 20120728 yossiepon end
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			LPDIRECT3DTEXTURE9 pTexture = NULL
		);

	//更新
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 moveVector, float rollAngle);
// >>> add 20120729 yossiepon begin
// >>> modify 20180414 yossiepon begin
	virtual int Transform(
			LPDIRECT3DDEVICE9 pD3DDevice,
			D3DXVECTOR3 basePosVector,
			D3DXVECTOR3 playbackPosVector,
			D3DXVECTOR3 camVector,
			D3DXVECTOR3 lookVector,
			float rollAngle
		);
// <<< modify 20180414 yossiepon end
// <<< add 20120729 yossiepon end

// >>> modify function decl to virtual 20180412 yossiepon begin
	//描画
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
// <<< modify function decl to virtual 20180412 yossiepon end

	//解放
	void Release();

	//キー状態変更
	int ResetKey(unsigned char noteNo);
	int PushKey(
			unsigned char noteNo,
			float keyDownRate,
			unsigned long elapsedTime,
			D3DXCOLOR* pActiveKeyColor = NULL
		);
// >>> add 20120728 yossiepon begin
// >>> modify 20140920 yossiepon begin
	virtual int PushKey(
			unsigned char chNo,
			unsigned char noteNo,
			float keyDownRate,
			unsigned long elapsedTime,
			D3DXCOLOR* pActiveKeyColor = NULL
		);
// <<< modify 20140920 yossiepon end
// <<< add 20120728 yossiepon end

	//共有用テクスチャ取得
	LPDIRECT3DTEXTURE9 GetTexture();

// >>> modify access level to protected 20180412 yossiepon begin
protected:
// <<< modify 20180412 yossiepon end

	//頂点バッファ構造体
	struct MTPIANOKEYBOARD_VERTEX {
		D3DXVECTOR3 p;	//頂点座標
		D3DXVECTOR3 n;	//法線
		DWORD		c;	//ディフューズ色
		D3DXVECTOR2 t;	//テクスチャ画像位置
	};

	//バッファ情報
	typedef struct {
		unsigned long vertexPos;
		unsigned long vertexNum;
		unsigned long indexPos;
		unsigned long indexNum;
// >>> add 20180413 yossiepon begin
		unsigned long indexTotal;
		unsigned long revIndexPos;
		unsigned long revIndexTotal;
// <<< add 20180413 yossiepon end
	} MTBufInfo;

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

	//キーボードプリミティブ
	DXPrimitive m_PrimitiveKeyboard;

	//テクスチャ
	LPDIRECT3DTEXTURE9 m_pTexture;
	D3DXIMAGE_INFO m_ImgInfo;

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

	//キーボードデザイン
	MTPianoKeyboardDesign m_KeyboardDesign;

	//バッファ情報
	MTBufInfo m_BufInfo[SM_MAX_NOTE_NUM];

	//頂点バッファFVFフォーマット
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

	int _CreateKeyboard(LPDIRECT3DDEVICE9 pD3DDevice);
	void _CreateBufInfo();
	int _CreateVertexOfKeyboard(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateVertexOfKey(unsigned char noteNo);
// >>> modify function decl to virtual 20180412 yossiepon begin
	virtual int _CreateVertexOfKeyWhite1(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyWhite2(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyWhite3(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyBlack(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
// <<< modify function decl to virtual 20180412 yossiepon end

// >>> modify access level 20180412 yossiepon begin
private:
// <<< modify 20180412 yossiepon end

	int _LoadTexture(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);
	void _MakeMaterial(D3DMATERIAL9* pMaterial);

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

	int _RotateKey(unsigned char noteNo, float angle, D3DXCOLOR* pColor = NULL);

// >>> modify access level 20161224 yossiepon begin
private:
// <<< modify 20161224 yossiepon end

	D3DXVECTOR3 _RotateYZ(float centerY, float centerZ, D3DXVECTOR3 p1, float angle);

	int _HideKey(unsigned char noteNo);

};


