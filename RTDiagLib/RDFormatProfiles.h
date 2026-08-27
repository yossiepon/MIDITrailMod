//******************************************************************************
//
// RTDiagLib / RDFormatProfiles
//
// Format template profiles for log output and overlay display.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "RDMetricDefs.h"

namespace RDFormatProfile
{
	static const RDFormatTemplateEntry MachineSignature[] = {
		{ "OS",      "${OsProductName} (${OsVersion}), ${MachineType} ${CpuArchitecture}" },
		{ "CPU",     "${CpuName} (${CpuArchitecture})" },
		{ "CPU info","${CpuVendor}, ${CpuSockets}P/${CpuPhysicalCores}C/${CpuLogicalProcessors}T, ${CpuBaseMHz} MHz" },
		{ "GPU",     "${GpuName} ${RdpSession}" },
		{ "PCI",     "${GpuPciId}" },
		{ "VRAM",    "${GpuVramUsedMB} / ${GpuVramTotalMB} MB used (Budget: ${GpuVramBudgetMB} MB)" },
		{ "Driver",  "${GpuDriverVersion}" },
		{ "RAM",     "${PhysMemAvailableMB} / ${PhysMemTotalMB} MB free" },
		{ "Commit",  "${CommitUsedMB} / ${CommitLimitMB} MB used" },
		{ "Machine", "${MachineType}" },
		{ "Diag",    "startup ${DiagStartupTotalUs} us [OS:${DiagStartupOsInfoUs} CPU:${DiagStartupCpuInfoUs} GPU:${DiagStartupGpuInfoUs} Mem:${DiagStartupMemoryInfoUs} WMI:${DiagStartupWmiInfoUs}]" },
	};

	static const size_t MachineSignatureCount =
		sizeof(MachineSignature) / sizeof(MachineSignature[0]);

	static const RDFormatTemplateEntry RuntimeSystem[] = {
		{ "CPU",    "${CpuUsageSystem}% (Sys) / ${CpuUsageProcess}% (Proc)" },
		{ "RAM",    "${PhysMemAvailableMB} / ${PhysMemTotalMB} MB free" },
		{ "Commit", "${CommitUsedMB} / ${CommitLimitMB} MB used" },
		{ "Process","Commit: ${ProcessCommitMB} MB, WS: ${ProcessWorkingSetMB} MB, Handles: ${ProcessHandles}" },
		{ "VRAM",   "${GpuVramUsedMB} / ${GpuVramTotalMB} MB used (Free: ${GpuVramFreeMB} MB)" },
		{ "GPU",    "${GpuUsagePercent}%" },
		{ "Frame",  "${AppFps} FPS (${AppAvgFrameTimeMs} ms) 1%Low:${AppFps1PercentLow} 0.1%Low:${AppFps01PercentLow}" },
		{ "Timing", "Update:${AppSceneUpdateTimeMs} Draw:${AppDrawTimeMs} Present:${AppPresentTimeMs} GPU:${AppGpuRenderTimeMs} ms" },
		{ "Stab",   "StdDev:${AppFrameTimeStdDev} ms, Stutter:${AppStutterPercent}%" },
		{ "Render", "Inst:${AppInstanceCount} Buf:${AppInstanceBufferSizeKB} KB" },
		{ "Notes",  "NPS:${AppNps} Poly:${AppPolyphony} (Peak:${AppPolyphonyPeak}) Track:${AppNoteTracking} (Peak:${AppNoteTrackingPeak})" },
		{ "GPU API1", "${GpuUsageVendorPercent}%, Core ${GpuCoreClock} MHz, Mem ${GpuMemoryClock} MHz, Fan ${GpuFanSpeedRPM} RPM" },
		{ "GPU API2", "${GpuTemperature} C (Hotspot ${GpuHotspotTemperature} C), ${GpuPowerWatts} W, ${GpuVoltage} V" },
		{ "KDMAPI", "${AppMidiOutTransport}, Voices:${KdmapiTotalActiveVoices}/${KdmapiMaxVoices} CPU:${KdmapiRenderingTime}% Latency:${KdmapiAudioLatency}ms" },
		{ "Diag",   "poll ${DiagPollingTotalUs} us [CPU:${DiagPollingCpuInfoUs} GPU:${DiagPollingGpuInfoUs} Mem:${DiagPollingMemoryInfoUs}] (${DiagPollingCount} comp)" },
	};

	static const size_t RuntimeSystemCount =
		sizeof(RuntimeSystem) / sizeof(RuntimeSystem[0]);

	static const RDFormatTemplateEntry OverlayMachineSignature[] = {
		{ "OS",  "${OsProductName} (${OsVersion}), ${MachineType} ${CpuArchitecture}" },
		{ "CPU", "${CpuName} ${CpuBaseMHz} MHz (${CpuSockets}P/${CpuPhysicalCores}C/${CpuLogicalProcessors}T)" },
		{ "GPU", "${GpuName} ${GpuVramTotalGB}GB (${GpuDriverVersion}) ${RdpSession}" },
	};

	static const size_t OverlayMachineSignatureCount =
		sizeof(OverlayMachineSignature) / sizeof(OverlayMachineSignature[0]);

	static const RDFormatTemplateEntry OverlayRuntime[] = {
		{ "",        "" },
		{ "RAM",     "${PhysMemAvailableMB} / ${PhysMemTotalMB} MB free" },
		{ "Commit",  "${CommitFreeMB} / ${CommitLimitMB} MB free" },
		{ "Process", "WS ${ProcessWorkingSetMB} MB, Commit ${ProcessCommitMB} MB, Handles ${ProcessHandles}" },
		{ "VRAM",    "${GpuVramUsedMB} / ${GpuVramTotalMB} MB used (Free ${GpuVramFreeMB} MB)" },
		{ "",        "" },
		{ "Load",    "CPU ${CpuUsageSystem}% (Sys) / ${CpuUsageProcess}% (Proc), GPU ${GpuUsagePercent}%" },
		{ "GPU API1", "${GpuUsageVendorPercent}%, Core ${GpuCoreClock} MHz, Mem ${GpuMemoryClock} MHz, Fan ${GpuFanSpeedRPM} RPM" },
		{ "GPU API2", "${GpuTemperature} C (Hotspot ${GpuHotspotTemperature} C), ${GpuPowerWatts} W, ${GpuVoltage} V" },
		{ "",        "" },
		{ "Frame",   "${AppFps} FPS (${AppAvgFrameTimeMs} ms), 1%Low ${AppFps1PercentLow}, 0.1%Low ${AppFps01PercentLow}" },
		{ "Timing",  "Update ${AppSceneUpdateTimeMs} ms, Draw ${AppDrawTimeMs} ms, Present ${AppPresentTimeMs} ms, GPU ${AppGpuRenderTimeMs} ms" },
		{ "Stab",    "StdDev ${AppFrameTimeStdDev} ms, Stutter ${AppStutterPercent}%" },
		{ "Render",  "Buf ${AppInstanceBufferSizeKB} KB, Inst ${AppInstanceCount}" },
		{ "",        "" },
		{ "Notes",   "${AppNps} NPS, Poly ${AppPolyphony} (Peak ${AppPolyphonyPeak}), Track ${AppNoteTracking} (Peak ${AppNoteTrackingPeak})" },
		{ "KDMAPI", "${AppMidiOutTransport}, Voices ${KdmapiTotalActiveVoices}/${KdmapiMaxVoices}, CPU ${KdmapiRenderingTime}%, Latency ${KdmapiAudioLatency} ms" },
		{ "",        "" },
		{ "Diag",    "poll ${DiagPollingTotalUs} us [CPU ${DiagPollingCpuInfoUs} us, GPU ${DiagPollingGpuInfoUs} us, Mem ${DiagPollingMemoryInfoUs} us] (${DiagPollingCount} comp)" },
	};

	static const size_t OverlayRuntimeCount =
		sizeof(OverlayRuntime) / sizeof(OverlayRuntime[0]);

	static const RDFormatTemplateEntry FileLoaded[] = {
		{ "File",     "${AppLoadedFileName}" },
		{ "Notes",    "${AppTotalNoteCount} notes" },
		{ "Duration", "${AppTotalPlayTimeMs} ms" },
	};

	static const size_t FileLoadedCount =
		sizeof(FileLoaded) / sizeof(FileLoaded[0]);

	static const RDFormatTemplateEntry SceneReady[] = {
		{ "Scene",    "${AppSceneType}" },
		{ "Buffer",   "${AppInstanceBufferSizeKB} KB (${AppTotalNoteCount} notes)" },
	};

	static const size_t SceneReadyCount =
		sizeof(SceneReady) / sizeof(SceneReady[0]);
}
