#pragma once

#include "RDInterfaces.h"
#include <d3d11.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

class RDGpuInfo : public IRDStartupComponent
{
public:
	RDGpuInfo() = default;
	virtual ~RDGpuInfo() = default;

	void SetDevice(ID3D11Device* pDevice);
	void CollectStartup() override;

	Microsoft::WRL::ComPtr<IDXGIAdapter3> GetAdapter3() const { return m_pAdapter3; }

private:
	Microsoft::WRL::ComPtr<IDXGIAdapter> m_pAdapter;
	Microsoft::WRL::ComPtr<IDXGIAdapter3> m_pAdapter3;
};
