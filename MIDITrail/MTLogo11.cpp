//******************************************************************************
//
// MIDITrail / MTLogo11
//
// DX11 startup / title logo (port of MTLogo + MTSceneTitle).
//
//******************************************************************************

#include "stdafx.h"
#include <mmsystem.h>
#include "YNBaseLib.h"
#include "MTLogo11.h"

using namespace YNBaseLib;
using namespace DirectX;

// title / layout (mirrors MTLogo)
#define MTLOGO11_TITLE          _T("MIDITrail")
#define MTLOGO11_FONT_NAME      _T("Arial")
#define MTLOGO11_FONT_SIZE      (40)
#define MTLOGO11_FONT_RGB       (0x00FFFFFF)   // white; alpha carries the glyph coverage
#define MTLOGO11_POS_X          (20.0f)
#define MTLOGO11_POS_Y          (-15.0f)
#define MTLOGO11_MAG            (0.1f)
#define MTLOGO11_TILE_NUM       (40)
#define MTLOGO11_GRADATION_TIME (1000)         // msec

// camera (mirrors MTSceneTitle)
#define MTLOGO11_CAMERA_POSZ        (-80.0f)
#define MTLOGO11_CAMERA_POSZ_DELTA  (0.05f)
#define MTLOGO11_VIEW_ANGLE         (45.0f)
#define MTLOGO11_NEAR_PLANE         (1.0f)
#define MTLOGO11_FAR_PLANE          (1000.0f)

#define MTLOGO11_VERTEX_NUM  (6 * MTLOGO11_TILE_NUM)


//******************************************************************************
// Constructor / destructor
//******************************************************************************
MTLogo11::MTLogo11()
{
	m_pDevice = NULL;
	m_pSRV = NULL;
	m_TexW = 0;
	m_TexH = 0;
	m_pVtx = NULL;
	m_CamPosZ = MTLOGO11_CAMERA_POSZ;
	m_StartTime = 0;
	m_Ready = false;
}

MTLogo11::~MTLogo11()
{
	Release();
}

//******************************************************************************
// Release
//******************************************************************************
void MTLogo11::Release()
{
	if (m_pSRV != NULL) { m_pSRV->Release(); m_pSRV = NULL; }
	m_Prim.Release();
	m_Font.Clear();
	delete [] m_pVtx;
	m_pVtx = NULL;
	m_TexW = 0;
	m_TexH = 0;
	m_CamPosZ = MTLOGO11_CAMERA_POSZ;
	m_StartTime = 0;
	m_Ready = false;
}

//******************************************************************************
// Create
//******************************************************************************
int MTLogo11::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	int result = 0;
	unsigned long* pi = NULL;

	Release();
	m_pDevice = pDevice;

	if ((pDevice == NULL) || (pContext == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	// title text texture (white glyphs, alpha = coverage)
	result = _CreateTexture();
	if (result != 0) goto EXIT;

	// geometry: TILE_NUM quads (triangle list, 6 verts each), identity index
	result = m_Prim.CreateVertexBuffer(pDevice, MTLOGO11_VERTEX_NUM);
	if (result != 0) goto EXIT;
	result = m_Prim.CreateIndexBuffer(pDevice, MTLOGO11_VERTEX_NUM);
	if (result != 0) goto EXIT;
	result = m_Prim.LockIndex(pContext, &pi);
	if (result != 0) goto EXIT;
	for (unsigned long i = 0; i < MTLOGO11_VERTEX_NUM; i++) pi[i] = i;
	m_Prim.UnlockIndex(pContext);

	m_Prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);   // full ambient: keep the vertex color as-is

	result = _CreateVertex(pContext);
	if (result != 0) goto EXIT;

	m_Ready = true;

EXIT:;
	return result;
}

//******************************************************************************
// Title text texture (GDI font bitmap -> D3D11 texture; white RGB + alpha)
//******************************************************************************
int MTLogo11::_CreateTexture()
{
	int result = 0;
	unsigned long h = 0, w = 0, x = 0, y = 0;
	DWORD* pBuf = NULL;
	ID3D11Texture2D* pTex = NULL;
	HRESULT hr = S_OK;

	result = m_Font.SetFont(MTLOGO11_FONT_NAME, MTLOGO11_FONT_SIZE, false);
	if (result != 0) goto EXIT;

	result = m_Font.CreateBmp(MTLOGO11_TITLE);
	if (result != 0) goto EXIT;

	m_Font.GetBmpSize(&h, &w);
	if ((w == 0) || (h == 0)) { result = YN_SET_ERR("Program error.", 0, 0); goto EXIT; }

	pBuf = (DWORD*)malloc((size_t)w * h * sizeof(DWORD));
	if (pBuf == NULL) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			BYTE p = m_Font.GetBmpPixcel(x, y);
			DWORD a = (0xFF * p) / 16;
			pBuf[y * w + x] = (p == 0) ? 0 : ((a << 24) | (MTLOGO11_FONT_RGB & 0x00FFFFFF));
		}
	}

	{
		D3D11_TEXTURE2D_DESC td;
		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&td, sizeof(td));
		td.Width = w; td.Height = h; td.MipLevels = 1; td.ArraySize = 1;
		td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.Usage = D3D11_USAGE_IMMUTABLE;
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = pBuf;
		sd.SysMemPitch = w * sizeof(DWORD);
		hr = m_pDevice->CreateTexture2D(&td, &sd, &pTex);
		if (SUCCEEDED(hr)) {
			hr = m_pDevice->CreateShaderResourceView(pTex, NULL, &m_pSRV);
			pTex->Release();
		}
		if (FAILED(hr)) { result = YN_SET_ERR("DirectX API error.", hr, 0); goto EXIT; }
	}

	m_TexW = w;
	m_TexH = h;

EXIT:;
	if (pBuf != NULL) free(pBuf);
	return result;
}

//******************************************************************************
// Build the tile strip (positions + UV fixed; colors filled later)
//******************************************************************************
int MTLogo11::_CreateVertex(ID3D11DeviceContext* pContext)
{
	int result = 0;
	unsigned long i = 0, k = 0;
	float tileW = ((float)m_TexW / (float)MTLOGO11_TILE_NUM) * MTLOGO11_MAG;
	float tileH = (float)m_TexH * MTLOGO11_MAG;

	try {
		m_pVtx = new DXP11_VERTEX[MTLOGO11_VERTEX_NUM];
	}
	catch (std::bad_alloc) {
		return YN_SET_ERR("Could not allocate memory.", 0, 0);
	}
	ZeroMemory(m_pVtx, sizeof(DXP11_VERTEX) * MTLOGO11_VERTEX_NUM);

	for (i = 0; i < MTLOGO11_TILE_NUM; i++) {
		DXP11_VERTEX* v = m_pVtx + (i * 6);
		float x0 = tileW * (float)i;
		float x1 = tileW * (float)(i + 1);
		float u0 = (float)i / (float)MTLOGO11_TILE_NUM;
		float u1 = (float)(i + 1) / (float)MTLOGO11_TILE_NUM;

		// positions: XY plane, top edge at y=0, bottom at y=-tileH (matches MTLogo)
		v[0].pos[0] = x0; v[0].pos[1] = 0.0f;   v[0].uv[0] = u0; v[0].uv[1] = 0.0f;
		v[1].pos[0] = x1; v[1].pos[1] = 0.0f;   v[1].uv[0] = u1; v[1].uv[1] = 0.0f;
		v[2].pos[0] = x0; v[2].pos[1] = -tileH; v[2].uv[0] = u0; v[2].uv[1] = 1.0f;
		v[3].pos[0] = x0; v[3].pos[1] = -tileH; v[3].uv[0] = u0; v[3].uv[1] = 1.0f;
		v[4].pos[0] = x1; v[4].pos[1] = 0.0f;   v[4].uv[0] = u1; v[4].uv[1] = 0.0f;
		v[5].pos[0] = x1; v[5].pos[1] = -tileH; v[5].uv[0] = u1; v[5].uv[1] = 1.0f;

		for (k = 0; k < 6; k++) {
			v[k].pos[0] += MTLOGO11_POS_X;
			v[k].pos[1] += MTLOGO11_POS_Y;
			v[k].pos[2] = 0.0f;
			v[k].normal[0] = 0.0f; v[k].normal[1] = 0.0f; v[k].normal[2] = -1.0f;
			v[k].color = 0xFF000000;   // start black; gradation overwrites per frame
		}
	}

	return result;
}

//******************************************************************************
// Gradation: a bright->dark wave swept across the tiles (mirrors MTLogo)
//******************************************************************************
void MTLogo11::_SetGradationColor()
{
	unsigned long i = 0, k = 0;
	unsigned long sceneTime = 0, delay = 0, tileTime = 0;
	float color = 0.0f;

	if (m_StartTime == 0) m_StartTime = timeGetTime();
	sceneTime = timeGetTime() - m_StartTime;

	for (i = 0; i < MTLOGO11_TILE_NUM; i++) {
		delay = i * (MTLOGO11_GRADATION_TIME / MTLOGO11_TILE_NUM);
		tileTime = (sceneTime < delay) ? 0 : (sceneTime - delay);

		if (tileTime < MTLOGO11_GRADATION_TIME) {
			color = (float)tileTime / (float)MTLOGO11_GRADATION_TIME;             // ramp up
		}
		else if (tileTime < (MTLOGO11_GRADATION_TIME * 2)) {
			color = 1.0f - ((float)(tileTime - MTLOGO11_GRADATION_TIME) / (float)MTLOGO11_GRADATION_TIME);  // ramp down
		}
		else {
			color = 0.0f;
		}

		BYTE b = (BYTE)(color * 255.0f);
		unsigned long c = 0xFF000000 | ((unsigned long)b << 16) | ((unsigned long)b << 8) | (unsigned long)b;

		DXP11_VERTEX* v = m_pVtx + (i * 6);
		for (k = 0; k < 6; k++) v[k].color = c;
	}
}

//******************************************************************************
// Draw
//******************************************************************************
int MTLogo11::DrawDX11(ID3D11DeviceContext* pContext, float aspect)
{
	int result = 0;
	DXP11_VERTEX* pv = NULL;
	D3DXMATRIX d3dView, d3dProj;
	XMFLOAT4 light(0.0f, 0.0f, 1.0f, 0.0f);

	if (!m_Ready || (m_pVtx == NULL) || (aspect <= 0.0f)) return 0;

	// update the gradation colors and upload the strip
	_SetGradationColor();
	if (m_Prim.LockVertex(pContext, &pv) == 0) {
		memcpy(pv, m_pVtx, sizeof(DXP11_VERTEX) * MTLOGO11_VERTEX_NUM);
		m_Prim.UnlockVertex(pContext);
	}

	// receding camera (z slowly increases), then build view*proj
	m_CamPosZ += MTLOGO11_CAMERA_POSZ_DELTA;
	m_Camera.Initialize();
	m_Camera.SetBaseParam(MTLOGO11_VIEW_ANGLE, MTLOGO11_NEAR_PLANE, MTLOGO11_FAR_PLANE);
	m_Camera.SetPosition(
			D3DXVECTOR3(0.0f, 0.0f, m_CamPosZ),
			D3DXVECTOR3(0.0f, 0.0f, 0.0f),
			D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	result = m_Camera.GetMatrices(aspect, &d3dView, &d3dProj);
	if (result != 0) return result;

	XMMATRIX view = XMLoadFloat4x4((const XMFLOAT4X4*)&d3dView);
	XMMATRIX proj = XMLoadFloat4x4((const XMFLOAT4X4*)&d3dProj);
	XMMATRIX viewProj = view * proj;

	m_Prim.SetWorldMatrix(XMMatrixIdentity());
	m_Prim.SetTexture(m_pSRV);
	m_Prim.SetAdditiveBlend(false);
	m_Prim.Draw(pContext, viewProj, light, 2 * MTLOGO11_TILE_NUM, 0);

	return 0;
}
