#include "stdafx.h"
#include "RDGpuVendorTelemetry.h"
#include "RDDiagManager.h"
#include "RDGpuVendorTelemetryNullProvider.h"
#include "RDGpuVendorTelemetryNvApi.h"
#include "RDGpuVendorTelemetryAdlx.h"
#include "RDGpuVendorTelemetryIgcl.h"
#include <spdlog/spdlog.h>

std::unique_ptr<IRDGpuVendorTelemetryProvider> CreateRDGpuVendorTelemetryProvider(uint32_t vendorId)
{
	std::unique_ptr<IRDGpuVendorTelemetryProvider> provider;

	switch (vendorId) {
	case 0x10DE:
		provider = std::make_unique<RDGpuVendorTelemetryNvApiProvider>();
		break;
	case 0x1002:
		provider = std::make_unique<RDGpuVendorTelemetryAdlxProvider>();
		break;
	case 0x8086:
#ifdef _M_AMD64
		provider = std::make_unique<RDGpuVendorTelemetryIgclProvider>();
#else
		provider = std::make_unique<RDGpuVendorTelemetryNullProvider>();
#endif
		break;
	default:
		provider = std::make_unique<RDGpuVendorTelemetryNullProvider>();
		break;
	}

	if (provider && provider->Initialize()) {
		return provider;
	}

	auto logger = spdlog::get("RD");
	if (logger) logger->debug("GpuTelemetry: vendor 0x{:04X} provider init failed, using NullProvider", vendorId);
	return std::make_unique<RDGpuVendorTelemetryNullProvider>();
}

RDGpuVendorTelemetry::RDGpuVendorTelemetry()
	: m_vendorId(0)
{
}

RDGpuVendorTelemetry::~RDGpuVendorTelemetry()
{
	if (m_provider) {
		m_provider->Shutdown();
	}
}

void RDGpuVendorTelemetry::SetVendorId(uint32_t vendorId)
{
	m_vendorId = vendorId;
}

bool RDGpuVendorTelemetry::InitializeProvider()
{
	m_provider = CreateRDGpuVendorTelemetryProvider(m_vendorId);
	return m_provider != nullptr;
}

void RDGpuVendorTelemetry::CollectIntervalPolling()
{
	if (!m_provider) return;

	RDGpuVendorTelemetryData data = {};
	m_provider->CollectTelemetry(data);

	if (!data.isAvailable) return;

	if (data.temperatureCelsius.supported) {
		RDDiagManager::SetFloat(RDMetricId::GpuTemperature, data.temperatureCelsius.value);
	}
	if (data.coreClockMHz.supported) {
		RDDiagManager::SetInt(RDMetricId::GpuCoreClock, static_cast<int64_t>(data.coreClockMHz.value));
	}
	if (data.memoryClockMHz.supported) {
		RDDiagManager::SetInt(RDMetricId::GpuMemoryClock, static_cast<int64_t>(data.memoryClockMHz.value));
	}
	if (data.powerWatts.supported) {
		RDDiagManager::SetFloat(RDMetricId::GpuPowerWatts, data.powerWatts.value);
	}
	if (data.fanSpeedRPM.supported) {
		RDDiagManager::SetInt(RDMetricId::GpuFanSpeedRPM, static_cast<int64_t>(data.fanSpeedRPM.value));
	}
	if (data.usagePercent.supported) {
		RDDiagManager::SetFloat(RDMetricId::GpuUsageVendorPercent, data.usagePercent.value);
	}
}
