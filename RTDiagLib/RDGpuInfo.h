//******************************************************************************
//
// RTDiagLib / RDGpuInfo
//
// GPU information and VRAM usage collector via DXGI/PDH.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "RDInterfaces.h"
#include <d3d11.h>
#include <dxgi1_4.h>
#include <pdh.h>
#include <wrl/client.h>

class RDGpuInfo : public IRDStartupComponent, public IRDIntervalPollingComponent
{
public:
	RDGpuInfo();
	virtual ~RDGpuInfo();

	void SetDevice(ID3D11Device* pDevice);
	void CollectStartup() override;
	void CollectIntervalPolling() override;
	DWORD GetPollingIntervalMs() const override { return 1000; }

	UINT GetVendorId() const { return m_vendorId; }

private:
	void _InitPdh();
	void _CollectVram();
	void _CollectGpuUsage();

	Microsoft::WRL::ComPtr<IDXGIAdapter> m_pAdapter;
	Microsoft::WRL::ComPtr<IDXGIAdapter3> m_pAdapter3;

	UINT         m_vendorId;
	PDH_HQUERY   m_pdhQuery;
	PDH_HCOUNTER m_pdhGpuCounter;
	bool         m_pdhInitialized;
};
