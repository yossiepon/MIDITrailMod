//******************************************************************************
//
// RTDiagLib / RDMetricDefs
//
// Metric ID definitions and metadata table.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

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
	CommitFreeMB,

	// Process (IntervalPolling)
	ProcessCommitMB,
	ProcessWorkingSetMB,
	ProcessHandles,

	// Session (Startup)
	RdpSession,

	// GPU (Startup)
	GpuName,
	GpuVramTotalMB,
	GpuVramTotalGB,
	GpuDriverVersion,
	GpuPciId,
	// GPU (IntervalPolling)
	GpuVramUsedMB,
	GpuVramFreeMB,
	GpuVramBudgetMB,
	GpuUsagePercent,

	// WMI (Startup)
	MachineType,

	// App: Frame timing (Frame, Push)
	AppFrameTimeMs,
	AppSceneUpdateTimeMs,
	AppDrawTimeMs,
	AppPresentTimeMs,
	AppGpuRenderTimeMs,

	// App: Frame statistics (Frame, Computed)
	AppAvgFrameTimeMs,
	AppFps,
	AppFps1PercentLow,
	AppFps01PercentLow,
	AppFrameTimeStdDev,
	AppStutterPercent,

	// App: Event-driven (Push, one-shot)
	AppLoadedFileName,
	AppTotalNoteCount,
	AppTotalPlayTimeMs,
	AppSceneType,
	AppMidiOutTransport,

	// App: Version info (Push, one-shot)
	AppVersion,
	AppModVersion,

	// App: Black MIDI metrics (Frame, Push)
	AppNoteActivationsPerFrame,
	AppNps,
	AppNoteTracking,
	AppNoteTrackingPeak,
	AppPolyphony,
	AppPolyphonyPeak,
	AppInstanceCount,
	AppInstanceBufferSizeKB,

	// GPU Telemetry: Vendor SDK (IntervalPolling)
	GpuTemperature,
	GpuCoreClock,
	GpuMemoryClock,
	GpuPowerWatts,
	GpuFanSpeedRPM,
	GpuUsageVendorPercent,
	GpuHotspotTemperature,
	GpuVoltage,
	GpuIntakeTemperature,
	GpuVramTemperature,

	// KDMAPI: OmniMIDI synthesizer info via KDMAPI (IntervalPolling)
	KdmapiStatus,
	KdmapiRenderingTime,
	KdmapiAudioLatency,
	KdmapiTotalActiveVoices,
	KdmapiMaxVoices,
	KdmapiModVersion,

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
