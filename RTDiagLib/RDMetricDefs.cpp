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
	{ RDMetricId::GpuUsagePercent,      "GpuUsagePercent",       RDMetricType::Float },

	{ RDMetricId::MachineType,          "MachineType",          RDMetricType::String },

	{ RDMetricId::AppFrameTimeMs,       "AppFrameTimeMs",       RDMetricType::Float },
	{ RDMetricId::AppSceneUpdateTimeMs, "AppSceneUpdateTimeMs", RDMetricType::Float },
	{ RDMetricId::AppDrawTimeMs,        "AppDrawTimeMs",        RDMetricType::Float },
	{ RDMetricId::AppPresentTimeMs,     "AppPresentTimeMs",     RDMetricType::Float },
	{ RDMetricId::AppGpuRenderTimeMs,   "AppGpuRenderTimeMs",  RDMetricType::Float },

	{ RDMetricId::AppAvgFrameTimeMs,    "AppAvgFrameTimeMs",    RDMetricType::Float },
	{ RDMetricId::AppFps,               "AppFps",               RDMetricType::Float },
	{ RDMetricId::AppFps1PercentLow,    "AppFps1PercentLow",    RDMetricType::Float },
	{ RDMetricId::AppFps01PercentLow,   "AppFps01PercentLow",   RDMetricType::Float },
	{ RDMetricId::AppFrameTimeStdDev,   "AppFrameTimeStdDev",   RDMetricType::Float },
	{ RDMetricId::AppStutterPercent,    "AppStutterPercent",     RDMetricType::Float },

	{ RDMetricId::AppLoadedFileName,    "AppLoadedFileName",    RDMetricType::String },
	{ RDMetricId::AppTotalNoteCount,    "AppTotalNoteCount",    RDMetricType::Int },
	{ RDMetricId::AppTotalPlayTimeMs,   "AppTotalPlayTimeMs",   RDMetricType::Int },
	{ RDMetricId::AppSceneType,         "AppSceneType",         RDMetricType::String },
	{ RDMetricId::AppMidiOutTransport,  "AppMidiOutTransport",  RDMetricType::String },
	{ RDMetricId::AppVersion,           "AppVersion",           RDMetricType::String },
	{ RDMetricId::AppModVersion,        "AppModVersion",        RDMetricType::String },

	{ RDMetricId::AppNoteActivationsPerFrame, "AppNoteActivationsPerFrame", RDMetricType::Int },
	{ RDMetricId::AppNps,               "AppNps",               RDMetricType::Float },
	{ RDMetricId::AppNoteTracking,         "AppNoteTracking",         RDMetricType::Int },
	{ RDMetricId::AppNoteTrackingPeak,     "AppNoteTrackingPeak",     RDMetricType::Int },
	{ RDMetricId::AppPolyphony,            "AppPolyphony",            RDMetricType::Int },
	{ RDMetricId::AppPolyphonyPeak,        "AppPolyphonyPeak",        RDMetricType::Int },
	{ RDMetricId::AppInstanceCount,     "AppInstanceCount",     RDMetricType::Int },
	{ RDMetricId::AppInstanceBufferSizeKB, "AppInstanceBufferSizeKB", RDMetricType::Int },

	{ RDMetricId::GpuTemperature,           "GpuTemperature",           RDMetricType::Float },
	{ RDMetricId::GpuCoreClock,             "GpuCoreClock",             RDMetricType::Int },
	{ RDMetricId::GpuMemoryClock,           "GpuMemoryClock",           RDMetricType::Int },
	{ RDMetricId::GpuPowerWatts,            "GpuPowerWatts",            RDMetricType::Float },
	{ RDMetricId::GpuFanSpeedRPM,           "GpuFanSpeedRPM",           RDMetricType::Int },
	{ RDMetricId::GpuUsageVendorPercent,    "GpuUsageVendorPercent",    RDMetricType::Float },
	{ RDMetricId::GpuHotspotTemperature,    "GpuHotspotTemperature",    RDMetricType::Float },
	{ RDMetricId::GpuVoltage,               "GpuVoltage",               RDMetricType::Float },
	{ RDMetricId::GpuIntakeTemperature,     "GpuIntakeTemperature",     RDMetricType::Float },
	{ RDMetricId::GpuVramTemperature,       "GpuVramTemperature",       RDMetricType::Float },

	{ RDMetricId::KdmapiStatus,             "KdmapiStatus",             RDMetricType::String },
	{ RDMetricId::KdmapiRenderingTime,      "KdmapiRenderingTime",      RDMetricType::Float },
	{ RDMetricId::KdmapiAudioLatency,       "KdmapiAudioLatency",       RDMetricType::Float },
	{ RDMetricId::KdmapiTotalActiveVoices,  "KdmapiTotalActiveVoices",  RDMetricType::Int },
	{ RDMetricId::KdmapiMaxVoices,          "KdmapiMaxVoices",          RDMetricType::Int },
	{ RDMetricId::KdmapiModVersion,         "KdmapiModVersion",         RDMetricType::String },

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
