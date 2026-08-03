//******************************************************************************
//
// MIDITrail / MTGridRing11
//
// DX11 Ring-scene grid (M4.13) - port of MTGridRing.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTGridRing11.h"
#include "DXH.h"

using namespace YNBaseLib;
using namespace DirectX;

#define MTGRING11_SEG_NUM  (128)


MTGridRing11::MTGridRing11()
{
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_Ready = false;
}

MTGridRing11::~MTGridRing11()
{
	Release();
}

void MTGridRing11::Release()
{
	m_Prim.Release();
	m_Ready = false;
}

//******************************************************************************
// Create: two concentric rings (inner at t=0, outer at song end)
//******************************************************************************
int MTGridRing11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	D3DXVECTOR3 basePosStart, basePosEnd, mv;
	DWORD color = 0;
	unsigned long totalTickTime = 0;
	DXP11_VERTEX* pv = NULL;
	unsigned long* pi = NULL;
	unsigned long i = 0;
	const D3DXVECTOR3* bases[2];

	Release();

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	mv = m_NoteDesign.GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);

	totalTickTime = pSeqData->GetTotalTickTime();
	m_NoteDesign.GetGridRingBasePos(totalTickTime, &basePosStart, &basePosEnd);
	color = (DWORD)m_NoteDesign.GetGridLineColor();
	bases[0] = &basePosStart;
	bases[1] = &basePosEnd;

	result = m_Prim.CreateVertexBuffer(pDevice, MTGRING11_SEG_NUM * 2);
	if (result != 0) goto EXIT;
	result = m_Prim.CreateIndexBuffer(pDevice, MTGRING11_SEG_NUM * 2 * 2);
	if (result != 0) goto EXIT;

	result = m_Prim.LockVertex(pContext, &pv);
	if (result != 0) goto EXIT;
	for (int r = 0; r < 2; r++) {
		const D3DXVECTOR3& base = *bases[r];
		for (i = 0; i < MTGRING11_SEG_NUM; i++) {
			unsigned long v = r * MTGRING11_SEG_NUM + i;
			D3DXVECTOR3 p = (i == 0) ? base
					: DXH::RotateYZ(0.0f, 0.0f, base, (360.0f / (float)MTGRING11_SEG_NUM) * (float)i);
			pv[v].pos[0] = p.x; pv[v].pos[1] = p.y; pv[v].pos[2] = p.z;
			pv[v].normal[0] = -1.0f; pv[v].normal[1] = 0.0f; pv[v].normal[2] = 0.0f;
			pv[v].color = color;
			pv[v].uv[0] = 0.0f; pv[v].uv[1] = 0.0f;
		}
	}
	m_Prim.UnlockVertex(pContext);

	result = m_Prim.LockIndex(pContext, &pi);
	if (result != 0) goto EXIT;
	for (int r = 0; r < 2; r++) {
		unsigned long ofs = r * MTGRING11_SEG_NUM;
		for (i = 0; i < MTGRING11_SEG_NUM; i++) {
			unsigned long base = (ofs + i) * 2;
			pi[base]     = ofs + i;
			pi[base + 1] = ofs + ((i + 1) % MTGRING11_SEG_NUM);   // close each ring
		}
	}
	m_Prim.UnlockIndex(pContext);

	m_Prim.SetLineTopology(true);
	m_Prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);
	m_Ready = true;

EXIT:;
	return result;
}

//******************************************************************************
// Draw: static rings, world = RotX(roll) * Trans(worldMove)
//******************************************************************************
int MTGridRing11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		const XMFLOAT4& lightDir,
		float rollAngle
	)
{
	if (!m_Ready) return 0;

	XMMATRIX world = XMMatrixRotationX(XMConvertToRadians(rollAngle))
	               * XMMatrixTranslation(m_WorldMove.x, m_WorldMove.y, m_WorldMove.z);
	m_Prim.SetWorldMatrix(world);
	return m_Prim.Draw(pContext, viewProj, lightDir, -1, 0);
}
