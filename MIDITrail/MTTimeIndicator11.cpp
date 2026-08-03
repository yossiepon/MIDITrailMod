//******************************************************************************
//
// MIDITrail / MTTimeIndicator11
//
// DX11 time indicator (M4.4) - port of MTTimeIndicator.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTTimeIndicator11.h"

using namespace YNBaseLib;
using namespace DirectX;


MTTimeIndicator11::MTTimeIndicator11()
{
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_CurTickTime = 0;
	m_EnableLine = false;
	m_Enable = true;
	m_Ready = false;
}

MTTimeIndicator11::~MTTimeIndicator11()
{
	Release();
}

void MTTimeIndicator11::Release()
{
	m_Prim.Release();
	m_PrimLine.Release();
	m_Ready = false;
}

//******************************************************************************
// Create: build the playback-section quad + the degenerate line
//******************************************************************************
int MTTimeIndicator11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	D3DXVECTOR3 vLU, vRU, vLD, vRD;
	D3DXVECTOR3 mv;
	DWORD color = 0;
	float delta = 0.0f;
	DXP11_VERTEX* pv = NULL;
	unsigned long* pi = NULL;

	Release();

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	mv = m_NoteDesign.GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);

	// playback section corners (port-area rectangle, local frame at the now-line)
	m_NoteDesign.GetPlaybackSectionVirtexPos(0, &vLU, &vRU, &vLD, &vRD);
	color = (DWORD)m_NoteDesign.GetPlaybackSectionColor();

	// the quad collapses to an invisible sliver when viewed head-on -> use a line
	delta = vLU.z - vRU.z;
	if (delta < 0) delta = -delta;
	m_EnableLine = (delta < 0.1f);

	//----------------------------------
	// quad (triangle list: LU,RU,LD + LD,RU,RD; strip 0,1,2,3)
	//----------------------------------
	result = m_Prim.CreateVertexBuffer(pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_Prim.CreateIndexBuffer(pDevice, 6);
	if (result != 0) goto EXIT;

	result = m_Prim.LockVertex(pContext, &pv);
	if (result != 0) goto EXIT;
	pv[0].pos[0]=vLU.x; pv[0].pos[1]=vLU.y; pv[0].pos[2]=vLU.z;
	pv[1].pos[0]=vRU.x; pv[1].pos[1]=vRU.y; pv[1].pos[2]=vRU.z;
	pv[2].pos[0]=vLD.x; pv[2].pos[1]=vLD.y; pv[2].pos[2]=vLD.z;
	pv[3].pos[0]=vRD.x; pv[3].pos[1]=vRD.y; pv[3].pos[2]=vRD.z;
	for (int k = 0; k < 4; k++) {
		pv[k].normal[0]=-1.0f; pv[k].normal[1]=0.0f; pv[k].normal[2]=0.0f;
		pv[k].color = color;
		pv[k].uv[0]=0.0f; pv[k].uv[1]=0.0f;
	}
	m_Prim.UnlockVertex(pContext);

	result = m_Prim.LockIndex(pContext, &pi);
	if (result != 0) goto EXIT;
	pi[0]=0; pi[1]=1; pi[2]=2; pi[3]=2; pi[4]=1; pi[5]=3;
	m_Prim.UnlockIndex(pContext);

	m_Prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);   // unlit, use vertex color

	//----------------------------------
	// line (LU -> LD)
	//----------------------------------
	result = m_PrimLine.CreateVertexBuffer(pDevice, 2);
	if (result != 0) goto EXIT;
	result = m_PrimLine.CreateIndexBuffer(pDevice, 2);
	if (result != 0) goto EXIT;

	result = m_PrimLine.LockVertex(pContext, &pv);
	if (result != 0) goto EXIT;
	pv[0].pos[0]=vLU.x; pv[0].pos[1]=vLU.y; pv[0].pos[2]=vLU.z;
	pv[1].pos[0]=vLD.x; pv[1].pos[1]=vLD.y; pv[1].pos[2]=vLD.z;
	for (int k = 0; k < 2; k++) {
		pv[k].normal[0]=0.0f; pv[k].normal[1]=0.0f; pv[k].normal[2]=-1.0f;
		pv[k].color = color;
		pv[k].uv[0]=0.0f; pv[k].uv[1]=0.0f;
	}
	m_PrimLine.UnlockVertex(pContext);

	result = m_PrimLine.LockIndex(pContext, &pi);
	if (result != 0) goto EXIT;
	pi[0]=0; pi[1]=1;
	m_PrimLine.UnlockIndex(pContext);

	m_PrimLine.SetLineTopology(true);
	m_PrimLine.SetMaterialAmbient(1.0f, 1.0f, 1.0f);

	m_Ready = true;

EXIT:;
	return result;
}

//******************************************************************************
// Draw: slides along X with the now-line, like the notes
//******************************************************************************
int MTTimeIndicator11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		const XMFLOAT4& lightDir,
		float rollAngle
	)
{
	if (!m_Ready || !m_Enable) return 0;

	float curPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);
	XMMATRIX world = XMMatrixRotationX(XMConvertToRadians(rollAngle))
	               * XMMatrixTranslation(m_WorldMove.x + curPos, m_WorldMove.y, m_WorldMove.z);

	if (m_EnableLine) {
		m_PrimLine.SetWorldMatrix(world);
		return m_PrimLine.Draw(pContext, viewProj, lightDir, -1, 0);
	}
	m_Prim.SetWorldMatrix(world);
	return m_Prim.Draw(pContext, viewProj, lightDir, -1, 0);
}
