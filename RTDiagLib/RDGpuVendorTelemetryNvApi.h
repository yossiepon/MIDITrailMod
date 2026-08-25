//******************************************************************************
//
// RTDiagLib / RDGpuVendorTelemetryNvApiProvider
//
// NVIDIA GPU vendor telemetry provider via NVAPI and NVML.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
// Non-official NVAPI struct layouts and fallback patterns referenced from:
//   LibreHardwareMonitor (MIT) - https://github.com/LibreHardwareMonitor/LibreHardwareMonitor
//   NvAPIWrapper (MIT) - https://github.com/falahati/NvAPIWrapper
//
//******************************************************************************

#pragma once

#include "IRDGpuVendorTelemetryProvider.h"
#include <Windows.h>

class RDGpuVendorTelemetryNvApiProvider : public IRDGpuVendorTelemetryProvider
{
public:
	RDGpuVendorTelemetryNvApiProvider();
	~RDGpuVendorTelemetryNvApiProvider() override;

	bool Initialize() override;
	void Shutdown() override;
	void CollectTelemetry(RDGpuVendorTelemetryData& data) override;

private:
	bool _InitNvml();

	typedef int NvAPI_Status;
	typedef unsigned int NvU32;
	typedef int NvS32;

	struct NvPhysicalGpuHandle__ { int unused; };
	typedef NvPhysicalGpuHandle__* NvPhysicalGpuHandle;

	static const NvU32 NVAPI_MAX_THERMAL_SENSORS = 3;
	static const NvU32 NVAPI_MAX_CLOCKS = 32;
	static const NvU32 NVAPI_MAX_UTILIZATIONS = 8;

	struct NV_GPU_THERMAL_SETTINGS {
		NvU32 version;
		NvU32 count;
		struct Sensor {
			int controller;
			NvS32 defaultMinTemp;
			NvS32 defaultMaxTemp;
			NvS32 currentTemp;
			int target;
		} sensor[3];
	};

	struct NV_GPU_CLOCK_FREQUENCIES {
		NvU32 version;
		NvU32 ClockType : 4;
		NvU32 reserved : 20;
		NvU32 reserved1 : 8;
		struct Domain {
			NvU32 bIsPresent : 1;
			NvU32 reservedBits : 31;
			NvU32 frequency;
		} domain[32];
	};

	struct NV_GPU_DYNAMIC_PSTATES_INFO_EX {
		NvU32 version;
		NvU32 flags;
		struct Utilization {
			NvU32 bIsPresent : 1;
			NvU32 percentage;
		} utilization[8];
	};

	struct NV_POWER_TOPOLOGY_STATUS {
		NvU32 version;
		NvU32 count;
		struct Entry {
			NvU32 domain;
			NvU32 reserved;
			NvU32 power;
			NvU32 unknown;
		} entries[4];
	};


	typedef void*(__cdecl* NvAPI_QueryInterfaceFunc)(NvU32 id);
	typedef NvAPI_Status(__cdecl* NvAPI_InitializeFunc)();
	typedef NvAPI_Status(__cdecl* NvAPI_UnloadFunc)();
	typedef NvAPI_Status(__cdecl* NvAPI_EnumPhysicalGPUsFunc)(NvPhysicalGpuHandle handles[64], NvU32* count);
	typedef NvAPI_Status(__cdecl* NvAPI_GPU_GetThermalSettingsFunc)(NvPhysicalGpuHandle handle, NvU32 sensorIndex, NV_GPU_THERMAL_SETTINGS* pSettings);
	typedef NvAPI_Status(__cdecl* NvAPI_GPU_GetAllClockFrequenciesFunc)(NvPhysicalGpuHandle handle, NV_GPU_CLOCK_FREQUENCIES* pFreqs);
	typedef NvAPI_Status(__cdecl* NvAPI_GPU_GetDynamicPstatesInfoExFunc)(NvPhysicalGpuHandle handle, NV_GPU_DYNAMIC_PSTATES_INFO_EX* pInfo);
	typedef NvAPI_Status(__cdecl* NvAPI_GPU_GetTachReadingFunc)(NvPhysicalGpuHandle handle, NvU32* pValue);
	struct NV_GPU_FAN_COOLERS_STATUS {
		NvU32 version;
		NvU32 count;
		NvU32 reserved[8];
		struct Item {
			NvU32 coolerId;
			NvU32 currentRpm;
			NvU32 currentMinLevel;
			NvU32 currentMaxLevel;
			NvU32 currentLevel;
			NvU32 reserved[8];
		} items[32];
	};

	typedef NvAPI_Status(__cdecl* NvAPI_GPU_PowerTopologyGetStatusFunc)(NvPhysicalGpuHandle handle, NV_POWER_TOPOLOGY_STATUS* pStatus);
	typedef NvAPI_Status(__cdecl* NvAPI_GPU_ClientFanCoolersGetStatusFunc)(NvPhysicalGpuHandle handle, void* pStatus);


	typedef int(*NvmlInitFunc)();
	typedef int(*NvmlShutdownFunc)();
	typedef int(*NvmlDeviceGetHandleByIndexFunc)(unsigned int index, void** device);
	typedef int(*NvmlDeviceGetPowerUsageFunc)(void* device, unsigned int* power);

	HMODULE m_hNvApi;
	HMODULE m_hNvml;
	bool    m_initialized;
	NvU32   m_clockVersion;

	NvPhysicalGpuHandle m_gpuHandle;

	NvAPI_InitializeFunc                  m_pfnInitialize;
	NvAPI_UnloadFunc                      m_pfnUnload;
	NvAPI_EnumPhysicalGPUsFunc            m_pfnEnumPhysicalGPUs;
	NvAPI_GPU_GetThermalSettingsFunc       m_pfnGetThermalSettings;
	NvAPI_GPU_GetAllClockFrequenciesFunc   m_pfnGetAllClockFrequencies;
	NvAPI_GPU_GetDynamicPstatesInfoExFunc  m_pfnGetDynamicPstatesInfoEx;
	NvAPI_GPU_GetTachReadingFunc           m_pfnGetTachReading;
	NvAPI_GPU_PowerTopologyGetStatusFunc   m_pfnPowerTopologyGetStatus;
	NvAPI_GPU_ClientFanCoolersGetStatusFunc m_pfnFanCoolersGetStatus;


	NvmlInitFunc                   m_pfnNvmlInit;
	NvmlShutdownFunc               m_pfnNvmlShutdown;
	NvmlDeviceGetHandleByIndexFunc m_pfnNvmlDeviceGetHandleByIndex;
	NvmlDeviceGetPowerUsageFunc    m_pfnNvmlDeviceGetPowerUsage;
	void*                          m_nvmlDevice;
	bool                           m_nvmlInitialized;
};
