//******************************************************************************
//
// MIDITrail / MTBackgroundImage11
//
// Background image renderer.
//
// Copyright (C) 2016-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "shlwapi.h"
#include "MTParam.h"
#include "MTBackgroundImage11.h"
#include <mbctype.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTBackgroundImage11::MTBackgroundImage11()
{
	m_hWnd = NULL;
	m_pContext = NULL;
	m_pSRV = NULL;
	m_ImgWidth = 0;
	m_ImgHeight = 0;
	m_isReady = false;
}

MTBackgroundImage11::~MTBackgroundImage11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTBackgroundImage11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		HWND hWnd
	)
{
	int result = 0;

	Release();

	m_hWnd = hWnd;
	m_pContext = pContext;

	result = _InitConfFile();
	if (result != 0) goto EXIT;

	result = _LoadTexture(pDevice);
	if (result != 0) goto EXIT;

	if (m_pSRV == NULL) goto EXIT;

	result = _CreateVertices(pDevice, pContext);
	if (result != 0) goto EXIT;

	m_Primitive.SetLightEnable(false);
	m_Primitive.SetDepthWrite(false);
	m_isReady = true;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTBackgroundImage11::Release()
{
	m_Primitive.Release();
	if (m_pSRV != NULL) {
		m_pSRV->Release();
		m_pSRV = NULL;
	}
	m_isReady = false;
}

//******************************************************************************
// Vertex generation
// Creates a quad covering the whole screen in NDC coordinates.
// Aspect-ratio differences are absorbed via UV coordinates (center-crop display of the image).
//******************************************************************************
int MTBackgroundImage11::_CreateVertices(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	BOOL bresult = 0;
	RECT rect;

	result = m_Primitive.CreateVertexBuffer(pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_Primitive.CreateIndexBuffer(pDevice, 6);
	if (result != 0) goto EXIT;

	result = m_Primitive.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(pContext, &pIndex);
	if (result != 0) goto EXIT;

	{
		// Client area size
		bresult = GetClientRect(m_hWnd, &rect);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
		float cw = (float)(rect.right - rect.left);
		float ch = (float)(rect.bottom - rect.top);

		// UV coordinate calculation based on aspect ratio
		float ratio_cwh = cw / ch;
		float ratio_iwh = (float)m_ImgWidth / (float)m_ImgHeight;

		float u0 = 0.0f, u1 = 1.0f;
		float v0 = 0.0f, v1 = 1.0f;

		if (ratio_cwh < ratio_iwh) {
			// Image is wider -> crop left and right
			float visibleFraction = ratio_cwh / ratio_iwh;
			float margin = (1.0f - visibleFraction) / 2.0f;
			u0 = margin;
			u1 = 1.0f - margin;
		}
		else if (ratio_cwh > ratio_iwh) {
			// Image is taller -> crop top and bottom
			float visibleFraction = ratio_iwh / ratio_cwh;
			float margin = (1.0f - visibleFraction) / 2.0f;
			v0 = margin;
			v1 = 1.0f - margin;
		}

		// Full-screen quad in NDC coordinates (-1 to +1)
		//  0(-1,+1)----1(+1,+1)
		//   |                |
		//  2(-1,-1)----3(+1,-1)

		auto setVtx = [&](unsigned long i, float x, float y, float u, float v) {
			pVertex[i].pos[0] = x;
			pVertex[i].pos[1] = y;
			pVertex[i].pos[2] = 0.0f;
			pVertex[i].normal[0] = 0.0f;
			pVertex[i].normal[1] = 0.0f;
			pVertex[i].normal[2] = -1.0f;
			pVertex[i].color = 0xFFFFFFFF;
			pVertex[i].uv[0] = u;
			pVertex[i].uv[1] = v;
		};

		setVtx(0, -1.0f, +1.0f, u0, v0);
		setVtx(1, +1.0f, +1.0f, u1, v0);
		setVtx(2, -1.0f, -1.0f, u0, v1);
		setVtx(3, +1.0f, -1.0f, u1, v1);

		pIndex[0] = 0; pIndex[1] = 1; pIndex[2] = 2;
		pIndex[3] = 2; pIndex[4] = 1; pIndex[5] = 3;
	}

	m_Primitive.UnlockVertex(pContext);
	m_Primitive.UnlockIndex(pContext);

EXIT:;
	return result;
}

//******************************************************************************
// Draw
// Draws the NDC quad as-is by setting WVP to the identity matrix.
//******************************************************************************
int MTBackgroundImage11::Draw(ID3D11DeviceContext* pContext)
{
	if (!m_isEnable || !m_isReady) return 0;

	// WVP = identity matrix -> NDC coordinates are output unchanged
	Matrix identity;
	m_Primitive.SetWorldMatrix(identity);
	m_Primitive.SetTexture(m_pSRV);

	Vector4 lightDir(0.0f, 0.0f, 0.0f, 0.0f);
	return m_Primitive.Draw(pContext, identity, lightDir);
}

//******************************************************************************
// Initialize config file
//******************************************************************************
int MTBackgroundImage11::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_GRAPHIC);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Load texture
//******************************************************************************
int MTBackgroundImage11::_LoadTexture(ID3D11Device* pDevice)
{
	int result = 0;
	int apiresult = 0;
	WCHAR imageFilePathW[_MAX_PATH] = { L'\0' };
	TCHAR imageFilePathA[_MAX_PATH] = { _T('\0') };

	result = m_ConfFile.SetCurSection(_T("Background-image"));
	if (result != 0) goto EXIT;
	result = m_ConfFile.GetWStr(_T("ImageFilePath_W"), imageFilePathW, _MAX_PATH, L"*** NO DATA ***");
	if (result != 0) goto EXIT;

	if (wcscmp(imageFilePathW, L"*** NO DATA ***") == 0) {
		memset(imageFilePathW, 0, sizeof(WCHAR) * _MAX_PATH);
		result = m_ConfFile.GetStr(_T("ImageFilePath"), imageFilePathA, _MAX_PATH, _T(""));
		if (result != 0) goto EXIT;
		if (_tcslen(imageFilePathA) > 0) {
			apiresult = MultiByteToWideChar(
								_getmbcp(),
								MB_PRECOMPOSED,
								imageFilePathA,
								(int)_tcslen(imageFilePathA),
								imageFilePathW,
								_MAX_PATH - 1
							);
			if (apiresult == 0) {
				result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
				goto EXIT;
			}
		}
	}

	if (wcslen(imageFilePathW) == 0) goto EXIT;
	if (!PathFileExistsW(imageFilePathW)) goto EXIT;

	// Convert to an MBCS path and load via DXTexture11
	{
		TCHAR imgPathA[_MAX_PATH] = {_T('\0')};
		WideCharToMultiByte(CP_ACP, 0, imageFilePathW, -1, imgPathA, _MAX_PATH, NULL, NULL);
		result = DXTexture11::LoadFromFile(pDevice, imgPathA, &m_pSRV, &m_ImgWidth, &m_ImgHeight);
		if (result != 0) {
			TCHAR warnMsg[512];
			_sntprintf_s(warnMsg, 512, _TRUNCATE, _T("Background image load failed: %s"), imgPathA);
			YN_SET_WARN(warnMsg, 0, 0);
			YN_SHOW_ERR(NULL);
			result = 0;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTBackgroundImage11::Reset()
{
}

//******************************************************************************
// Window resize notification
//******************************************************************************
void MTBackgroundImage11::OnWindowResize()
{
	if (!m_isReady || m_pContext == NULL) return;

	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	if (m_Primitive.LockVertex(m_pContext, &pVertex) != 0) return;

	RECT rect;
	if (!GetClientRect(m_hWnd, &rect)) {
		m_Primitive.UnlockVertex(m_pContext);
		return;
	}
	float cw = (float)(rect.right - rect.left);
	float ch = (float)(rect.bottom - rect.top);

	float ratio_cwh = cw / ch;
	float ratio_iwh = (float)m_ImgWidth / (float)m_ImgHeight;

	float u0 = 0.0f, u1 = 1.0f;
	float v0 = 0.0f, v1 = 1.0f;

	if (ratio_cwh < ratio_iwh) {
		float visibleFraction = ratio_cwh / ratio_iwh;
		float margin = (1.0f - visibleFraction) / 2.0f;
		u0 = margin;
		u1 = 1.0f - margin;
	}
	else if (ratio_cwh > ratio_iwh) {
		float visibleFraction = ratio_iwh / ratio_cwh;
		float margin = (1.0f - visibleFraction) / 2.0f;
		v0 = margin;
		v1 = 1.0f - margin;
	}

	pVertex[0].uv[0] = u0; pVertex[0].uv[1] = v0;
	pVertex[1].uv[0] = u1; pVertex[1].uv[1] = v0;
	pVertex[2].uv[0] = u0; pVertex[2].uv[1] = v1;
	pVertex[3].uv[0] = u1; pVertex[3].uv[1] = v1;

	m_Primitive.UnlockVertex(m_pContext);
}
