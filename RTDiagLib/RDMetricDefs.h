#pragma once

#include "RTDiagLib.h"
#include <cstdint>

enum class RDMetricId : uint16_t
{
	// OS (Startup)
	OsProductName,
	OsVersion,

	// CPU (Startup)
	CpuName,
	CpuArchitecture,
	CpuVendor,
	CpuSockets,
	CpuPhysicalCores,
	CpuLogicalProcessors,
	CpuBaseMHz,
	// CPU (IntervalPolling)
	CpuUsageSystem,
	CpuUsageProcess,

	// Physical Memory (Startup)
	PhysMemTotalMB,
	// Physical Memory (IntervalPolling)
	PhysMemAvailableMB,

	// Commit: Physical + PageFile (Startup)
	CommitLimitMB,
	// Commit (IntervalPolling)
	CommitUsedMB,

	// Process (IntervalPolling)
	ProcessCommitMB,
	ProcessWorkingSetMB,
	ProcessHandles,

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

	// Diag: Startup timing
	DiagStartupTotalUs,
	DiagStartupOsInfoUs,
	DiagStartupCpuInfoUs,
	DiagStartupGpuInfoUs,
	DiagStartupMemoryInfoUs,
	DiagStartupWmiInfoUs,

	// Diag: Polling timing
	DiagPollingTotalUs,
	DiagPollingCpuInfoUs,
	DiagPollingGpuInfoUs,
	DiagPollingMemoryInfoUs,
	DiagPollingCount,

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
