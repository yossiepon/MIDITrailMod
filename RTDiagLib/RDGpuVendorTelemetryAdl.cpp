//******************************************************************************
//
// RTDiagLib / RDGpuVendorTelemetryAdlProvider
//
// AMD GPU vendor telemetry provider via ADL (legacy fallback for ADLX).
// Provides OD5 basic metrics + OD6 power for GPUs/drivers where ADLX
// is unavailable.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "RDGpuVendorTelemetryAdl.h"
#include <spdlog/spdlog.h>

static void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
	return malloc(iSize);
}

RDGpuVendorTelemetryAdlProvider::RDGpuVendorTelemetryAdlProvider()
	: m_hAdl(NULL)
	, m_initialized(false)
	, m_context(nullptr)
	, m_adapterIndex(0)
	, m_pfnCreate(nullptr)
	, m_pfnDestroy(nullptr)
	, m_pfnGetTemp(nullptr)
	, m_pfnGetActivity(nullptr)
	, m_pfnGetFanSpeed(nullptr)
	, m_pfnGetPower(nullptr)
{
}

RDGpuVendorTelemetryAdlProvider::~RDGpuVendorTelemetryAdlProvider()
{
	Shutdown();
}

bool RDGpuVendorTelemetryAdlProvider::_MethodExists(const char* name)
{
	if (!m_hAdl) return false;
	return GetProcAddress(m_hAdl, name) != nullptr;
}

bool RDGpuVendorTelemetryAdlProvider::Initialize()
{
	auto logger = spdlog::get("RD");

	m_hAdl = LoadLibraryA("atiadlxx.dll");
	if (!m_hAdl) {
		m_hAdl = LoadLibraryA("atiadlxy.dll");
	}
	if (!m_hAdl) {
		if (logger) logger->debug("ADL: DLL not found");
		return false;
	}

	m_pfnCreate = reinterpret_cast<ADL2_Main_Control_Create_Fn>(
		GetProcAddress(m_hAdl, "ADL2_Main_Control_Create"));
	m_pfnDestroy = reinterpret_cast<ADL2_Main_Control_Destroy_Fn>(
		GetProcAddress(m_hAdl, "ADL2_Main_Control_Destroy"));

	if (!m_pfnCreate || !m_pfnDestroy) {
		if (logger) logger->debug("ADL: required functions not found");
		FreeLibrary(m_hAdl);
		m_hAdl = NULL;
		return false;
	}

	ADL_STATUS status = m_pfnCreate(ADL_Main_Memory_Alloc, 1, &m_context);
	if (status != ADL_OK || !m_context) {
		if (logger) logger->debug("ADL: Main_Control_Create failed: {}", status);
		FreeLibrary(m_hAdl);
		m_hAdl = NULL;
		return false;
	}

	if (_MethodExists("ADL2_Overdrive5_Temperature_Get")) {
		m_pfnGetTemp = reinterpret_cast<ADL2_Overdrive5_Temperature_Get_Fn>(
			GetProcAddress(m_hAdl, "ADL2_Overdrive5_Temperature_Get"));
	}
	if (_MethodExists("ADL2_Overdrive5_CurrentActivity_Get")) {
		m_pfnGetActivity = reinterpret_cast<ADL2_Overdrive5_CurrentActivity_Get_Fn>(
			GetProcAddress(m_hAdl, "ADL2_Overdrive5_CurrentActivity_Get"));
	}
	if (_MethodExists("ADL2_Overdrive5_FanSpeed_Get")) {
		m_pfnGetFanSpeed = reinterpret_cast<ADL2_Overdrive5_FanSpeed_Get_Fn>(
			GetProcAddress(m_hAdl, "ADL2_Overdrive5_FanSpeed_Get"));
	}
	if (_MethodExists("ADL2_Overdrive6_CurrentPower_Get")) {
		m_pfnGetPower = reinterpret_cast<ADL2_Overdrive6_CurrentPower_Get_Fn>(
			GetProcAddress(m_hAdl, "ADL2_Overdrive6_CurrentPower_Get"));
	}

	m_initialized = true;
	if (logger) logger->debug("ADL: initialized (legacy fallback)");
	return true;
}

void RDGpuVendorTelemetryAdlProvider::Shutdown()
{
	if (m_initialized && m_pfnDestroy && m_context) {
		m_pfnDestroy(m_context);
	}
	m_context = nullptr;
	m_initialized = false;

	if (m_hAdl) {
		FreeLibrary(m_hAdl);
		m_hAdl = NULL;
	}
}

void RDGpuVendorTelemetryAdlProvider::CollectTelemetry(RDGpuVendorTelemetryData& data)
{
	data.isAvailable = m_initialized;
	data.temperatureCelsius = { false, 0.0 };
	data.coreClockMHz       = { false, 0.0 };
	data.memoryClockMHz     = { false, 0.0 };
	data.usagePercent       = { false, 0.0 };
	data.powerWatts         = { false, 0.0 };
	data.fanSpeedRPM        = { false, 0.0 };

	if (!m_initialized) return;

	if (m_pfnGetTemp) {
		ADLTemperature temp = {};
		temp.iSize = sizeof(ADLTemperature);
		if (m_pfnGetTemp(m_context, m_adapterIndex, 0, &temp) == ADL_OK) {
			data.temperatureCelsius = { true, static_cast<double>(temp.iTemperature) / 1000.0 };
		}
	}

	if (m_pfnGetActivity) {
		ADLPMActivity activity = {};
		activity.iSize = sizeof(ADLPMActivity);
		if (m_pfnGetActivity(m_context, m_adapterIndex, &activity) == ADL_OK) {
			if (activity.iEngineClock > 0) {
				data.coreClockMHz = { true, static_cast<double>(activity.iEngineClock) / 100.0 };
			}
			if (activity.iMemoryClock > 0) {
				data.memoryClockMHz = { true, static_cast<double>(activity.iMemoryClock) / 100.0 };
			}
			if (activity.iVddc > 0) {
				data.voltageVolts = { true, static_cast<double>(activity.iVddc) / 1000.0 };
			}
			data.usagePercent = { true, static_cast<double>(activity.iActivityPercent > 100 ? 100 : activity.iActivityPercent) };
		}
	}

	if (m_pfnGetPower) {
		int powerOf8 = 0;
		if (m_pfnGetPower(m_context, m_adapterIndex, ODN_GPU_TOTAL_POWER, &powerOf8) == ADL_OK) {
			data.powerWatts = { true, static_cast<double>(powerOf8 >> 8) };
		}
	}

	if (m_pfnGetFanSpeed) {
		ADLFanSpeedValue fanSpeed = {};
		fanSpeed.iSize = sizeof(ADLFanSpeedValue);
		fanSpeed.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_RPM;
		if (m_pfnGetFanSpeed(m_context, m_adapterIndex, 0, &fanSpeed) == ADL_OK) {
			data.fanSpeedRPM = { true, static_cast<double>(fanSpeed.iFanSpeed) };
		}
	}
}
