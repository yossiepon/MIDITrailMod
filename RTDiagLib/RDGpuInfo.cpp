//******************************************************************************
//
// RTDiagLib / RDGpuInfo
//
// GPU information and VRAM usage collector via DXGI/PDH.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "RDGpuInfo.h"
#include "RDDiagManager.h"
#include <spdlog/spdlog.h>
#include <pdhmsg.h>
#include <string>

#pragma comment(lib, "pdh.lib")

using Microsoft::WRL::ComPtr;

RDGpuInfo::RDGpuInfo()
	: m_vendorId(0)
	, m_pdhQuery(NULL)
	, m_pdhGpuCounter(NULL)
	, m_pdhInitialized(false)
{
}

RDGpuInfo::~RDGpuInfo()
{
	if (m_pdhQuery) {
		PdhCloseQuery(m_pdhQuery);
		m_pdhQuery = NULL;
	}
}

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

	DXGI_ADAPTER_DESC adapterDesc = {};
	if (SUCCEEDED(m_pAdapter->GetDesc(&adapterDesc))) {
		m_vendorId = adapterDesc.VendorId;
	}
}

void RDGpuInfo::CollectStartup()
{
	if (!m_pAdapter) return;

	DXGI_ADAPTER_DESC desc = {};
	HRESULT hr = m_pAdapter->GetDesc(&desc);
	if (FAILED(hr)) return;

	m_vendorId = desc.VendorId;

	char gpuName[256] = {};
	WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, gpuName, sizeof(gpuName), NULL, NULL);
	RDDiagManager::SetString(RDMetricId::GpuName, gpuName);

	int64_t vramMB = static_cast<int64_t>(desc.DedicatedVideoMemory / (1024 * 1024));
	RDDiagManager::SetInt(RDMetricId::GpuVramTotalMB, vramMB);
	RDDiagManager::SetInt(RDMetricId::GpuVramTotalGB, (vramMB + 1023) / 1024);

	_CollectVram();
	_InitPdh();
}

void RDGpuInfo::CollectIntervalPolling()
{
	_CollectVram();
	_CollectGpuUsage();
}

void RDGpuInfo::_CollectVram()
{
	if (!m_pAdapter3) return;

	DXGI_QUERY_VIDEO_MEMORY_INFO memInfo = {};
	HRESULT hr = m_pAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);
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

void RDGpuInfo::_InitPdh()
{
	PDH_STATUS status = PdhOpenQueryW(NULL, 0, &m_pdhQuery);
	if (status != ERROR_SUCCESS) {
		auto logger = spdlog::get("RD");
		if (logger) logger->warn("PdhOpenQuery failed: 0x{:08X}", static_cast<unsigned>(status));
		return;
	}

	// Enumerate GPU engine counters to find engtype_3D instances
	DWORD counterListSize = 0;
	DWORD instanceListSize = 0;
	PdhEnumObjectItemsW(NULL, NULL, L"GPU Engine",
		NULL, &counterListSize, NULL, &instanceListSize,
		PERF_DETAIL_WIZARD, 0);

	if (instanceListSize == 0) {
		auto logger = spdlog::get("RD");
		if (logger) logger->warn("No GPU Engine counters found");
		return;
	}

	std::vector<wchar_t> instanceList(instanceListSize);
	std::vector<wchar_t> counterList(counterListSize);
	status = PdhEnumObjectItemsW(NULL, NULL, L"GPU Engine",
		counterList.data(), &counterListSize,
		instanceList.data(), &instanceListSize,
		PERF_DETAIL_WIZARD, 0);

	if (status != ERROR_SUCCESS) return;

	// Find first engtype_3D instance and build wildcard counter path
	// Instance format: "pid_XXXX_luid_0xYY_0xZZZZZZZZ_phys_N_eng_M_engtype_3D"
	std::wstring counterPath;
	const wchar_t* p = instanceList.data();
	while (*p) {
		std::wstring instance(p);
		if (instance.find(L"engtype_3D") != std::wstring::npos) {
			// Use wildcard to aggregate all 3D engine instances
			counterPath = L"\\GPU Engine(*)\\Utilization Percentage";
			break;
		}
		p += wcslen(p) + 1;
	}

	if (counterPath.empty()) {
		auto logger = spdlog::get("RD");
		if (logger) logger->warn("No engtype_3D GPU engine instance found");
		return;
	}

	status = PdhAddCounterW(m_pdhQuery, counterPath.c_str(), 0, &m_pdhGpuCounter);
	if (status != ERROR_SUCCESS) {
		auto logger = spdlog::get("RD");
		if (logger) logger->warn("PdhAddCounter failed: 0x{:08X}", static_cast<unsigned>(status));
		return;
	}

	// Initial data collection (first call returns no data, establishes baseline)
	PdhCollectQueryData(m_pdhQuery);
	m_pdhInitialized = true;
}

void RDGpuInfo::_CollectGpuUsage()
{
	if (!m_pdhInitialized) return;

	PDH_STATUS status = PdhCollectQueryData(m_pdhQuery);
	if (status != ERROR_SUCCESS) return;

	// Get all engtype_3D instance values and find the maximum
	DWORD bufferSize = 0;
	DWORD itemCount = 0;
	status = PdhGetFormattedCounterArrayW(m_pdhGpuCounter, PDH_FMT_DOUBLE,
		&bufferSize, &itemCount, NULL);

	if (status != PDH_MORE_DATA || bufferSize == 0) return;

	std::vector<uint8_t> buffer(bufferSize);
	auto* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer.data());
	status = PdhGetFormattedCounterArrayW(m_pdhGpuCounter, PDH_FMT_DOUBLE,
		&bufferSize, &itemCount, items);

	if (status != ERROR_SUCCESS) return;

	double maxUsage = 0.0;
	for (DWORD i = 0; i < itemCount; i++) {
		if (items[i].FmtValue.CStatus == PDH_CSTATUS_VALID_DATA) {
			std::wstring name(items[i].szName);
			if (name.find(L"engtype_3D") != std::wstring::npos) {
				if (items[i].FmtValue.doubleValue > maxUsage) {
					maxUsage = items[i].FmtValue.doubleValue;
				}
			}
		}
	}

	RDDiagManager::SetFloat(RDMetricId::GpuUsagePercent, maxUsage);
}
