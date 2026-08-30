//******************************************************************************
//
// RTDiagLib / RDMetricDefs
//
// Metric ID definitions and metadata table.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "RDMetricDefs.h"

static const RDMetricMeta s_MetricMetaTable[] =
{
	{ RDMetricId::OsProductName,        "OsProductName",        RDMetricType::String },
	{ RDMetricId::OsVersion,            "OsVersion",            RDMetricType::String },

	{ RDMetricId::CpuName,              "CpuName",              RDMetricType::String },
	{ RDMetricId::CpuArchitecture,      "CpuArchitecture",      RDMetricType::String },
	{ RDMetricId::CpuVendor,            "CpuVendor",            RDMetricType::String },
	{ RDMetricId::CpuSockets,           "CpuSockets",           RDMetricType::Int },
	{ RDMetricId::CpuPhysicalCores,     "CpuPhysicalCores",     RDMetricType::Int },
	{ RDMetricId::CpuLogicalProcessors, "CpuLogicalProcessors", RDMetricType::Int },
	{ RDMetricId::CpuBaseMHz,           "CpuBaseMHz",           RDMetricType::Int },
	{ RDMetricId::CpuUsageSystem,       "CpuUsageSystem",       RDMetricType::Float },
	{ RDMetricId::CpuUsageProcess,      "CpuUsageProcess",      RDMetricType::Float },

	{ RDMetricId::PhysMemTotalMB,       "PhysMemTotalMB",       RDMetricType::Int },
	{ RDMetricId::PhysMemAvailableMB,   "PhysMemAvailableMB",   RDMetricType::Int },

	{ RDMetricId::CommitLimitMB,        "CommitLimitMB",        RDMetricType::Int },
	{ RDMetricId::CommitUsedMB,         "CommitUsedMB",         RDMetricType::Int },
	{ RDMetricId::CommitFreeMB,         "CommitFreeMB",         RDMetricType::Int },

	{ RDMetricId::ProcessCommitMB,      "ProcessCommitMB",      RDMetricType::Int },
	{ RDMetricId::ProcessWorkingSetMB,  "ProcessWorkingSetMB",  RDMetricType::Int },
	{ RDMetricId::ProcessHandles,       "ProcessHandles",       RDMetricType::Int },

	{ RDMetricId::RdpSession,           "RdpSession",           RDMetricType::String },

	{ RDMetricId::GpuName,              "GpuName",              RDMetricType::String },
	{ RDMetricId::GpuVramTotalMB,       "GpuVramTotalMB",       RDMetricType::Int },
	{ RDMetricId::GpuVramTotalGB,       "GpuVramTotalGB",       RDMetricType::Int },
	{ RDMetricId::GpuDriverVersion,     "GpuDriverVersion",     RDMetricType::String },
	{ RDMetricId::GpuPciId,             "GpuPciId",             RDMetricType::String },
	{ RDMetricId::GpuVramUsedMB,        "GpuVramUsedMB",        RDMetricType::Int },
	{ RDMetricId::GpuVramFreeMB,        "GpuVramFreeMB",        RDMetricType::Int },
	{ RDMetricId::GpuVramBudgetMB,      "GpuVramBudgetMB",      RDMetricType::Int },
	{ RDMetricId::GpuUsagePercent,      "GpuUsagePercent",      RDMetricType::Float },

	{ RDMetricId::GpuTemperature,           "GpuTemperature",           RDMetricType::Float },
	{ RDMetricId::GpuCoreClock,             "GpuCoreClock",             RDMetricType::Int },
	{ RDMetricId::GpuMemoryClock,           "GpuMemoryClock",           RDMetricType::Int },
	{ RDMetricId::GpuPowerWatts,            "GpuPowerWatts",            RDMetricType::Float },
	{ RDMetricId::GpuFanSpeedRPM,           "GpuFanSpeedRPM",          RDMetricType::Int },
	{ RDMetricId::GpuUsageVendorPercent,    "GpuUsageVendorPercent",    RDMetricType::Float },
	{ RDMetricId::GpuHotspotTemperature,    "GpuHotspotTemperature",    RDMetricType::Float },
	{ RDMetricId::GpuVoltage,               "GpuVoltage",               RDMetricType::Float },
	{ RDMetricId::GpuIntakeTemperature,     "GpuIntakeTemperature",     RDMetricType::Float },
	{ RDMetricId::GpuVramTemperature,       "GpuVramTemperature",       RDMetricType::Float },

	{ RDMetricId::MachineType,          "MachineType",          RDMetricType::String },

	{ RDMetricId::AppIdVersion,         "AppIdVersion",         RDMetricType::String },
	{ RDMetricId::AppIdModVersion,      "AppIdModVersion",      RDMetricType::String },
	{ RDMetricId::AppIdBuildConfig,     "AppIdBuildConfig",     RDMetricType::String },
	{ RDMetricId::AppIdDxFeatureLevel,  "AppIdDxFeatureLevel",  RDMetricType::String },

	{ RDMetricId::RenderFrameTimeMs,       "RenderFrameTimeMs",       RDMetricType::Float },
	{ RDMetricId::RenderSceneUpdateTimeMs, "RenderSceneUpdateTimeMs", RDMetricType::Float },
	{ RDMetricId::RenderDrawTimeMs,        "RenderDrawTimeMs",        RDMetricType::Float },
	{ RDMetricId::RenderPresentTimeMs,     "RenderPresentTimeMs",     RDMetricType::Float },
	{ RDMetricId::RenderGpuRenderTimeMs,   "RenderGpuRenderTimeMs",   RDMetricType::Float },

	{ RDMetricId::RenderAvgFrameTimeMs,    "RenderAvgFrameTimeMs",    RDMetricType::Float },
	{ RDMetricId::RenderFps,               "RenderFps",               RDMetricType::Float },
	{ RDMetricId::RenderFps1PercentLow,    "RenderFps1PercentLow",    RDMetricType::Float },
	{ RDMetricId::RenderFps01PercentLow,   "RenderFps01PercentLow",   RDMetricType::Float },
	{ RDMetricId::RenderFrameTimeStdDev,   "RenderFrameTimeStdDev",   RDMetricType::Float },
	{ RDMetricId::RenderStutterPercent,    "RenderStutterPercent",    RDMetricType::Float },

	{ RDMetricId::RenderInstanceCount,        "RenderInstanceCount",        RDMetricType::Int },
	{ RDMetricId::RenderInstanceBufferSizeKB, "RenderInstanceBufferSizeKB", RDMetricType::Int },

	{ RDMetricId::PlaybackLoadedFileName,    "PlaybackLoadedFileName",    RDMetricType::String },
	{ RDMetricId::PlaybackTotalNoteCount,    "PlaybackTotalNoteCount",    RDMetricType::Int },
	{ RDMetricId::PlaybackTotalPlayTimeMs,   "PlaybackTotalPlayTimeMs",   RDMetricType::Int },
	{ RDMetricId::PlaybackSceneType,         "PlaybackSceneType",         RDMetricType::String },

	{ RDMetricId::PlaybackNoteActivationsPerFrame, "PlaybackNoteActivationsPerFrame", RDMetricType::Int },
	{ RDMetricId::PlaybackNps,               "PlaybackNps",               RDMetricType::Float },
	{ RDMetricId::PlaybackNoteTracking,      "PlaybackNoteTracking",      RDMetricType::Int },
	{ RDMetricId::PlaybackNoteTrackingPeak,  "PlaybackNoteTrackingPeak",  RDMetricType::Int },
	{ RDMetricId::PlaybackPolyphony,         "PlaybackPolyphony",         RDMetricType::Int },
	{ RDMetricId::PlaybackPolyphonyPeak,     "PlaybackPolyphonyPeak",     RDMetricType::Int },

	{ RDMetricId::MidiOutTransport,     "MidiOutTransport",     RDMetricType::String },
	{ RDMetricId::MidiOutDeviceName,    "MidiOutDeviceName",    RDMetricType::String },
	{ RDMetricId::MidiOutActivePorts,   "MidiOutActivePorts",   RDMetricType::Int },

	{ RDMetricId::SynthActiveVoices,        "SynthActiveVoices",        RDMetricType::Int },
	{ RDMetricId::SynthMaxVoices,           "SynthMaxVoices",           RDMetricType::Int },
	{ RDMetricId::SynthRenderLoad,          "SynthRenderLoad",          RDMetricType::Float },
	{ RDMetricId::SynthRenderHeadroom,      "SynthRenderHeadroom",      RDMetricType::Float },
	{ RDMetricId::SynthAudioLatency,        "SynthAudioLatency",        RDMetricType::Float },
	{ RDMetricId::SynthAudioFrequency,      "SynthAudioFrequency",      RDMetricType::Int },
	{ RDMetricId::SynthAudioBitDepth,       "SynthAudioBitDepth",       RDMetricType::Int },
	{ RDMetricId::SynthAudioSampleType,     "SynthAudioSampleType",     RDMetricType::String },
	{ RDMetricId::SynthAudioBufferSize,     "SynthAudioBufferSize",     RDMetricType::Int },
	{ RDMetricId::SynthOutputVolume,        "SynthOutputVolume",        RDMetricType::Float },

	{ RDMetricId::KdmapiStatus,             "KdmapiStatus",             RDMetricType::String },
	{ RDMetricId::KdmapiVersion,            "KdmapiVersion",            RDMetricType::String },
	{ RDMetricId::KdmapiModVersion,         "KdmapiModVersion",         RDMetricType::String },
	{ RDMetricId::KdmapiAudioEngine,        "KdmapiAudioEngine",        RDMetricType::String },
	{ RDMetricId::KdmapiSincInterpolation,  "KdmapiSincInterpolation",  RDMetricType::String },
	{ RDMetricId::KdmapiSincConvQuality,   "KdmapiSincConvQuality",   RDMetricType::String },
	{ RDMetricId::KdmapiOmniMidiVersion,   "KdmapiOmniMidiVersion",   RDMetricType::String },
	{ RDMetricId::KdmapiASIODeviceName,    "KdmapiASIODeviceName",    RDMetricType::String },
	{ RDMetricId::SynthCurrentSFList,      "SynthCurrentSFList",      RDMetricType::Int },
	{ RDMetricId::SynthNumChannels,        "SynthNumChannels",        RDMetricType::Int },

	{ RDMetricId::DiagStartupTotalUs,   "DiagStartupTotalUs",   RDMetricType::Int },
	{ RDMetricId::DiagStartupOsInfoUs,  "DiagStartupOsInfoUs",  RDMetricType::Int },
	{ RDMetricId::DiagStartupCpuInfoUs, "DiagStartupCpuInfoUs", RDMetricType::Int },
	{ RDMetricId::DiagStartupGpuInfoUs, "DiagStartupGpuInfoUs", RDMetricType::Int },
	{ RDMetricId::DiagStartupMemoryInfoUs,"DiagStartupMemoryInfoUs",RDMetricType::Int },
	{ RDMetricId::DiagStartupWmiInfoUs, "DiagStartupWmiInfoUs", RDMetricType::Int },

	{ RDMetricId::DiagPollingTotalUs,   "DiagPollingTotalUs",   RDMetricType::Int },
	{ RDMetricId::DiagPollingCpuInfoUs, "DiagPollingCpuInfoUs", RDMetricType::Int },
	{ RDMetricId::DiagPollingGpuInfoUs, "DiagPollingGpuInfoUs", RDMetricType::Int },
	{ RDMetricId::DiagPollingMemoryInfoUs,"DiagPollingMemoryInfoUs",RDMetricType::Int },
	{ RDMetricId::DiagPollingCount,     "DiagPollingCount",     RDMetricType::Int },
};

static_assert(
	sizeof(s_MetricMetaTable) / sizeof(s_MetricMetaTable[0]) == static_cast<size_t>(RDMetricId::COUNT),
	"s_MetricMetaTable must have exactly RDMetricId::COUNT entries"
);

const RDMetricMeta* RDGetMetricMetaTable()
{
	return s_MetricMetaTable;
}

size_t RDGetMetricMetaCount()
{
	return sizeof(s_MetricMetaTable) / sizeof(s_MetricMetaTable[0]);
}
