#include "stdafx.h"
#include "RDGpuInfo.h"
#include "RDDiagManager.h"
#include <spdlog/spdlog.h>

using Microsoft::WRL::ComPtr;

void RDGpuInfo::SetDevice(ID3D11Device* pDevice)
{
	if (!pDevice) return;

	ComPtr<IDXGIDevice> dxgiDevice;
	HRESULT hr = pDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
	if (FAILED(hr)) {
		auto logger = spdlog::get("RD");
		if (logger) logger->warn("Failed to QI IDXGIDevice: 0x{:08X}", static_cast<unsigned>(hr));
		return;
	}

	hr = dxgiDevice->GetAdapter(&m_pAdapter);
	if (FAILED(hr)) {
		auto logger = spdlog::get("RD");
		if (logger) logger->warn("Failed to GetAdapter: 0x{:08X}", static_cast<unsigned>(hr));
		return;
	}

	m_pAdapter->QueryInterface(IID_PPV_ARGS(&m_pAdapter3));
}

void RDGpuInfo::CollectStartup()
{
	if (!m_pAdapter) return;

	DXGI_ADAPTER_DESC desc = {};
	HRESULT hr = m_pAdapter->GetDesc(&desc);
	if (FAILED(hr)) return;

	char gpuName[256] = {};
	WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, gpuName, sizeof(gpuName), NULL, NULL);
	RDDiagManager::SetString(RDMetricId::GpuName, gpuName);

	int64_t vramMB = static_cast<int64_t>(desc.DedicatedVideoMemory / (1024 * 1024));
	RDDiagManager::SetInt(RDMetricId::GpuVramTotalMB, vramMB);

	if (m_pAdapter3) {
		DXGI_QUERY_VIDEO_MEMORY_INFO memInfo = {};
		hr = m_pAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);
		if (SUCCEEDED(hr)) {
			int64_t usedMB = static_cast<int64_t>(memInfo.CurrentUsage / (1024 * 1024));
			int64_t budgetMB = static_cast<int64_t>(memInfo.Budget / (1024 * 1024));
			int64_t freeMB = budgetMB - usedMB;
			if (freeMB < 0) freeMB = 0;
			RDDiagManager::SetInt(RDMetricId::GpuVramUsedMB, usedMB);
			RDDiagManager::SetInt(RDMetricId::GpuVramBudgetMB, budgetMB);
			RDDiagManager::SetInt(RDMetricId::GpuVramFreeMB, freeMB);
		}
	}
}
