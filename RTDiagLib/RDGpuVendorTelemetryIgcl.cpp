//******************************************************************************
//
// RTDiagLib / RDGpuVendorTelemetryIgcl
//
// Intel GPU vendor telemetry provider via IGCL (64bit only).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
// Struct definitions derived from:
//   Intel Graphics Control Library SDK - https://github.com/intel/drivers.gpu.control-library
//
//******************************************************************************

#include "stdafx.h"

#ifdef _M_AMD64

#include "RDGpuVendorTelemetryIgcl.h"
#include <spdlog/spdlog.h>
#include <cstring>

#define CTL_MAKE_VERSION(major, minor) ((major << 16) | (minor & 0x0000ffff))
#define CTL_IMPL_VERSION CTL_MAKE_VERSION(1, 8)

RDGpuVendorTelemetryIgclProvider::RDGpuVendorTelemetryIgclProvider()
	: m_hIgcl(NULL)
	, m_initialized(false)
	, m_apiHandle(nullptr)
	, m_deviceHandle(nullptr)
	, m_pfnInit(nullptr)
	, m_pfnClose(nullptr)
	, m_pfnEnumerateDevices(nullptr)
	, m_pfnPowerTelemetryGet(nullptr)
	, m_hasPrevEnergy(false)
	, m_hasPrevActivity(false)
{
	memset(&m_prevEnergyCounter, 0, sizeof(m_prevEnergyCounter));
	memset(&m_prevEnergyTimestamp, 0, sizeof(m_prevEnergyTimestamp));
	memset(&m_prevGlobalActivity, 0, sizeof(m_prevGlobalActivity));
	memset(&m_prevActivityTimestamp, 0, sizeof(m_prevActivityTimestamp));
}

RDGpuVendorTelemetryIgclProvider::~RDGpuVendorTelemetryIgclProvider()
{
	Shutdown();
}

bool RDGpuVendorTelemetryIgclProvider::Initialize()
{
	auto logger = spdlog::get("RD");

	m_hIgcl = LoadLibraryA("igcl.dll");
	if (!m_hIgcl) {
		m_hIgcl = LoadLibraryA("ControlLib.dll");
	}
	if (!m_hIgcl) {
		if (logger) logger->debug("IGCL: DLL not found (tried igcl.dll and ControlLib.dll)");
		return false;
	}

	m_pfnInit = reinterpret_cast<ctlInitFunc>(GetProcAddress(m_hIgcl, "ctlInit"));
	m_pfnClose = reinterpret_cast<ctlCloseFunc>(GetProcAddress(m_hIgcl, "ctlClose"));
	m_pfnEnumerateDevices = reinterpret_cast<ctlEnumerateDevicesFunc>(
		GetProcAddress(m_hIgcl, "ctlEnumerateDevices"));
	m_pfnPowerTelemetryGet = reinterpret_cast<ctlPowerTelemetryGetFunc>(
		GetProcAddress(m_hIgcl, "ctlPowerTelemetryGet"));

	if (!m_pfnInit || !m_pfnClose || !m_pfnEnumerateDevices || !m_pfnPowerTelemetryGet) {
		if (logger) logger->warn("IGCL: required functions not found");
		FreeLibrary(m_hIgcl);
		m_hIgcl = NULL;
		return false;
	}

	ctl_init_args_t initArgs = {};
	initArgs.Size = sizeof(ctl_init_args_t);
	initArgs.Version = 0;
	initArgs.AppVersion = CTL_IMPL_VERSION;
	initArgs.flags = 1; // CTL_INIT_FLAG_USE_LEVEL_ZERO

	ctl_result_t res = m_pfnInit(&initArgs, &m_apiHandle);
	if (res != CTL_RESULT_SUCCESS || !m_apiHandle) {
		if (logger) logger->warn("IGCL: ctlInit failed: 0x{:08X}", res);
		FreeLibrary(m_hIgcl);
		m_hIgcl = NULL;
		return false;
	}

	uint32_t deviceCount = 0;
	res = m_pfnEnumerateDevices(m_apiHandle, &deviceCount, nullptr);
	if (res != CTL_RESULT_SUCCESS || deviceCount == 0) {
		if (logger) logger->warn("IGCL: no devices found");
		m_pfnClose(m_apiHandle);
		m_apiHandle = nullptr;
		FreeLibrary(m_hIgcl);
		m_hIgcl = NULL;
		return false;
	}

	ctl_device_adapter_handle_t devices[16] = {};
	uint32_t queryCount = (deviceCount < 16) ? deviceCount : 16;
	res = m_pfnEnumerateDevices(m_apiHandle, &queryCount, devices);
	if (res != CTL_RESULT_SUCCESS || queryCount == 0) {
		if (logger) logger->warn("IGCL: ctlEnumerateDevices failed: 0x{:08X}", res);
		m_pfnClose(m_apiHandle);
		m_apiHandle = nullptr;
		FreeLibrary(m_hIgcl);
		m_hIgcl = NULL;
		return false;
	}

	m_deviceHandle = devices[0];
	m_initialized = true;
	if (logger) logger->debug("IGCL: initialized (device count: {})", queryCount);
	return true;
}

void RDGpuVendorTelemetryIgclProvider::Shutdown()
{
	if (m_initialized && m_pfnClose && m_apiHandle) {
		m_pfnClose(m_apiHandle);
	}
	m_apiHandle = nullptr;
	m_deviceHandle = nullptr;
	m_initialized = false;
	m_hasPrevEnergy = false;
	m_hasPrevActivity = false;

	if (m_hIgcl) {
		FreeLibrary(m_hIgcl);
		m_hIgcl = NULL;
	}
}

double RDGpuVendorTelemetryIgclProvider::_ReadItemDouble(const ctl_oc_telemetry_item_t& item)
{
	switch (item.type) {
	case CTL_DATA_TYPE_DOUBLE: return item.value.datadouble;
	case CTL_DATA_TYPE_FLOAT:  return static_cast<double>(item.value.datafloat);
	case CTL_DATA_TYPE_INT32:  return static_cast<double>(item.value.data32);
	case CTL_DATA_TYPE_UINT32: return static_cast<double>(item.value.datau32);
	case CTL_DATA_TYPE_INT64:  return static_cast<double>(item.value.data64);
	case CTL_DATA_TYPE_UINT64: return static_cast<double>(item.value.datau64);
	default: return 0.0;
	}
}

void RDGpuVendorTelemetryIgclProvider::CollectTelemetry(RDGpuVendorTelemetryData& data)
{
	data.isAvailable = m_initialized;
	data.temperatureCelsius = { false, 0.0 };
	data.coreClockMHz       = { false, 0.0 };
	data.memoryClockMHz     = { false, 0.0 };
	data.usagePercent       = { false, 0.0 };
	data.powerWatts         = { false, 0.0 };
	data.fanSpeedRPM        = { false, 0.0 };

	if (!m_initialized) return;

	ctl_power_telemetry_t telemetry = {};
	telemetry.Size = sizeof(ctl_power_telemetry_t);
	telemetry.Version = 1;

	ctl_result_t res = m_pfnPowerTelemetryGet(m_deviceHandle, &telemetry);
	if (res != CTL_RESULT_SUCCESS) return;

	if (telemetry.gpuCurrentTemperature.bSupported) {
		data.temperatureCelsius = { true, _ReadItemDouble(telemetry.gpuCurrentTemperature) };
	}

	if (telemetry.gpuCurrentClockFrequency.bSupported) {
		data.coreClockMHz = { true, _ReadItemDouble(telemetry.gpuCurrentClockFrequency) };
	}

	if (telemetry.vramCurrentClockFrequency.bSupported) {
		data.memoryClockMHz = { true, _ReadItemDouble(telemetry.vramCurrentClockFrequency) };
	}

	// Power: prefer totalCardEnergyCounter (board), fallback to gpuEnergyCounter (chip)
	{
		const ctl_oc_telemetry_item_t* pEnergy = nullptr;
		if (telemetry.totalCardEnergyCounter.bSupported)
			pEnergy = &telemetry.totalCardEnergyCounter;
		else if (telemetry.gpuEnergyCounter.bSupported)
			pEnergy = &telemetry.gpuEnergyCounter;

		if (pEnergy && telemetry.timeStamp.bSupported) {
			if (m_hasPrevEnergy) {
				double energyNow = _ReadItemDouble(*pEnergy);
				double energyPrev = _ReadItemDouble(m_prevEnergyCounter);
				double timeNow = _ReadItemDouble(telemetry.timeStamp);
				double timePrev = _ReadItemDouble(m_prevEnergyTimestamp);
				double dt = timeNow - timePrev;
				if (dt > 0.0) {
					data.powerWatts = { true, (energyNow - energyPrev) / dt };
				}
			}
			m_prevEnergyCounter = *pEnergy;
			m_prevEnergyTimestamp = telemetry.timeStamp;
			m_hasPrevEnergy = true;
		}
	}

	if (telemetry.fanSpeed[0].bSupported) {
		data.fanSpeedRPM = { true, _ReadItemDouble(telemetry.fanSpeed[0]) };
	}

	// GPU usage from globalActivityCounter (delta/time, same pattern as energy)
	if (telemetry.globalActivityCounter.bSupported && telemetry.timeStamp.bSupported) {
		if (m_hasPrevActivity) {
			double actNow = _ReadItemDouble(telemetry.globalActivityCounter);
			double actPrev = _ReadItemDouble(m_prevGlobalActivity);
			double timeNow = _ReadItemDouble(telemetry.timeStamp);
			double timePrev = _ReadItemDouble(m_prevActivityTimestamp);
			double dt = timeNow - timePrev;
			if (dt > 0.0) {
				double usage = ((actNow - actPrev) / dt) * 100.0;
				if (usage >= 0.0 && usage <= 100.0) {
					data.usagePercent = { true, usage };
				}
			}
		}
		m_prevGlobalActivity = telemetry.globalActivityCounter;
		m_prevActivityTimestamp = telemetry.timeStamp;
		m_hasPrevActivity = true;
	}

	if (telemetry.gpuVoltage.bSupported) {
		data.voltageVolts = { true, _ReadItemDouble(telemetry.gpuVoltage) };
	}

	if (telemetry.vramCurrentTemperature.bSupported) {
		data.vramTemperatureCelsius = { true, _ReadItemDouble(telemetry.vramCurrentTemperature) };
	}
}

#endif // _M_AMD64
