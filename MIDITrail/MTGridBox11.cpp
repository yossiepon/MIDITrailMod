//******************************************************************************
//
// MIDITrail / MTGridBox11
//
// DX11 grid box (M3)
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTGridBox11.h"
#include <stdlib.h>

using namespace YNBaseLib;
using namespace DirectX;

// matches MTGridBox::MTGRIDBOX_VERTEX (XYZ|NORMAL|DIFFUSE, 28 bytes)
struct MTGB11_SRCVTX {
	float p[3];
	float n[3];
	unsigned long c;
};


MTGridBox11::MTGridBox11()
{
	m_Ready = false;
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

MTGridBox11::~MTGridBox11()
{
	Release();
}

void MTGridBox11::Release()
{
	m_Prim.Release();
	m_Ready = false;
}

//******************************************************************************
// Create: build the grid lines and upload them
//******************************************************************************
int MTGridBox11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long vn = 0, in = 0, i = 0;
	void* pCpuVB = NULL;
	unsigned long* pCpuIB = NULL;
	DXP11_VERTEX* pv = NULL;
	unsigned long* pi = NULL;
	MTGB11_SRCVTX* src = NULL;
	D3DXVECTOR3 mv;

	Release();

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_Geom.InitForDX11(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	mv = m_Geom.GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);

	m_Geom.GetGeometrySize(&vn, &in);
	if ((vn == 0) || (in == 0)) { result = 0; goto EXIT; }

	pCpuVB = malloc((size_t)vn * sizeof(MTGB11_SRCVTX));
	pCpuIB = (unsigned long*)malloc((size_t)in * sizeof(unsigned long));
	if ((pCpuVB == NULL) || (pCpuIB == NULL)) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }

	result = m_Geom.BuildGeometryCPU(pSeqData, pCpuVB, pCpuIB);
	if (result != 0) goto EXIT;

	result = m_Prim.CreateVertexBuffer(pDevice, vn);
	if (result != 0) goto EXIT;
	result = m_Prim.CreateIndexBuffer(pDevice, in);
	if (result != 0) goto EXIT;

	// convert 28-B grid verts -> 36-B DXP11_VERTEX (uv = 0)
	result = m_Prim.LockVertex(pContext, &pv);
	if (result != 0) goto EXIT;
	src = (MTGB11_SRCVTX*)pCpuVB;
	for (i = 0; i < vn; i++) {
		pv[i].pos[0] = src[i].p[0]; pv[i].pos[1] = src[i].p[1]; pv[i].pos[2] = src[i].p[2];
		pv[i].normal[0] = src[i].n[0]; pv[i].normal[1] = src[i].n[1]; pv[i].normal[2] = src[i].n[2];
		pv[i].color = src[i].c;
		pv[i].uv[0] = 0.0f; pv[i].uv[1] = 0.0f;
	}
	m_Prim.UnlockVertex(pContext);

	result = m_Prim.LockIndex(pContext, &pi);
	if (result != 0) goto EXIT;
	memcpy(pi, pCpuIB, (size_t)in * sizeof(unsigned long));
	m_Prim.UnlockIndex(pContext);

	m_Prim.SetLineTopology(true);
	m_Prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);   // lines are unlit (use vertex color)
	// ced 20260716: the grid is a backdrop reference drawn before the notes. It must not
	// write depth - otherwise a fully transparent grid (conf GridLineRGBA alpha 00) still
	// leaves depth behind and punches invisible gaps into the notes that cross it.
	m_Prim.SetDepthWrite(false);
	m_Ready = true;

EXIT:;
	if (pCpuVB != NULL) free(pCpuVB);
	if (pCpuIB != NULL) free(pCpuIB);
	return result;
}

//******************************************************************************
// Draw: static grid, world = RotX(roll) * Trans(worldMove)
//******************************************************************************
int MTGridBox11::DrawDX11(
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
