//******************************************************************************
//
// MIDITrail / MTPianoKeyboard11
//
// DX11 piano keyboard renderer (1ch).
// Generates 3D key geometry (white C/F, D/G/A, E/B + black), manages CPU
// mirror buffers, and flushes to GPU on key state changes.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXPrimitive11.h"
#include "MTPianoKeyboardDesign.h"
#include "SMIDILib.h"
#include <directxtk/SimpleMath.h>

using namespace SMIDILib;

struct KbdVertex;  // cpp で定義（DXPRIMITIVE11_VERTEX と同一メモリレイアウト）


//******************************************************************************
// Key state (passed from Ctrl to Keyboard per frame)
//******************************************************************************
struct MTKeyboardKeyState {
	float rate;            // 0.0 (up) .. 1.0 (fully pressed)
	unsigned long color;   // D3DCOLOR 0xAARRGGBB (valid when rate > 0)
};

//******************************************************************************
// DX11 piano keyboard renderer (1ch)
//******************************************************************************
class MTPianoKeyboard11
{
public:

	MTPianoKeyboard11();
	virtual ~MTPianoKeyboard11();

	virtual int Create(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext,
				const TCHAR* pSceneName,
				SMSeqData* pSeqData,
				ID3D11ShaderResourceView* pSRV
			);
	void Release();

	virtual int Update(
				ID3D11DeviceContext* pContext,
				const MTKeyboardKeyState keyStates[SM_MAX_NOTE_NUM],
				const DirectX::SimpleMath::Matrix& world
			);

	virtual int Draw(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				const DirectX::SimpleMath::Vector4& lightDir
			);

protected:

	virtual float _GetKeyRotateAngle();

private:

	DXPrimitive11 m_Prim;
	MTPianoKeyboardDesign m_KeyboardDesign;
	ID3D11ShaderResourceView* m_pSRV;

	DXPRIMITIVE11_VERTEX* m_pBaseVerts;
	DXPRIMITIVE11_VERTEX* m_pWorkVerts;
	unsigned long m_VertexNum;
	float m_PrevRate[SM_MAX_NOTE_NUM];
	unsigned long m_PrevColor[SM_MAX_NOTE_NUM];

	struct BufInfo {
		unsigned long vertexPos;
		unsigned long vertexNum;
		unsigned long indexPos;
		unsigned long indexNum;
	};
	BufInfo m_BufInfo[SM_MAX_NOTE_NUM];

	void _CreateBufInfo();
	int _CreateVertexOfKeyboard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	int _CreateVertexOfKey(
				unsigned char noteNo,
				DXPRIMITIVE11_VERTEX* pVertex,
				unsigned long* pIndex
			);
	int _CreateVertexOfKeyWhite1(
				unsigned char noteNo, KbdVertex* pVertex,
				unsigned long* pIndex, DirectX::SimpleMath::Color* pColor = NULL);
	int _CreateVertexOfKeyWhite2(
				unsigned char noteNo, KbdVertex* pVertex,
				unsigned long* pIndex, DirectX::SimpleMath::Color* pColor = NULL);
	int _CreateVertexOfKeyWhite3(
				unsigned char noteNo, KbdVertex* pVertex,
				unsigned long* pIndex, DirectX::SimpleMath::Color* pColor = NULL);
	int _CreateVertexOfKeyBlack(
				unsigned char noteNo, KbdVertex* pVertex,
				unsigned long* pIndex, DirectX::SimpleMath::Color* pColor = NULL);

	int _BuildKeyCPU(unsigned char noteNo, float rate,
				DirectX::SimpleMath::Color* pColor = NULL);
	int _ResetKeyCPU(unsigned char noteNo);
	int _FlushToGPU(ID3D11DeviceContext* pContext);
};
