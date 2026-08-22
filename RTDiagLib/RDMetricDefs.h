#pragma once

#include "RTDiagLib.h"
#include <cstdint>

enum class RDMetricId : uint16_t
{
	// OS (Startup)
	OsVersion,

	// CPU (Startup)
	CpuName,
	CpuPhysicalCores,
	CpuLogicalProcessors,
	// CPU (IntervalPolling)
	CpuUsageSystem,
	CpuUsageProcess,

	// Memory (Startup)
	MemoryTotalMB,
	// Memory (IntervalPolling)
	MemoryUsedMB,
	MemoryAvailableMB,
	MemoryWorkingSetMB,
	MemoryHandles,

	// GPU (Startup)
	GpuName,
	GpuVramTotalMB,
	GpuDriverVersion,
	// GPU (IntervalPolling)
	GpuVramUsedMB,
	GpuVramFreeMB,
	GpuVramBudgetMB,
	GpuUsagePercent,

	// WMI (Startup)
	MachineType,

	// App (Frame, Push)
	AppSceneUpdateTimeMs,
	AppDrawTimeMs,
	AppPresentTimeMs,

	COUNT
};

enum class RDMetricType : uint8_t
{
	Int,
	Float,
	String
};

struct RDMetricMeta
{
	RDMetricId   id;
	const char*  key;
	RDMetricType type;
};

struct RTDIAGLIB_API RDFormattedEntry
{
	const char* label;
	const char* value;
};

struct RDFormatTemplateEntry
{
	const char* label;
	const char* templateStr;
};
