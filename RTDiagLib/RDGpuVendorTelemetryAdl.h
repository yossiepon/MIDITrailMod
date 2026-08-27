//******************************************************************************
//
// RTDiagLib / RDGpuVendorTelemetryAdlProvider
//
// AMD GPU vendor telemetry provider via ADL (legacy fallback for ADLX).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
// API usage patterns derived from:
//   LibreHardwareMonitor (MIT) - https://github.com/LibreHardwareMonitor/LibreHardwareMonitor
//
//******************************************************************************

#pragma once

#include "IRDGpuVendorTelemetryProvider.h"
#include <Windows.h>
#include <cstdint>

class RDGpuVendorTelemetryAdlProvider : public IRDGpuVendorTelemetryProvider
{
public:
	RDGpuVendorTelemetryAdlProvider();
	~RDGpuVendorTelemetryAdlProvider() override;

	bool Initialize() override;
	void Shutdown() override;
	void CollectTelemetry(RDGpuVendorTelemetryData& data) override;

private:
	typedef int ADL_STATUS;
	typedef void* (__stdcall* ADL_MAIN_MALLOC_CALLBACK)(int);

	struct ADLPMActivity {
		int iSize;
		int iEngineClock;
		int iMemoryClock;
		int iVddc;
		int iActivityPercent;
		int iCurrentPerformanceLevel;
		int iCurrentBusSpeed;
		int iCurrentBusLanes;
		int iMaximumBusLanes;
		int iReserved;
	};

	struct ADLTemperature {
		int iSize;
		int iTemperature;
	};

	struct ADLFanSpeedValue {
		int iSize;
		int iSpeedType;
		int iFanSpeed;
		int iFlags;
	};

	enum ADLODNCurrentPowerType {
		ODN_GPU_TOTAL_POWER = 0,
		ODN_GPU_PPT_POWER = 1,
		ODN_GPU_SOCKET_POWER = 2,
		ODN_GPU_CHIP_POWER = 3
	};

	static const int ADL_OK = 0;
	static const int ADL_DL_FANCTRL_SPEED_TYPE_RPM = 2;

	typedef ADL_STATUS(__cdecl* ADL2_Main_Control_Create_Fn)(ADL_MAIN_MALLOC_CALLBACK, int, void**);
	typedef ADL_STATUS(__cdecl* ADL2_Main_Control_Destroy_Fn)(void*);
	typedef ADL_STATUS(__cdecl* ADL2_Adapter_NumberOfAdapters_Get_Fn)(void*, int*);
	typedef ADL_STATUS(__cdecl* ADL2_Overdrive5_Temperature_Get_Fn)(void*, int, int, ADLTemperature*);
	typedef ADL_STATUS(__cdecl* ADL2_Overdrive5_CurrentActivity_Get_Fn)(void*, int, ADLPMActivity*);
	typedef ADL_STATUS(__cdecl* ADL2_Overdrive5_FanSpeed_Get_Fn)(void*, int, int, ADLFanSpeedValue*);
	typedef ADL_STATUS(__cdecl* ADL2_Overdrive6_CurrentPower_Get_Fn)(void*, int, ADLODNCurrentPowerType, int*);

	bool _MethodExists(const char* name);

	HMODULE m_hAdl;
	bool    m_initialized;
	void*   m_context;
	int     m_adapterIndex;

	ADL2_Main_Control_Create_Fn        m_pfnCreate;
	ADL2_Main_Control_Destroy_Fn       m_pfnDestroy;
	ADL2_Overdrive5_Temperature_Get_Fn m_pfnGetTemp;
	ADL2_Overdrive5_CurrentActivity_Get_Fn m_pfnGetActivity;
	ADL2_Overdrive5_FanSpeed_Get_Fn    m_pfnGetFanSpeed;
	ADL2_Overdrive6_CurrentPower_Get_Fn m_pfnGetPower;
};
