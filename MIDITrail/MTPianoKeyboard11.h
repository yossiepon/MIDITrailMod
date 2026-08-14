//******************************************************************************
//
// MIDITrail / MTPianoKeyboard11
//
// Piano keyboard renderer base class (single channel, Rain/Roll shared).
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2016-2026 Yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#pragma once

#include "DXPrimitive11.h"
#include "MTPianoKeyboardDesign11.h"
#include "SMIDILib.h"
#include <directxtk/SimpleMath.h>

using namespace SMIDILib;


//******************************************************************************
// KbdVertex (DX9-compat readable vertex; same memory layout as DXPRIMITIVE11_VERTEX)
//******************************************************************************
struct KbdVertex {
	DirectX::SimpleMath::Vector3 p;
	DirectX::SimpleMath::Vector3 n;
	unsigned long c;
	DirectX::SimpleMath::Vector2 t;
};
static_assert(sizeof(KbdVertex) == sizeof(DXPRIMITIVE11_VERTEX), "KbdVertex layout mismatch");


//******************************************************************************
// Key state (passed from Ctrl to Keyboard per frame)
//******************************************************************************
struct MTKeyboardKeyState {
	float rate;            // 0.0 (up) .. 1.0 (fully pressed)
	unsigned long color;   // D3DCOLOR 0xAARRGGBB (valid when rate > 0)
	unsigned char chNo;    // MIDI channel that contributed the max rate
};

//******************************************************************************
// DX11 piano keyboard base class (1ch)
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

	int Update(
				ID3D11DeviceContext* pContext,
				const MTKeyboardKeyState keyStates[SM_MAX_NOTE_NUM],
				const DirectX::SimpleMath::Matrix& world
			);

	int Draw(
				ID3D11DeviceContext* pContext,
				const DirectX::SimpleMath::Matrix& viewProj,
				const DirectX::SimpleMath::Vector4& lightDir
			);

protected:

	// -- Pure virtual hooks (derived classes must implement) --

	virtual int _CreateVertexOfKeyboard(
				ID3D11Device* pDevice,
				ID3D11DeviceContext* pContext
			) = 0;

	virtual int _BuildKeyCPU(unsigned char noteNo, float rate,
				DirectX::SimpleMath::Color* pColor = NULL) = 0;

	// -- Virtual with default implementation --

	virtual int _ResetKeyCPU(unsigned char noteNo);

	// -- Infrastructure (accessible to derived) --

	DXPrimitive11 m_Prim;
	DXPRIMITIVE11_VERTEX* m_pBaseVerts;
	DXPRIMITIVE11_VERTEX* m_pWorkVerts;
	unsigned long m_VertexNum;

	struct BufInfo {
		unsigned long vertexPos;
		unsigned long vertexNum;
		unsigned long indexPos;
		unsigned long indexNum;
	};
	BufInfo m_BufInfo[SM_MAX_NOTE_NUM];

	MTPianoKeyboardDesign11* m_pKeyboardDesign;

	void _CreateBufInfo();
	int _FlushToGPU(ID3D11DeviceContext* pContext);

	// -- Linear key vertex generation (shared by Rain/Roll) --
	// Implemented in MTPianoKeyboard11_vertex.cpp.
	// Generates key geometry in Rain coordinate system (X=pitch, Y=height, Z=depth).
	// Uses m_pKeyboardDesign for key positions and dimensions.

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

private:

	float m_PrevRate[SM_MAX_NOTE_NUM];
	unsigned long m_PrevColor[SM_MAX_NOTE_NUM];
	ID3D11ShaderResourceView* m_pSRV;
};
