//******************************************************************************
//
// MIDITrail / MTTimeIndicatorRing11
//
// DX11 Ring-scene time indicator (M4.13) - port of MTTimeIndicatorRing.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTTimeIndicatorRing11.h"
#include "DXH.h"

using namespace YNBaseLib;
using namespace DirectX;

#define MTTIRING11_SEG_NUM  (128)


MTTimeIndicatorRing11::MTTimeIndicatorRing11()
{
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_CurTickTime = 0;
	m_Ready = false;
}

MTTimeIndicatorRing11::~MTTimeIndicatorRing11()
{
	Release();
}

void MTTimeIndicatorRing11::Release()
{
	m_Prim.Release();
	m_Ready = false;
}

//******************************************************************************
// Create: a closed circle (LINELIST) on the YZ plane, radius = ring radius
//******************************************************************************
int MTTimeIndicatorRing11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	D3DXVECTOR3 basePos, mv;
	DWORD color = 0;
	DXP11_VERTEX* pv = NULL;
	unsigned long* pi = NULL;
	unsigned long i = 0;

	Release();

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	mv = m_NoteDesign.GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);

	// base point on the ring (at time 0): (0, ringRadius, 0)
	basePos = D3DXVECTOR3(m_NoteDesign.GetPlayPosX(0),
						   m_NoteDesign.GetPortOriginY(0),
						   m_NoteDesign.GetPortOriginZ(0));
	color = (DWORD)m_NoteDesign.GetGridLineColor();

	result = m_Prim.CreateVertexBuffer(pDevice, MTTIRING11_SEG_NUM);
	if (result != 0) goto EXIT;
	result = m_Prim.CreateIndexBuffer(pDevice, MTTIRING11_SEG_NUM * 2);
	if (result != 0) goto EXIT;

	result = m_Prim.LockVertex(pContext, &pv);
	if (result != 0) goto EXIT;
	for (i = 0; i < MTTIRING11_SEG_NUM; i++) {
		D3DXVECTOR3 p = (i == 0) ? basePos
				: DXH::RotateYZ(0.0f, 0.0f, basePos, (360.0f / (float)MTTIRING11_SEG_NUM) * (float)i);
		pv[i].pos[0] = p.x; pv[i].pos[1] = p.y; pv[i].pos[2] = p.z;
		pv[i].normal[0] = -1.0f; pv[i].normal[1] = 0.0f; pv[i].normal[2] = 0.0f;
		pv[i].color = color;
		pv[i].uv[0] = 0.0f; pv[i].uv[1] = 0.0f;
	}
	m_Prim.UnlockVertex(pContext);

	result = m_Prim.LockIndex(pContext, &pi);
	if (result != 0) goto EXIT;
	for (i = 0; i < MTTIRING11_SEG_NUM; i++) {
		pi[i * 2]     = i;
		pi[i * 2 + 1] = (i + 1) % MTTIRING11_SEG_NUM;   // close the ring
	}
	m_Prim.UnlockIndex(pContext);

	m_Prim.SetLineTopology(true);
	m_Prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);
	m_Ready = true;

EXIT:;
	return result;
}

//******************************************************************************
// Draw: world = RotX(roll) * Trans(worldMove + (playPosX,0,0))
//******************************************************************************
int MTTimeIndicatorRing11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		const XMFLOAT4& lightDir,
		float rollAngle
	)
{
	if (!m_Ready) return 0;

	float curPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);
	XMMATRIX world = XMMatrixRotationX(XMConvertToRadians(rollAngle))
	               * XMMatrixTranslation(m_WorldMove.x + curPos, m_WorldMove.y, m_WorldMove.z);
	m_Prim.SetWorldMatrix(world);
	return m_Prim.Draw(pContext, viewProj, lightDir, -1, 0);
}
