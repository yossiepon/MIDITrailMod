//******************************************************************************
//
// MIDITrail / MTStars11
//
// DX11 starfield - port of MTStars.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTStars11.h"
#include <stdlib.h>
#include <math.h>

using namespace YNBaseLib;
using namespace DirectX;


MTStars11::MTStars11()
{
	m_NumStars = 0;
	m_Ready = false;
}

MTStars11::~MTStars11()
{
	Release();
}

void MTStars11::Release()
{
	m_Prim.Release();
	m_NumStars = 0;
	m_Ready = false;
}

//******************************************************************************
// Create: build the point cloud (off if numStars <= 0)
//******************************************************************************
int MTStars11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		int numStars
	)
{
	int result = 0;
	DXP11_VERTEX* pv = NULL;
	unsigned long* pi = NULL;

	Release();

	if ((pDevice == NULL) || (numStars <= 0)) return 0;   // disabled
	m_NumStars = numStars;

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_Prim.CreateVertexBuffer(pDevice, m_NumStars);
	if (result != 0) goto EXIT;
	result = m_Prim.CreateIndexBuffer(pDevice, m_NumStars);
	if (result != 0) goto EXIT;

	// identity index buffer (one index per point)
	result = m_Prim.LockIndex(pContext, &pi);
	if (result != 0) goto EXIT;
	for (int i = 0; i < m_NumStars; i++) pi[i] = (unsigned long)i;
	m_Prim.UnlockIndex(pContext);

	// vertices: uniform distribution on a sphere of radius 500, grayscale color.
	// The normal is set opposite the scene light direction (0.3,-0.6,0.5) so the
	// directional lighting in the shared shader leaves the star colors unchanged.
	result = m_Prim.LockVertex(pContext, &pv);
	if (result != 0) goto EXIT;
	{
		XMVECTOR nrm = XMVector3Normalize(XMVectorSet(-0.3f, 0.6f, -0.5f, 0.0f));
		XMFLOAT3 n;
		XMStoreFloat3(&n, nrm);
		const float r = 500.0f;
		for (int i = 0; i < m_NumStars; i++) {
			float phi = ((float)rand() / RAND_MAX) * 2.0f * 3.1415926f;
			float y   = ((float)rand() / RAND_MAX) * 2.0f * r - r;
			float x   = sqrtf((r * r) - (y * y)) * cosf(phi);
			float z   = sqrtf((r * r) - (y * y)) * sinf(phi);
			unsigned long g = (unsigned long)(((float)rand() / RAND_MAX) * 255.0f);
			if (g > 255) g = 255;
			pv[i].pos[0] = x; pv[i].pos[1] = y; pv[i].pos[2] = z;
			pv[i].normal[0] = n.x; pv[i].normal[1] = n.y; pv[i].normal[2] = n.z;
			pv[i].color = 0xFF000000 | (g << 16) | (g << 8) | g;
			pv[i].uv[0] = 0.0f; pv[i].uv[1] = 0.0f;
		}
	}
	m_Prim.UnlockVertex(pContext);

	m_Prim.SetMaterialAmbient(0.5f, 0.5f, 0.5f);
	m_Prim.SetTexture(NULL);
	m_Prim.SetPointTopology(true);
	m_Ready = true;

EXIT:;
	return result;
}

//******************************************************************************
// Draw: translate the point cloud to the camera position (sky follows camera)
//******************************************************************************
int MTStars11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		const XMFLOAT3& camPos
	)
{
	XMFLOAT4 light(0.3f, -0.6f, 0.5f, 0.0f);

	if (!m_Ready) return 0;

	m_Prim.SetWorldMatrix(XMMatrixTranslation(camPos.x, camPos.y, camPos.z));
	return m_Prim.Draw(pContext, viewProj, light);
}
