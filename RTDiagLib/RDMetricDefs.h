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

	// WMI (Startup)
	MachineType,

	// App Identity (Push, one-shot)
	AppIdVersion,
	AppIdModVersion,
	AppIdBuildConfig,
	AppIdDxFeatureLevel,

	// Rendering: Frame timing (Frame, Push)
	RenderFrameTimeMs,
	RenderSceneUpdateTimeMs,
	RenderDrawTimeMs,
	RenderPresentTimeMs,
	RenderGpuRenderTimeMs,

	// Rendering: Frame statistics (Frame, Computed)
	RenderAvgFrameTimeMs,
	RenderFps,
	RenderFps1PercentLow,
	RenderFps01PercentLow,
	RenderFrameTimeStdDev,
	RenderStutterPercent,

	// Rendering: Instanced drawing (Frame, Push)
	RenderInstanceCount,
	RenderInstanceBufferSizeKB,

	// Playback: Content metadata (Push, one-shot)
	PlaybackLoadedFileName,
	PlaybackTotalNoteCount,
	PlaybackTotalPlayTimeMs,
	PlaybackSceneType,

	// Playback: Note metrics (Frame, Push)
	PlaybackNoteActivationsPerFrame, // internal: per-frame counter for NPS computation (not displayed)
	PlaybackNps,
	PlaybackNoteTracking,
	PlaybackNoteTrackingPeak,
	PlaybackPolyphony,
	PlaybackPolyphonyPeak,

	// MIDI Output (Push, event-driven)
	MidiOutTransport,
	MidiOutDeviceName,
	MidiOutActivePorts,

	// MIDI Synth: Universal parameters (IntervalPolling, RDKdmapiInfo)
	SynthActiveVoices,
	SynthMaxVoices,
	SynthRenderLoad,         // BASS_ATTRIB_CPU: audio rendering load (%)
	SynthAudioLatency,
	SynthAudioFrequency,
	SynthAudioBitDepth,
	SynthAudioSampleType,    // "float" or "int"
	SynthAudioBufferSize,
	SynthOutputVolume,       // 0.0-100.0%, -1.0 = N/A

	// MIDI Synth: KDMAPI-specific (Startup + IntervalPolling, RDKdmapiInfo)
	KdmapiStatus,
	KdmapiVersion,
	KdmapiModVersion,
	KdmapiAudioEngine,
	KdmapiSincInterpolation,

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
