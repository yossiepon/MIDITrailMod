//******************************************************************************
//
// MIDITrail / DXTexture11
//
// Direct3D 11 texture loader (WIC-based; no DXSDK / d3dx dependency)
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <tchar.h>


class DXTexture11
{
public:
	// Load an image file (PNG/BMP/JPG/...) into a shader resource view.
	// Returns 0 on success. *ppSRV is caller-owned (Release it).
	static int LoadFromFile(
			ID3D11Device* pDevice,
			const TCHAR* pImgFilePath,
			ID3D11ShaderResourceView** ppSRV,
			unsigned int* pWidth,
			unsigned int* pHeight
		);

	// Build a shader resource view from a CPU-side RGBA8 pixel buffer (row-major,
	// pitch = width*4). Used for dynamically rasterized text (note lyrics).
	// Returns 0 on success. *ppSRV is caller-owned (Release it).
	static int CreateFromRGBA(
			ID3D11Device* pDevice,
			const unsigned char* pPixels,
			unsigned int width,
			unsigned int height,
			ID3D11ShaderResourceView** ppSRV
		);
};
