//******************************************************************************
//
// MIDITrail / MTBackgroundImage11
//
// Background image renderer.
//
// Copyright (C) 2016-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
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
// コンストラクタ / デストラクタ
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
// 生成
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
// 解放
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
// 頂点生成
// NDC 座標で画面全体をカバーするクワッドを作成。
// アスペクト比の違いは UV 座標で吸収（画像の中央をクロップ表示）。
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
		// クライアント領域のサイズ
		bresult = GetClientRect(m_hWnd, &rect);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
		float cw = (float)(rect.right - rect.left);
		float ch = (float)(rect.bottom - rect.top);

		// アスペクト比に応じた UV 座標計算
		float ratio_cwh = cw / ch;
		float ratio_iwh = (float)m_ImgWidth / (float)m_ImgHeight;

		float u0 = 0.0f, u1 = 1.0f;
		float v0 = 0.0f, v1 = 1.0f;

		if (ratio_cwh < ratio_iwh) {
			// 画像の方が横長 → 左右をクロップ
			float visibleFraction = ratio_cwh / ratio_iwh;
			float margin = (1.0f - visibleFraction) / 2.0f;
			u0 = margin;
			u1 = 1.0f - margin;
		}
		else if (ratio_cwh > ratio_iwh) {
			// 画像の方が縦長 → 上下をクロップ
			float visibleFraction = ratio_iwh / ratio_cwh;
			float margin = (1.0f - visibleFraction) / 2.0f;
			v0 = margin;
			v1 = 1.0f - margin;
		}

		// NDC 座標で画面全体のクワッド (-1〜+1)
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
// 描画
// WVP を単位行列にして NDC 座標のクワッドをそのまま描画する。
//******************************************************************************
int MTBackgroundImage11::Draw(ID3D11DeviceContext* pContext)
{
	if (!m_isEnable || !m_isReady) return 0;

	// WVP = 単位行列 → NDC 座標がそのまま出力される
	Matrix identity;
	m_Primitive.SetWorldMatrix(identity);
	m_Primitive.SetTexture(m_pSRV);

	Vector4 lightDir(0.0f, 0.0f, 0.0f, 0.0f);
	return m_Primitive.Draw(pContext, identity, lightDir);
}

//******************************************************************************
// 設定ファイル初期化
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
// テクスチャ読み込み
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

	// MBCS パスに変換して DXTexture11 で読み込み
	{
		TCHAR imgPathA[_MAX_PATH] = {_T('\0')};
		WideCharToMultiByte(CP_ACP, 0, imageFilePathW, -1, imgPathA, _MAX_PATH, NULL, NULL);
		result = DXTexture11::LoadFromFile(pDevice, imgPathA, &m_pSRV, &m_ImgWidth, &m_ImgHeight);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTBackgroundImage11::Reset()
{
}

//******************************************************************************
// ウィンドウリサイズ通知
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
