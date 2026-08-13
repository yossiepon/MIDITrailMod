//******************************************************************************
//
// MIDITrail / DXTexture11
//
// Direct3D 11 texture loader (WIC-based).
//
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXTexture11.h"
#include <wincodec.h>

using namespace YNBaseLib;


//******************************************************************************
// Load an image file into a shader resource view (RGBA8)
//******************************************************************************
int DXTexture11::LoadFromFile(
		ID3D11Device* pDevice,
		const TCHAR* pImgFilePath,
		ID3D11ShaderResourceView** ppSRV,
		unsigned int* pWidth,
		unsigned int* pHeight
	)
{
	int result = 0;
	HRESULT hr = S_OK;
	WCHAR pathW[_MAX_PATH] = {0};
	IWICImagingFactory* pFactory = NULL;
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pFrame = NULL;
	IWICFormatConverter* pConverter = NULL;
	UINT width = 0;
	UINT height = 0;
	unsigned char* pPixels = NULL;
	ID3D11Texture2D* pTex = NULL;

	if (pDevice == NULL || pImgFilePath == NULL || ppSRV == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	*ppSRV = NULL;
	if (pWidth != NULL) *pWidth = 0;
	if (pHeight != NULL) *pHeight = 0;

	MultiByteToWideChar(CP_ACP, 0, pImgFilePath, -1, pathW, _MAX_PATH);

	hr = CoCreateInstance(
				CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
				IID_PPV_ARGS(&pFactory));
	if (FAILED(hr) || pFactory == NULL) {
		result = YN_SET_ERR("WIC factory creation failed.", hr, 0);
		goto EXIT;
	}

	hr = pFactory->CreateDecoderFromFilename(
				pathW, NULL, GENERIC_READ,
				WICDecodeMetadataCacheOnDemand, &pDecoder);
	if (FAILED(hr)) {
		result = YN_SET_ERR("WIC decode error.", hr, 0);
		goto EXIT;
	}

	hr = pDecoder->GetFrame(0, &pFrame);
	if (FAILED(hr)) {
		result = YN_SET_ERR("WIC frame error.", hr, 0);
		goto EXIT;
	}

	hr = pFrame->GetSize(&width, &height);
	if (FAILED(hr) || width == 0 || height == 0) {
		result = YN_SET_ERR("WIC size error.", hr, 0);
		goto EXIT;
	}

	hr = pFactory->CreateFormatConverter(&pConverter);
	if (FAILED(hr)) {
		result = YN_SET_ERR("WIC converter creation failed.", hr, 0);
		goto EXIT;
	}
	hr = pConverter->Initialize(
				pFrame, GUID_WICPixelFormat32bppRGBA,
				WICBitmapDitherTypeNone, NULL, 0.0,
				WICBitmapPaletteTypeCustom);
	if (FAILED(hr)) {
		result = YN_SET_ERR("WIC format conversion failed.", hr, 0);
		goto EXIT;
	}

	try {
		pPixels = new unsigned char[(size_t)width * height * 4];
	}
	catch (std::bad_alloc&) {
		result = YN_SET_ERR("Could not allocate memory.", width, height);
		goto EXIT;
	}
	hr = pConverter->CopyPixels(NULL, width * 4, width * height * 4, pPixels);
	if (FAILED(hr)) {
		result = YN_SET_ERR("WIC pixel copy failed.", hr, 0);
		goto EXIT;
	}

	result = CreateFromRGBA(pDevice, pPixels, width, height, ppSRV);
	if (result != 0) goto EXIT;

	if (pWidth != NULL) *pWidth = width;
	if (pHeight != NULL) *pHeight = height;

EXIT:;
	if (pTex != NULL) pTex->Release();
	delete[] pPixels;
	if (pConverter != NULL) pConverter->Release();
	if (pFrame != NULL) pFrame->Release();
	if (pDecoder != NULL) pDecoder->Release();
	if (pFactory != NULL) pFactory->Release();
	return result;
}

//******************************************************************************
// Build an SRV from a CPU-side RGBA8 pixel buffer
//******************************************************************************
int DXTexture11::CreateFromRGBA(
		ID3D11Device* pDevice,
		const unsigned char* pPixels,
		unsigned int width,
		unsigned int height,
		ID3D11ShaderResourceView** ppSRV
	)
{
	int result = 0;
	HRESULT hr = S_OK;
	ID3D11Texture2D* pTex = NULL;

	if (pDevice == NULL || pPixels == NULL || ppSRV == NULL ||
		width == 0 || height == 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	*ppSRV = NULL;

	D3D11_TEXTURE2D_DESC td = {};
	td.Width     = width;
	td.Height    = height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage     = D3D11_USAGE_IMMUTABLE;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA srd = {};
	srd.pSysMem     = pPixels;
	srd.SysMemPitch = width * 4;

	hr = pDevice->CreateTexture2D(&td, &srd, &pTex);
	if (FAILED(hr)) {
		result = YN_SET_ERR("CreateTexture2D failed.", hr, 0);
		goto EXIT;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
	sd.Format              = td.Format;
	sd.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
	sd.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(pTex, &sd, ppSRV);
	if (FAILED(hr)) {
		result = YN_SET_ERR("CreateShaderResourceView failed.", hr, 0);
		goto EXIT;
	}

EXIT:;
	if (pTex != NULL) pTex->Release();
	return result;
}
