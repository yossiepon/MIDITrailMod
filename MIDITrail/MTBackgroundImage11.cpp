//******************************************************************************
//
// MIDITrail / MTBackgroundImage11
//
// DX11 background image (M4.15) - port of MTBackgroundImage.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTBackgroundImage11.h"
#include "DXTexture11.h"

using namespace YNBaseLib;
using namespace DirectX;


MTBackgroundImage11::MTBackgroundImage11()
{
	m_pSRV = NULL;
	m_ImgW = 0;
	m_ImgH = 0;
	m_Ready = false;
}

MTBackgroundImage11::~MTBackgroundImage11()
{
	Release();
}

void MTBackgroundImage11::Release()
{
	if (m_pSRV != NULL) { m_pSRV->Release(); m_pSRV = NULL; }
	m_Prim.Release();
	m_ImgW = 0;
	m_ImgH = 0;
	m_Ready = false;
}

//******************************************************************************
// Create: load the image (off if path empty or load fails)
//******************************************************************************
int MTBackgroundImage11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pImgFilePath
	)
{
	int result = 0;
	unsigned long* pi = NULL;

	Release();

	if ((pImgFilePath == NULL) || (pImgFilePath[0] == _T('\0'))) return 0;   // disabled

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	if (DXTexture11::LoadFromFile(pDevice, pImgFilePath, &m_pSRV, &m_ImgW, &m_ImgH) != 0) {
		return 0;   // load failed -> silently disabled (matches DX9)
	}
	if ((m_ImgW == 0) || (m_ImgH == 0)) return 0;

	result = m_Prim.CreateVertexBuffer(pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_Prim.CreateIndexBuffer(pDevice, 6);
	if (result != 0) goto EXIT;
	result = m_Prim.LockIndex(pContext, &pi);
	if (result != 0) goto EXIT;
	pi[0]=0; pi[1]=1; pi[2]=2; pi[3]=2; pi[4]=1; pi[5]=3;
	m_Prim.UnlockIndex(pContext);

	m_Prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);
	m_Prim.SetTexture(m_pSRV);
	m_Ready = true;

EXIT:;
	return result;
}

//******************************************************************************
// Draw: letterboxed full-window quad (screen space, z at the far plane)
//******************************************************************************
int MTBackgroundImage11::DrawDX11(ID3D11DeviceContext* pContext, unsigned int screenW, unsigned int screenH)
{
	DXP11_VERTEX* pv = NULL;
	XMMATRIX ident = XMMatrixIdentity();
	XMFLOAT4 light(0.0f, 0.0f, 1.0f, 0.0f);

	if (!m_Ready || (screenW == 0) || (screenH == 0)) return 0;

	// aspect-preserving letterbox: fit the image inside the window
	float cw = (float)screenW, ch = (float)screenH;
	float ratioWin = cw / ch;
	float ratioImg = (float)m_ImgW / (float)m_ImgH;
	float x0p, y0p, x1p, y1p;   // pixel rect
	if (ratioWin < ratioImg) {  // image wider than window -> bars top/bottom
		x0p = 0.0f; x1p = cw;
		float h = cw / ratioImg;
		y0p = (ch - h) / 2.0f; y1p = ch - y0p;
	} else {                    // image taller -> bars left/right
		y0p = 0.0f; y1p = ch;
		float w = ch * ratioImg;
		x0p = (cw - w) / 2.0f; x1p = cw - x0p;
	}

	// pixels -> NDC; z at the far plane so the scene draws over it
	float x0 = -1.0f + 2.0f * x0p / cw;
	float x1 = -1.0f + 2.0f * x1p / cw;
	float y0 =  1.0f - 2.0f * y0p / ch;
	float y1 =  1.0f - 2.0f * y1p / ch;
	const float Z = 0.9999f;

	if (m_Prim.LockVertex(pContext, &pv) != 0) return 0;
	pv[0].pos[0]=x0; pv[0].pos[1]=y0; pv[0].pos[2]=Z; pv[0].uv[0]=0; pv[0].uv[1]=0;
	pv[1].pos[0]=x1; pv[1].pos[1]=y0; pv[1].pos[2]=Z; pv[1].uv[0]=1; pv[1].uv[1]=0;
	pv[2].pos[0]=x0; pv[2].pos[1]=y1; pv[2].pos[2]=Z; pv[2].uv[0]=0; pv[2].uv[1]=1;
	pv[3].pos[0]=x1; pv[3].pos[1]=y1; pv[3].pos[2]=Z; pv[3].uv[0]=1; pv[3].uv[1]=1;
	for (int k = 0; k < 4; k++) { pv[k].normal[0]=0; pv[k].normal[1]=0; pv[k].normal[2]=-1; pv[k].color=0xFFFFFFFF; }
	m_Prim.UnlockVertex(pContext);

	m_Prim.SetWorldMatrix(ident);
	m_Prim.SetTexture(m_pSRV);
	return m_Prim.Draw(pContext, ident, light, 2, 0);
}
