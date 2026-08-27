//******************************************************************************
//
// RTDiagLib / RDGpuVendorTelemetryAdlx
//
// AMD GPU vendor telemetry provider via ADLX.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
// Interface definitions derived from:
//   ADLX SDK (AMD) - https://github.com/GPUOpen-LibrariesAndSDKs/ADLX
//
//******************************************************************************

#pragma once

#include "IRDGpuVendorTelemetryProvider.h"
#include <Windows.h>
#include <cstdint>

// ---------------------------------------------------------------------------
// ADLX inline definitions (minimal subset for GPU telemetry)
// ---------------------------------------------------------------------------

typedef int32_t   adlx_int;
typedef uint32_t  adlx_uint;
typedef int64_t   adlx_int64;
typedef uint64_t  adlx_uint64;
typedef double    adlx_double;
typedef long      adlx_long;
typedef bool      adlx_bool;

typedef enum
{
	ADLX_OK = 0,
	ADLX_ALREADY_ENABLED,
	ADLX_ALREADY_INITIALIZED,
	ADLX_FAIL,
	ADLX_INVALID_ARGS,
	ADLX_BAD_VER,
	ADLX_UNKNOWN_INTERFACE,
	ADLX_TERMINATED,
	ADLX_ADL_INIT_ERROR,
	ADLX_NOT_FOUND,
	ADLX_INVALID_OBJECT,
	ADLX_ORPHAN_OBJECTS,
	ADLX_NOT_SUPPORTED,
	ADLX_PENDING_OPERATION,
	ADLX_GPU_INACTIVE,
	ADLX_GPU_IN_USE,
	ADLX_TIMEOUT_OPERATION,
	ADLX_NOT_ACTIVE,
	ADLX_RESET_NEEDED
} ADLX_RESULT;

#define ADLX_SUCCEEDED(x) \
	(ADLX_OK == (x) || ADLX_ALREADY_ENABLED == (x) || ADLX_ALREADY_INITIALIZED == (x))

#define ADLX_FULL_VERSION \
	((adlx_uint64)1 << 48 | (adlx_uint64)5 << 32 | (adlx_uint64)0 << 16 | (adlx_uint64)124)

#define ADLX_INIT_FUNCTION_NAME      "ADLXInitialize"
#define ADLX_TERMINATE_FUNCTION_NAME "ADLXTerminate"

namespace adlx {

// --- Base interface ---

class __declspec(novtable) IADLXInterface
{
public:
	virtual adlx_long __stdcall Acquire() = 0;
	virtual adlx_long __stdcall Release() = 0;
	virtual ADLX_RESULT __stdcall QueryInterface(const wchar_t* interfaceId, void** ppInterface) = 0;
};

// --- GPU ---

class __declspec(novtable) IADLXGPU : public IADLXInterface
{
public:
	virtual ADLX_RESULT __stdcall VendorId(const char** vendorId) = 0;
	virtual ADLX_RESULT __stdcall ASICFamilyType(void* pType) = 0;
	virtual ADLX_RESULT __stdcall Type(void* pType) = 0;
	virtual ADLX_RESULT __stdcall IsExternal(adlx_bool* isExternal) = 0;
	virtual ADLX_RESULT __stdcall Name(const char** name) = 0;
	virtual ADLX_RESULT __stdcall DriverPath(const char** driverPath) = 0;
	virtual ADLX_RESULT __stdcall PNPString(const char** pnpString) = 0;
	virtual ADLX_RESULT __stdcall HasDesktops(adlx_bool* hasDesktops) = 0;
	virtual ADLX_RESULT __stdcall TotalVRAM(adlx_uint* vramMB) = 0;
	virtual ADLX_RESULT __stdcall VRAMType(const char** type) = 0;
	virtual ADLX_RESULT __stdcall BIOSInfo(const char** partNumber, const char** version, const char** date) = 0;
	virtual ADLX_RESULT __stdcall DeviceId(const char** deviceId) = 0;
	virtual ADLX_RESULT __stdcall RevisionId(const char** revisionId) = 0;
	virtual ADLX_RESULT __stdcall SubSystemId(const char** subSystemId) = 0;
	virtual ADLX_RESULT __stdcall SubSystemVendorId(const char** subSystemVendorId) = 0;
	virtual ADLX_RESULT __stdcall UniqueId(adlx_int* uniqueId) = 0;
};

// --- GPU List ---

class __declspec(novtable) IADLXList : public IADLXInterface
{
public:
	virtual adlx_uint __stdcall Size() = 0;
	virtual adlx_bool __stdcall Empty() = 0;
	virtual adlx_uint __stdcall Begin() = 0;
	virtual adlx_uint __stdcall End() = 0;
	virtual ADLX_RESULT __stdcall At(const adlx_uint location, IADLXInterface** ppItem) = 0;
	virtual ADLX_RESULT __stdcall Clear() = 0;
	virtual ADLX_RESULT __stdcall Remove_Back() = 0;
	virtual ADLX_RESULT __stdcall Add_Back(IADLXInterface* pItem) = 0;
};

class __declspec(novtable) IADLXGPUList : public IADLXList
{
public:
	virtual ADLX_RESULT __stdcall At(const adlx_uint location, IADLXGPU** ppItem) = 0;
	virtual ADLX_RESULT __stdcall Add_Back(IADLXGPU* pItem) = 0;
};

// --- GPU Metrics ---

class __declspec(novtable) IADLXGPUMetricsSupport : public IADLXInterface
{
public:
	virtual ADLX_RESULT __stdcall IsSupportedGPUUsage(adlx_bool* supported) = 0;
	virtual ADLX_RESULT __stdcall IsSupportedGPUClockSpeed(adlx_bool* supported) = 0;
	virtual ADLX_RESULT __stdcall IsSupportedGPUVRAMClockSpeed(adlx_bool* supported) = 0;
	virtual ADLX_RESULT __stdcall IsSupportedGPUTemperature(adlx_bool* supported) = 0;
	virtual ADLX_RESULT __stdcall IsSupportedGPUHotspotTemperature(adlx_bool* supported) = 0;
	virtual ADLX_RESULT __stdcall IsSupportedGPUPower(adlx_bool* supported) = 0;
	virtual ADLX_RESULT __stdcall IsSupportedGPUTotalBoardPower(adlx_bool* supported) = 0;
	virtual ADLX_RESULT __stdcall IsSupportedGPUFanSpeed(adlx_bool* supported) = 0;
	virtual ADLX_RESULT __stdcall IsSupportedGPUVRAM(adlx_bool* supported) = 0;
	virtual ADLX_RESULT __stdcall IsSupportedGPUVoltage(adlx_bool* supported) = 0;
	virtual ADLX_RESULT __stdcall GetGPUUsageRange(adlx_int* minValue, adlx_int* maxValue) = 0;
	virtual ADLX_RESULT __stdcall GetGPUClockSpeedRange(adlx_int* minValue, adlx_int* maxValue) = 0;
	virtual ADLX_RESULT __stdcall GetGPUVRAMClockSpeedRange(adlx_int* minValue, adlx_int* maxValue) = 0;
	virtual ADLX_RESULT __stdcall GetGPUTemperatureRange(adlx_int* minValue, adlx_int* maxValue) = 0;
	virtual ADLX_RESULT __stdcall GetGPUHotspotTemperatureRange(adlx_int* minValue, adlx_int* maxValue) = 0;
	virtual ADLX_RESULT __stdcall GetGPUPowerRange(adlx_int* minValue, adlx_int* maxValue) = 0;
	virtual ADLX_RESULT __stdcall GetGPUFanSpeedRange(adlx_int* minValue, adlx_int* maxValue) = 0;
	virtual ADLX_RESULT __stdcall GetGPUVRAMRange(adlx_int* minValue, adlx_int* maxValue) = 0;
	virtual ADLX_RESULT __stdcall GetGPUVoltageRange(adlx_int* minValue, adlx_int* maxValue) = 0;
	virtual ADLX_RESULT __stdcall GetGPUTotalBoardPowerRange(adlx_int* minValue, adlx_int* maxValue) = 0;
	virtual ADLX_RESULT __stdcall GetGPUIntakeTemperatureRange(adlx_int* minValue, adlx_int* maxValue) = 0;
	virtual ADLX_RESULT __stdcall IsSupportedGPUIntakeTemperature(adlx_bool* supported) = 0;
};

class __declspec(novtable) IADLXGPUMetrics : public IADLXInterface
{
public:
	virtual ADLX_RESULT __stdcall TimeStamp(adlx_int64* ms) = 0;
	virtual ADLX_RESULT __stdcall GPUUsage(adlx_double* data) = 0;
	virtual ADLX_RESULT __stdcall GPUClockSpeed(adlx_int* data) = 0;
	virtual ADLX_RESULT __stdcall GPUVRAMClockSpeed(adlx_int* data) = 0;
	virtual ADLX_RESULT __stdcall GPUTemperature(adlx_double* data) = 0;
	virtual ADLX_RESULT __stdcall GPUHotspotTemperature(adlx_double* data) = 0;
	virtual ADLX_RESULT __stdcall GPUPower(adlx_double* data) = 0;
	virtual ADLX_RESULT __stdcall GPUTotalBoardPower(adlx_double* data) = 0;
	virtual ADLX_RESULT __stdcall GPUFanSpeed(adlx_int* data) = 0;
	virtual ADLX_RESULT __stdcall GPUVRAM(adlx_int* data) = 0;
	virtual ADLX_RESULT __stdcall GPUVoltage(adlx_int* data) = 0;
	virtual ADLX_RESULT __stdcall GPUIntakeTemperature(adlx_double* data) = 0;
};

// --- Performance Monitoring Services ---

class IADLXAllMetricsList;
class IADLXGPUMetricsList;
class IADLXSystemMetricsList;
class IADLXFPSList;
class IADLXAllMetrics;
class IADLXSystemMetrics;
class IADLXFPS;
class IADLXSystemMetricsSupport;

class __declspec(novtable) IADLXPerformanceMonitoringServices : public IADLXInterface
{
public:
	virtual ADLX_RESULT __stdcall GetSamplingIntervalRange(void* range) = 0;
	virtual ADLX_RESULT __stdcall SetSamplingInterval(adlx_int intervalMs) = 0;
	virtual ADLX_RESULT __stdcall GetSamplingInterval(adlx_int* intervalMs) = 0;
	virtual ADLX_RESULT __stdcall GetMaxPerformanceMetricsHistorySizeRange(void* range) = 0;
	virtual ADLX_RESULT __stdcall SetMaxPerformanceMetricsHistorySize(adlx_int sizeSec) = 0;
	virtual ADLX_RESULT __stdcall GetMaxPerformanceMetricsHistorySize(adlx_int* sizeSec) = 0;
	virtual ADLX_RESULT __stdcall ClearPerformanceMetricsHistory() = 0;
	virtual ADLX_RESULT __stdcall GetCurrentPerformanceMetricsHistorySize(adlx_int* sizeSec) = 0;
	virtual ADLX_RESULT __stdcall StartPerformanceMetricsTracking() = 0;
	virtual ADLX_RESULT __stdcall StopPerformanceMetricsTracking() = 0;
	virtual ADLX_RESULT __stdcall GetAllMetricsHistory(adlx_int startMs, adlx_int stopMs, IADLXAllMetricsList** ppList) = 0;
	virtual ADLX_RESULT __stdcall GetGPUMetricsHistory(IADLXGPU* pGPU, adlx_int startMs, adlx_int stopMs, IADLXGPUMetricsList** ppList) = 0;
	virtual ADLX_RESULT __stdcall GetSystemMetricsHistory(adlx_int startMs, adlx_int stopMs, IADLXSystemMetricsList** ppList) = 0;
	virtual ADLX_RESULT __stdcall GetFPSHistory(adlx_int startMs, adlx_int stopMs, IADLXFPSList** ppList) = 0;
	virtual ADLX_RESULT __stdcall GetCurrentAllMetrics(IADLXAllMetrics** ppMetrics) = 0;
	virtual ADLX_RESULT __stdcall GetCurrentGPUMetrics(IADLXGPU* pGPU, IADLXGPUMetrics** ppMetrics) = 0;
	virtual ADLX_RESULT __stdcall GetCurrentSystemMetrics(IADLXSystemMetrics** ppMetrics) = 0;
	virtual ADLX_RESULT __stdcall GetCurrentFPS(IADLXFPS** ppFPS) = 0;
	virtual ADLX_RESULT __stdcall GetSupportedGPUMetrics(IADLXGPU* pGPU, IADLXGPUMetricsSupport** ppMetricsSupport) = 0;
	virtual ADLX_RESULT __stdcall GetSupportedSystemMetrics(IADLXSystemMetricsSupport** ppMetricsSupport) = 0;
};

// --- System (singleton, NOT ref-counted, no IADLXInterface base) ---

class IADLXDisplayServices;
class IADLXDesktopServices;
class IADLXGPUsChangedHandling;
class IADLXLog;
class IADLX3DSettingsServices;
class IADLXGPUTuningServices;
class IADLXI2C;

class __declspec(novtable) IADLXSystem
{
public:
	virtual ADLX_RESULT __stdcall HybridGraphicsType(void* pHgType) = 0;
	virtual ADLX_RESULT __stdcall GetGPUs(IADLXGPUList** ppGPUs) = 0;
	virtual ADLX_RESULT __stdcall QueryInterface(const wchar_t* interfaceId, void** ppInterface) = 0;
	virtual ADLX_RESULT __stdcall GetDisplaysServices(IADLXDisplayServices** ppDisplayServices) = 0;
	virtual ADLX_RESULT __stdcall GetDesktopsServices(IADLXDesktopServices** ppDesktopServices) = 0;
	virtual ADLX_RESULT __stdcall GetGPUsChangedHandling(IADLXGPUsChangedHandling** ppGPUsChangedHandling) = 0;
	virtual ADLX_RESULT __stdcall EnableLog(void* mode, void* severity, IADLXLog* pLogger, const wchar_t* fileName) = 0;
	virtual ADLX_RESULT __stdcall Get3DSettingsServices(IADLX3DSettingsServices** pp3DSettingsServices) = 0;
	virtual ADLX_RESULT __stdcall GetGPUTuningServices(IADLXGPUTuningServices** ppGPUTuningServices) = 0;
	virtual ADLX_RESULT __stdcall GetPerformanceMonitoringServices(IADLXPerformanceMonitoringServices** ppPerformanceMonitoringServices) = 0;
	virtual ADLX_RESULT __stdcall TotalSystemRAM(adlx_uint* ramMB) = 0;
	virtual ADLX_RESULT __stdcall GetI2C(IADLXGPU* pGPU, IADLXI2C** ppI2C) = 0;
};

} // namespace adlx

// ---------------------------------------------------------------------------

class RDGpuVendorTelemetryAdlxProvider : public IRDGpuVendorTelemetryProvider
{
public:
	RDGpuVendorTelemetryAdlxProvider();
	~RDGpuVendorTelemetryAdlxProvider() override;

	bool Initialize() override;
	void Shutdown() override;
	void CollectTelemetry(RDGpuVendorTelemetryData& data) override;

private:
	typedef int(__cdecl* ADLXInitialize_Fn)(uint64_t version, adlx::IADLXSystem** ppSystem);
	typedef int(__cdecl* ADLXTerminate_Fn)();

	HMODULE m_hAdlx;
	bool    m_initialized;

	ADLXInitialize_Fn m_pfnInitialize;
	ADLXTerminate_Fn  m_pfnTerminate;

	adlx::IADLXSystem* m_pSystem;
	adlx::IADLXPerformanceMonitoringServices* m_pPerfMon;
	adlx::IADLXGPU* m_pGpu;
};
