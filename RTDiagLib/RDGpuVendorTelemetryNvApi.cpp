//******************************************************************************
//
// RTDiagLib / RDGpuVendorTelemetryNvApiProvider
//
// NVIDIA GPU vendor telemetry provider via NVAPI and NVML.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
// Official NVAPI struct definitions derived from:
//   NVAPI SDK (MIT) - https://github.com/NVIDIA/nvapi
// Non-official NVAPI struct layouts and fallback patterns derived from:
//   LibreHardwareMonitor (MIT) - https://github.com/LibreHardwareMonitor/LibreHardwareMonitor
//   NvAPIWrapper (MIT) - https://github.com/falahati/NvAPIWrapper
//
//******************************************************************************

#include "stdafx.h"
#include "RDGpuVendorTelemetryNvApi.h"
#include <spdlog/spdlog.h>

#define MAKE_NVAPI_VERSION(typeName, ver) (unsigned int)(sizeof(typeName) | ((ver) << 16))

static const unsigned int NVAPI_QI_INITIALIZE              = 0x0150e828;
static const unsigned int NVAPI_QI_UNLOAD                  = 0xd22bdd7e;
static const unsigned int NVAPI_QI_ENUM_PHYSICAL_GPUS      = 0xe5ac921f;
static const unsigned int NVAPI_QI_GET_THERMAL_SETTINGS    = 0xe3640a56;
static const unsigned int NVAPI_QI_GET_ALL_CLOCK_FREQS     = 0xdcb616c3;
static const unsigned int NVAPI_QI_GET_DYNAMIC_PSTATES_EX  = 0x60ded2ed;
static const unsigned int NVAPI_QI_GET_THERMAL_SENSORS     = 0x65fe3aad;
static const unsigned int NVAPI_QI_GET_USAGES              = 0x189a1fdf;
static const unsigned int NVAPI_QI_GET_TACH_READING        = 0x5f608315;
static const unsigned int NVAPI_QI_POWER_TOPOLOGY_STATUS   = 0xedcf624e;
static const unsigned int NVAPI_QI_FAN_COOLERS_GET_STATUS  = 0x35aed5e8;

RDGpuVendorTelemetryNvApiProvider::RDGpuVendorTelemetryNvApiProvider()
	: m_hNvApi(NULL)
	, m_hNvml(NULL)
	, m_initialized(false)
	, m_clockVersion(3)
	, m_thermalSensorsMask(0)
	, m_gpuHandle(nullptr)
	, m_pfnInitialize(nullptr)
	, m_pfnUnload(nullptr)
	, m_pfnEnumPhysicalGPUs(nullptr)
	, m_pfnGetThermalSettings(nullptr)
	, m_pfnGetAllClockFrequencies(nullptr)
	, m_pfnGetDynamicPstatesInfoEx(nullptr)
	, m_pfnGetThermalSensors(nullptr)
	, m_pfnGetUsages(nullptr)
	, m_pfnGetTachReading(nullptr)
	, m_pfnPowerTopologyGetStatus(nullptr)
	, m_pfnFanCoolersGetStatus(nullptr)

	, m_pfnNvmlInit(nullptr)
	, m_pfnNvmlShutdown(nullptr)
	, m_pfnNvmlDeviceGetHandleByIndex(nullptr)
	, m_pfnNvmlDeviceGetPowerUsage(nullptr)
	, m_nvmlDevice(nullptr)
	, m_nvmlInitialized(false)
{
}

RDGpuVendorTelemetryNvApiProvider::~RDGpuVendorTelemetryNvApiProvider()
{
	Shutdown();
}

bool RDGpuVendorTelemetryNvApiProvider::Initialize()
{
	auto logger = spdlog::get("RD");

#ifdef _M_AMD64
	m_hNvApi = LoadLibraryA("nvapi64.dll");
#else
	m_hNvApi = LoadLibraryA("nvapi.dll");
#endif

	if (!m_hNvApi) {
		if (logger) logger->debug("NvApi: DLL not found");
		return false;
	}

	auto queryInterface = reinterpret_cast<NvAPI_QueryInterfaceFunc>(
		GetProcAddress(m_hNvApi, "nvapi_QueryInterface"));
	if (!queryInterface) {
		if (logger) logger->warn("NvApi: nvapi_QueryInterface not found");
		FreeLibrary(m_hNvApi);
		m_hNvApi = NULL;
		return false;
	}

	m_pfnInitialize           = reinterpret_cast<NvAPI_InitializeFunc>(queryInterface(NVAPI_QI_INITIALIZE));
	m_pfnUnload               = reinterpret_cast<NvAPI_UnloadFunc>(queryInterface(NVAPI_QI_UNLOAD));
	m_pfnEnumPhysicalGPUs     = reinterpret_cast<NvAPI_EnumPhysicalGPUsFunc>(queryInterface(NVAPI_QI_ENUM_PHYSICAL_GPUS));
	m_pfnGetThermalSettings   = reinterpret_cast<NvAPI_GPU_GetThermalSettingsFunc>(queryInterface(NVAPI_QI_GET_THERMAL_SETTINGS));
	m_pfnGetAllClockFrequencies = reinterpret_cast<NvAPI_GPU_GetAllClockFrequenciesFunc>(queryInterface(NVAPI_QI_GET_ALL_CLOCK_FREQS));
	m_pfnGetDynamicPstatesInfoEx = reinterpret_cast<NvAPI_GPU_GetDynamicPstatesInfoExFunc>(queryInterface(NVAPI_QI_GET_DYNAMIC_PSTATES_EX));
	m_pfnGetThermalSensors    = reinterpret_cast<NvAPI_GPU_GetThermalSensorsFunc>(queryInterface(NVAPI_QI_GET_THERMAL_SENSORS));
	m_pfnGetUsages            = reinterpret_cast<NvAPI_GPU_GetUsagesFunc>(queryInterface(NVAPI_QI_GET_USAGES));
	m_pfnGetTachReading       = reinterpret_cast<NvAPI_GPU_GetTachReadingFunc>(queryInterface(NVAPI_QI_GET_TACH_READING));
	m_pfnPowerTopologyGetStatus = reinterpret_cast<NvAPI_GPU_PowerTopologyGetStatusFunc>(queryInterface(NVAPI_QI_POWER_TOPOLOGY_STATUS));
	m_pfnFanCoolersGetStatus = reinterpret_cast<NvAPI_GPU_ClientFanCoolersGetStatusFunc>(queryInterface(NVAPI_QI_FAN_COOLERS_GET_STATUS));


	if (!m_pfnInitialize || !m_pfnEnumPhysicalGPUs) {
		if (logger) logger->warn("NvApi: required functions not available");
		FreeLibrary(m_hNvApi);
		m_hNvApi = NULL;
		return false;
	}

	NvAPI_Status status = m_pfnInitialize();
	if (status != 0) {
		if (logger) logger->warn("NvApi: Initialize failed: {}", status);
		FreeLibrary(m_hNvApi);
		m_hNvApi = NULL;
		return false;
	}

	NvPhysicalGpuHandle handles[64] = {};
	NvU32 gpuCount = 0;
	status = m_pfnEnumPhysicalGPUs(handles, &gpuCount);
	if (status != 0 || gpuCount == 0) {
		if (logger) logger->warn("NvApi: EnumPhysicalGPUs failed or no GPUs: {}", status);
		if (m_pfnUnload) m_pfnUnload();
		FreeLibrary(m_hNvApi);
		m_hNvApi = NULL;
		return false;
	}

	m_gpuHandle = handles[0];

	if (m_pfnGetAllClockFrequencies) {
		for (NvU32 ver = 1; ver <= 3; ver++) {
			NV_GPU_CLOCK_FREQUENCIES freqs = {};
			freqs.version = MAKE_NVAPI_VERSION(NV_GPU_CLOCK_FREQUENCIES, ver);
			if (m_pfnGetAllClockFrequencies(m_gpuHandle, &freqs) == 0) {
				m_clockVersion = ver;
				break;
			}
		}
	}

	if (m_pfnGetThermalSensors) {
		for (NvU32 bit = 0; bit < 32; bit++) {
			NV_GPU_THERMAL_SENSORS sensors = {};
			sensors.version = MAKE_NVAPI_VERSION(NV_GPU_THERMAL_SENSORS, 2);
			sensors.mask = 1u << bit;
			if (m_pfnGetThermalSensors(m_gpuHandle, &sensors) != 0) {
				m_thermalSensorsMask = (1u << bit) - 1;
				break;
			}
		}
	}

	_InitNvml();

	m_initialized = true;
	if (logger) logger->debug("NvApi: initialized (GPU count: {})", gpuCount);
	return true;
}

bool RDGpuVendorTelemetryNvApiProvider::_InitNvml()
{
	auto logger = spdlog::get("RD");

	m_hNvml = LoadLibraryA("nvml.dll");
	if (!m_hNvml) {
		char nvsmiPath[MAX_PATH] = {};
		if (GetEnvironmentVariableA("ProgramW6432", nvsmiPath, MAX_PATH) > 0) {
			strncat_s(nvsmiPath, "\\NVIDIA Corporation\\NVSMI\\nvml.dll", MAX_PATH - strlen(nvsmiPath) - 1);
			m_hNvml = LoadLibraryA(nvsmiPath);
		}
	}
	if (!m_hNvml) {
		if (logger) logger->debug("NvApi: nvml.dll not found (power fallback unavailable)");
		return false;
	}

	m_pfnNvmlInit = reinterpret_cast<NvmlInitFunc>(GetProcAddress(m_hNvml, "nvmlInit_v2"));
	if (!m_pfnNvmlInit) {
		m_pfnNvmlInit = reinterpret_cast<NvmlInitFunc>(GetProcAddress(m_hNvml, "nvmlInit"));
	}
	m_pfnNvmlShutdown = reinterpret_cast<NvmlShutdownFunc>(GetProcAddress(m_hNvml, "nvmlShutdown"));
	m_pfnNvmlDeviceGetHandleByIndex = reinterpret_cast<NvmlDeviceGetHandleByIndexFunc>(
		GetProcAddress(m_hNvml, "nvmlDeviceGetHandleByIndex_v2"));
	if (!m_pfnNvmlDeviceGetHandleByIndex) {
		m_pfnNvmlDeviceGetHandleByIndex = reinterpret_cast<NvmlDeviceGetHandleByIndexFunc>(
			GetProcAddress(m_hNvml, "nvmlDeviceGetHandleByIndex"));
	}
	m_pfnNvmlDeviceGetPowerUsage = reinterpret_cast<NvmlDeviceGetPowerUsageFunc>(
		GetProcAddress(m_hNvml, "nvmlDeviceGetPowerUsage"));

	if (!m_pfnNvmlInit || !m_pfnNvmlDeviceGetHandleByIndex || !m_pfnNvmlDeviceGetPowerUsage) {
		if (logger) logger->debug("NvApi: NVML functions not available");
		FreeLibrary(m_hNvml);
		m_hNvml = NULL;
		return false;
	}

	int ret = m_pfnNvmlInit();
	if (ret != 0) {
		if (logger) logger->debug("NvApi: nvmlInit failed: {}", ret);
		FreeLibrary(m_hNvml);
		m_hNvml = NULL;
		return false;
	}

	ret = m_pfnNvmlDeviceGetHandleByIndex(0, &m_nvmlDevice);
	if (ret != 0) {
		if (logger) logger->debug("NvApi: nvmlDeviceGetHandleByIndex failed: {}", ret);
		if (m_pfnNvmlShutdown) m_pfnNvmlShutdown();
		FreeLibrary(m_hNvml);
		m_hNvml = NULL;
		return false;
	}

	m_nvmlInitialized = true;
	if (logger) logger->debug("NvApi: NVML initialized (power fallback ready)");
	return true;
}

void RDGpuVendorTelemetryNvApiProvider::Shutdown()
{
	if (m_nvmlInitialized) {
		if (m_pfnNvmlShutdown) m_pfnNvmlShutdown();
		m_nvmlInitialized = false;
		m_nvmlDevice = nullptr;
	}
	if (m_hNvml) {
		FreeLibrary(m_hNvml);
		m_hNvml = NULL;
	}

	if (m_initialized) {
		if (m_pfnUnload) m_pfnUnload();
		m_initialized = false;
	}
	if (m_hNvApi) {
		FreeLibrary(m_hNvApi);
		m_hNvApi = NULL;
	}
	m_gpuHandle = nullptr;
}

void RDGpuVendorTelemetryNvApiProvider::CollectTelemetry(RDGpuVendorTelemetryData& data)
{
	data.isAvailable = m_initialized;
	data.temperatureCelsius = { false, 0.0 };
	data.coreClockMHz       = { false, 0.0 };
	data.memoryClockMHz     = { false, 0.0 };
	data.usagePercent       = { false, 0.0 };
	data.powerWatts         = { false, 0.0 };
	data.fanSpeedRPM        = { false, 0.0 };

	if (!m_initialized) return;

	if (m_pfnGetThermalSettings) {
		NV_GPU_THERMAL_SETTINGS settings = {};
		settings.version = MAKE_NVAPI_VERSION(NV_GPU_THERMAL_SETTINGS, 2);
		if (m_pfnGetThermalSettings(m_gpuHandle, 0, &settings) == 0 && settings.count > 0) {
			data.temperatureCelsius = { true, static_cast<double>(settings.sensor[0].currentTemp) };
		}
	}

	if (m_pfnGetThermalSensors && m_thermalSensorsMask > 0) {
		NV_GPU_THERMAL_SENSORS sensors = {};
		sensors.version = MAKE_NVAPI_VERSION(NV_GPU_THERMAL_SENSORS, 2);
		sensors.mask = m_thermalSensorsMask;
		if (m_pfnGetThermalSensors(m_gpuHandle, &sensors) == 0) {
			double hotspot = static_cast<double>(sensors.temperatures[1]) / 256.0;
			if (hotspot > 0.0 && hotspot < 256.0) {
				data.hotspotTemperatureCelsius = { true, hotspot };
			}
		}
	}

	if (m_pfnGetAllClockFrequencies) {
		NV_GPU_CLOCK_FREQUENCIES freqs = {};
		freqs.version = MAKE_NVAPI_VERSION(NV_GPU_CLOCK_FREQUENCIES, m_clockVersion);
		freqs.ClockType = 0;
		if (m_pfnGetAllClockFrequencies(m_gpuHandle, &freqs) == 0) {
			if (freqs.domain[0].bIsPresent) {
				data.coreClockMHz = { true, static_cast<double>(freqs.domain[0].frequency) / 1000.0 };
			}
			if (freqs.domain[4].bIsPresent) {
				data.memoryClockMHz = { true, static_cast<double>(freqs.domain[4].frequency) / 1000.0 };
			}
		}
	}

	if (m_pfnGetDynamicPstatesInfoEx) {
		NV_GPU_DYNAMIC_PSTATES_INFO_EX pstates = {};
		pstates.version = MAKE_NVAPI_VERSION(NV_GPU_DYNAMIC_PSTATES_INFO_EX, 1);
		if (m_pfnGetDynamicPstatesInfoEx(m_gpuHandle, &pstates) == 0) {
			if (pstates.utilization[0].bIsPresent) {
				data.usagePercent = { true, static_cast<double>(pstates.utilization[0].percentage) };
			}
		}
	}
	if (!data.usagePercent.supported && m_pfnGetUsages) {
		NV_GPU_USAGES usages = {};
		usages.version = MAKE_NVAPI_VERSION(NV_GPU_USAGES, 1);
		if (m_pfnGetUsages(m_gpuHandle, &usages) == 0) {
			if (usages.entries[0].isPresent) {
				data.usagePercent = { true, static_cast<double>(usages.entries[0].percentage) };
			}
		}
	}

	if (m_nvmlInitialized && m_pfnNvmlDeviceGetPowerUsage) {
		unsigned int powerMw = 0;
		if (m_pfnNvmlDeviceGetPowerUsage(m_nvmlDevice, &powerMw) == 0) {
			data.powerWatts = { true, static_cast<double>(powerMw) / 1000.0 };
		}
	} else if (m_pfnPowerTopologyGetStatus) {
		NV_POWER_TOPOLOGY_STATUS powerStatus = {};
		powerStatus.version = MAKE_NVAPI_VERSION(NV_POWER_TOPOLOGY_STATUS, 1);
		if (m_pfnPowerTopologyGetStatus(m_gpuHandle, &powerStatus) == 0 && powerStatus.count > 0) {
			NvU32 maxPower = 0;
			for (NvU32 i = 0; i < powerStatus.count && i < 4; i++) {
				if (powerStatus.entries[i].powerUsage > maxPower)
					maxPower = powerStatus.entries[i].powerUsage;
			}
			data.powerWatts = { true, static_cast<double>(maxPower) / 1000.0 };
		}
	}

	{
		bool fanResolved = false;
		if (m_pfnFanCoolersGetStatus) {
			NV_GPU_FAN_COOLERS_STATUS fanStatus = {};
			fanStatus.version = MAKE_NVAPI_VERSION(NV_GPU_FAN_COOLERS_STATUS, 1);
			if (m_pfnFanCoolersGetStatus(m_gpuHandle, &fanStatus) == 0 && fanStatus.count > 0) {
				data.fanSpeedRPM = { true, static_cast<double>(fanStatus.items[0].currentRpm) };
				fanResolved = true;
			}
		}
		if (!fanResolved && m_pfnGetTachReading) {
			NvU32 tachRPM = 0;
			if (m_pfnGetTachReading(m_gpuHandle, &tachRPM) == 0) {
				data.fanSpeedRPM = { true, static_cast<double>(tachRPM) };
			}
		}
	}
}
