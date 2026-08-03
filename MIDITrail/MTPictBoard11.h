//******************************************************************************
//
// MIDITrail / MTPictBoard11
//
// DX11 picture board (M4.5): a textured billboard quad standing in the playback
// section (behind the notes), following the now-line on X. Port of MTPictBoard.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "SMIDILib.h"
#include "DXPrimitive11.h"
#include "MTNoteDesign.h"

using namespace SMIDILib;


class MTPictBoard11
{
public:
	MTPictBoard11();
	virtual ~MTPictBoard11();

	int Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const TCHAR* pSceneName, SMSeqData* pSeqData);
	void Release();

	void SetCurTickTime(unsigned long curTickTime) { m_CurTickTime = curTickTime; }
	void Reset() { m_CurTickTime = 0; }

	int DrawDX11(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& viewProj,
			const DirectX::XMFLOAT4& lightDir, float rollAngle);

	bool IsReady() { return m_Ready; }

private:
	MTNoteDesign m_NoteDesign;
	DXPrimitive11 m_Prim;
	ID3D11ShaderResourceView* m_pSRV;
	DirectX::XMFLOAT3 m_WorldMove;
	unsigned long m_CurTickTime;
	bool m_Ready;

	int _LoadTexture(ID3D11Device* pDevice, const TCHAR* pSceneName, unsigned int* pW, unsigned int* pH);

	void operator=(const MTPictBoard11&);
	MTPictBoard11(const MTPictBoard11&);
};
