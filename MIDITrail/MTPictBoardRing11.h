//******************************************************************************
//
// MIDITrail / MTPictBoardRing11
//
// DX11 Ring-scene picture board (M4.13): the board texture wrapped around the
// ring as a cylindrical band (128 segments), following the now-line on X. Port
// of MTPictBoardRing. (Shown in the Ring scene, unlike the box Mod scenes.)
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "SMIDILib.h"
#include "DXPrimitive11.h"
#include "MTNoteDesignRing.h"

using namespace SMIDILib;


class MTPictBoardRing11
{
public:
	MTPictBoardRing11();
	virtual ~MTPictBoardRing11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	void SetCurTickTime(unsigned long curTickTime) { m_CurTickTime = curTickTime; }
	void Reset() { m_CurTickTime = 0; }

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle);

	bool IsReady() { return m_Ready; }

private:
	MTNoteDesignRing m_NoteDesign;
	DXPrimitive11 m_Prim;
	ID3D11ShaderResourceView* m_pSRV;
	DirectX::XMFLOAT3 m_WorldMove;
	unsigned long m_CurTickTime;
	bool m_Ready;

	int _LoadTexture(ID3D11Device* pDevice, const TCHAR* pSceneName, unsigned int* pW, unsigned int* pH);

	void operator=(const MTPictBoardRing11&);
	MTPictBoardRing11(const MTPictBoardRing11&);
};
