//******************************************************************************
//
// RTDiagLib / RDGpuVendorTelemetryAdlx
//
// AMD GPU vendor telemetry provider via ADLX.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "RDGpuVendorTelemetryAdlx.h"

#include <spdlog/spdlog.h>

using namespace adlx;

RDGpuVendorTelemetryAdlxProvider::RDGpuVendorTelemetryAdlxProvider()
	: m_hAdlx(NULL)
	, m_initialized(false)
	, m_pfnInitialize(nullptr)
	, m_pfnTerminate(nullptr)
	, m_pSystem(nullptr)
	, m_pPerfMon(nullptr)
	, m_pGpu(nullptr)
{
}

RDGpuVendorTelemetryAdlxProvider::~RDGpuVendorTelemetryAdlxProvider()
{
	Shutdown();
}

bool RDGpuVendorTelemetryAdlxProvider::Initialize()
{
	auto logger = spdlog::get("RD");

#ifdef _M_AMD64
	m_hAdlx = LoadLibraryA("amdadlx64.dll");
#else
	m_hAdlx = LoadLibraryA("amdadlx32.dll");
#endif

	if (!m_hAdlx) {
		if (logger) logger->debug("ADLX: DLL not found");
		return false;
	}

	m_pfnInitialize = reinterpret_cast<ADLXInitialize_Fn>(
		GetProcAddress(m_hAdlx, ADLX_INIT_FUNCTION_NAME));
	m_pfnTerminate = reinterpret_cast<ADLXTerminate_Fn>(
		GetProcAddress(m_hAdlx, ADLX_TERMINATE_FUNCTION_NAME));

	if (!m_pfnInitialize || !m_pfnTerminate) {
		if (logger) logger->warn("ADLX: required functions not found");
		FreeLibrary(m_hAdlx);
		m_hAdlx = NULL;
		return false;
	}

	int initRes = m_pfnInitialize(ADLX_FULL_VERSION, &m_pSystem);
	if (initRes < 0 || !m_pSystem) {
		if (logger) logger->warn("ADLX: Initialize failed: {}", initRes);
		FreeLibrary(m_hAdlx);
		m_hAdlx = NULL;
		return false;
	}

	ADLX_RESULT res = m_pSystem->GetPerformanceMonitoringServices(&m_pPerfMon);
	if (!ADLX_SUCCEEDED(res) || !m_pPerfMon) {
		if (logger) logger->warn("ADLX: GetPerformanceMonitoringServices failed: {}", static_cast<int>(res));
		m_pfnTerminate();
		m_pSystem = nullptr;
		FreeLibrary(m_hAdlx);
		m_hAdlx = NULL;
		return false;
	}

	IADLXGPUList* pGPUList = nullptr;
	res = m_pSystem->GetGPUs(&pGPUList);
	if (!ADLX_SUCCEEDED(res) || !pGPUList || pGPUList->Size() == 0) {
		if (logger) logger->warn("ADLX: GetGPUs failed or no GPUs");
		if (pGPUList) pGPUList->Release();
		m_pPerfMon->Release();
		m_pPerfMon = nullptr;
		m_pfnTerminate();
		m_pSystem = nullptr;
		FreeLibrary(m_hAdlx);
		m_hAdlx = NULL;
		return false;
	}

	res = pGPUList->At(0, &m_pGpu);
	pGPUList->Release();

	if (!ADLX_SUCCEEDED(res) || !m_pGpu) {
		if (logger) logger->warn("ADLX: GPU At(0) failed");
		m_pPerfMon->Release();
		m_pPerfMon = nullptr;
		m_pfnTerminate();
		m_pSystem = nullptr;
		FreeLibrary(m_hAdlx);
		m_hAdlx = NULL;
		return false;
	}

	m_initialized = true;
	if (logger) logger->debug("ADLX: initialized");
	return true;
}

void RDGpuVendorTelemetryAdlxProvider::Shutdown()
{
	if (m_pGpu) {
		m_pGpu->Release();
		m_pGpu = nullptr;
	}
	if (m_pPerfMon) {
		m_pPerfMon->Release();
		m_pPerfMon = nullptr;
	}
	if (m_initialized && m_pfnTerminate) {
		m_pfnTerminate();
	}
	m_pSystem = nullptr;
	m_initialized = false;

	if (m_hAdlx) {
		FreeLibrary(m_hAdlx);
		m_hAdlx = NULL;
	}
}

void RDGpuVendorTelemetryAdlxProvider::CollectTelemetry(RDGpuVendorTelemetryData& data)
{
	data.isAvailable = m_initialized;
	data.temperatureCelsius = { false, 0.0 };
	data.coreClockMHz       = { false, 0.0 };
	data.memoryClockMHz     = { false, 0.0 };
	data.usagePercent       = { false, 0.0 };
	data.powerWatts         = { false, 0.0 };
	data.fanSpeedRPM        = { false, 0.0 };

	if (!m_initialized) return;

	IADLXGPUMetrics* pMetrics = nullptr;
	ADLX_RESULT res = m_pPerfMon->GetCurrentGPUMetrics(m_pGpu, &pMetrics);
	if (!ADLX_SUCCEEDED(res) || !pMetrics) return;

	IADLXGPUMetricsSupport* pSupport = nullptr;
	m_pPerfMon->GetSupportedGPUMetrics(m_pGpu, &pSupport);

	adlx_bool supported = false;

	if (pSupport) {
		pSupport->IsSupportedGPUTemperature(&supported);
		if (supported) {
			adlx_double temp = 0.0;
			if (ADLX_SUCCEEDED(pMetrics->GPUTemperature(&temp))) {
				data.temperatureCelsius = { true, temp };
			}
		}

		supported = false;
		pSupport->IsSupportedGPUClockSpeed(&supported);
		if (supported) {
			adlx_int clock = 0;
			if (ADLX_SUCCEEDED(pMetrics->GPUClockSpeed(&clock))) {
				data.coreClockMHz = { true, static_cast<double>(clock) };
			}
		}

		supported = false;
		pSupport->IsSupportedGPUVRAMClockSpeed(&supported);
		if (supported) {
			adlx_int vramClock = 0;
			if (ADLX_SUCCEEDED(pMetrics->GPUVRAMClockSpeed(&vramClock))) {
				data.memoryClockMHz = { true, static_cast<double>(vramClock) };
			}
		}

		supported = false;
		pSupport->IsSupportedGPUUsage(&supported);
		if (supported) {
			adlx_double usage = 0.0;
			if (ADLX_SUCCEEDED(pMetrics->GPUUsage(&usage))) {
				data.usagePercent = { true, usage };
			}
		}

		// Power: board power preferred over core power (ADR-0137)
		supported = false;
		pSupport->IsSupportedGPUTotalBoardPower(&supported);
		if (supported) {
			adlx_double power = 0.0;
			if (ADLX_SUCCEEDED(pMetrics->GPUTotalBoardPower(&power))) {
				data.powerWatts = { true, power };
			}
		}
		if (!data.powerWatts.supported) {
			supported = false;
			pSupport->IsSupportedGPUPower(&supported);
			if (supported) {
				adlx_double power = 0.0;
				if (ADLX_SUCCEEDED(pMetrics->GPUPower(&power))) {
					data.powerWatts = { true, power };
				}
			}
		}

		supported = false;
		pSupport->IsSupportedGPUFanSpeed(&supported);
		if (supported) {
			adlx_int fanRPM = 0;
			if (ADLX_SUCCEEDED(pMetrics->GPUFanSpeed(&fanRPM))) {
				data.fanSpeedRPM = { true, static_cast<double>(fanRPM) };
			}
		}

		supported = false;
		pSupport->IsSupportedGPUHotspotTemperature(&supported);
		if (supported) {
			adlx_double temp = 0.0;
			if (ADLX_SUCCEEDED(pMetrics->GPUHotspotTemperature(&temp))) {
				data.hotspotTemperatureCelsius = { true, temp };
			}
		}

		supported = false;
		pSupport->IsSupportedGPUVoltage(&supported);
		if (supported) {
			adlx_int voltage = 0;
			if (ADLX_SUCCEEDED(pMetrics->GPUVoltage(&voltage))) {
				data.voltageVolts = { true, static_cast<double>(voltage) / 1000.0 };
			}
		}

		supported = false;
		pSupport->IsSupportedGPUIntakeTemperature(&supported);
		if (supported) {
			adlx_double temp = 0.0;
			if (ADLX_SUCCEEDED(pMetrics->GPUIntakeTemperature(&temp))) {
				data.intakeTemperatureCelsius = { true, temp };
			}
		}

		pSupport->Release();
	}

	pMetrics->Release();
}
