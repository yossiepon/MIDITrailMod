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
		{ "Version", "MIDITrail ${AppIdVersion} (${AppIdModVersion}), KDMAPI ${KdmapiVersion} (${KdmapiModVersion})" },
		{ "Build",   "${AppIdBuildConfig}, DX ${AppIdDxFeatureLevel}" },
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
		{ "Frame",  "${RenderFps} FPS (${RenderAvgFrameTimeMs} ms) 1%Low:${RenderFps1PercentLow} 0.1%Low:${RenderFps01PercentLow}" },
		{ "Timing", "Update:${RenderSceneUpdateTimeMs} Draw:${RenderDrawTimeMs} Present:${RenderPresentTimeMs} GPU:${RenderGpuRenderTimeMs} ms" },
		{ "Stab",   "StdDev:${RenderFrameTimeStdDev} ms, Stutter:${RenderStutterPercent}%" },
		{ "Render", "Instances:${RenderInstanceCount} Buffer:${RenderInstanceBufferSizeKB} KB" },
		{ "Notes",  "NPS:${PlaybackNps} Poly:${PlaybackPolyphony} (Peak:${PlaybackPolyphonyPeak}) Tracking:${PlaybackNoteTracking} (Peak:${PlaybackNoteTrackingPeak})" },
		{ "GPU API1", "${GpuUsageVendorPercent}%, Core ${GpuCoreClock} MHz, Mem ${GpuMemoryClock} MHz, Fan ${GpuFanSpeedRPM} RPM" },
		{ "GPU API2", "${GpuTemperature} C (Hotspot ${GpuHotspotTemperature} C), ${GpuPowerWatts} W, ${GpuVoltage} V, Intake ${GpuIntakeTemperature} C, VRAM ${GpuVramTemperature} C" },
		{ "Synth",    "Voices:${SynthActiveVoices}/${SynthMaxVoices} CPU:${SynthRenderLoad}% Latency:${SynthAudioLatency}ms" },
		{ "Synth Audio", "Freq:${SynthAudioFrequency}Hz ${SynthAudioBitDepth}bit ${SynthAudioSampleType} Buf:${SynthBufferLength} Engine:${KdmapiAudioEngine} Sinc:${KdmapiSincInterpolation}" },
		{ "MIDI Out", "${MidiOutTransport}, ${MidiOutDeviceName}, ${MidiOutActivePorts} port(s)" },
		{ "Diag",   "poll ${DiagPollingTotalUs} us [CPU:${DiagPollingCpuInfoUs} GPU:${DiagPollingGpuInfoUs} Mem:${DiagPollingMemoryInfoUs}] (${DiagPollingCount} comp)" },
	};

	static const size_t RuntimeSystemCount =
		sizeof(RuntimeSystem) / sizeof(RuntimeSystem[0]);

	static const RDFormatTemplateEntry OverlayMachineSignature[] = {
		{ "App", "MIDITrail ${AppIdVersion} (${AppIdModVersion}), KDMAPI ${KdmapiVersion} (${KdmapiModVersion})" },
		{ "",    "" },
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
		{ "Frame",   "${RenderFps} FPS (${RenderAvgFrameTimeMs} ms), 1%Low ${RenderFps1PercentLow}, 0.1%Low ${RenderFps01PercentLow}" },
		{ "Timing",  "Update ${RenderSceneUpdateTimeMs} ms, Draw ${RenderDrawTimeMs} ms, Present ${RenderPresentTimeMs} ms, GPU ${RenderGpuRenderTimeMs} ms" },
		{ "Stab",    "StdDev ${RenderFrameTimeStdDev} ms, Stutter ${RenderStutterPercent}%" },
		{ "Render",  "Buffer ${RenderInstanceBufferSizeKB} KB, Instances ${RenderInstanceCount}" },
		{ "",        "" },
		{ "MIDI",    "${MidiOutTransport}, ${MidiOutActivePorts} port(s)" },
		{ "Notes",   "${PlaybackNps} NPS, Poly ${PlaybackPolyphony} (Peak ${PlaybackPolyphonyPeak}), Tracking ${PlaybackNoteTracking} (Peak ${PlaybackNoteTrackingPeak})" },
		{ "Synth",   "Voices ${SynthActiveVoices}/${SynthMaxVoices}, CPU ${SynthRenderLoad}%, Latency ${SynthAudioLatency} ms" },
		{ "Synth Audio", "${SynthAudioFrequency}Hz ${SynthAudioBitDepth}bit ${SynthAudioSampleType}, Buf ${SynthBufferLength}, ${KdmapiAudioEngine}, Sinc ${KdmapiSincInterpolation}" },
		{ "",        "" },
		{ "Diag",    "poll ${DiagPollingTotalUs} us [CPU ${DiagPollingCpuInfoUs} us, GPU ${DiagPollingGpuInfoUs} us, Mem ${DiagPollingMemoryInfoUs} us] (${DiagPollingCount} comp)" },
	};

	static const size_t OverlayRuntimeCount =
		sizeof(OverlayRuntime) / sizeof(OverlayRuntime[0]);

	static const RDFormatTemplateEntry FileLoaded[] = {
		{ "File",     "${PlaybackLoadedFileName}" },
		{ "Notes",    "${PlaybackTotalNoteCount} notes" },
		{ "Duration", "${PlaybackTotalPlayTimeMs} ms" },
	};

	static const size_t FileLoadedCount =
		sizeof(FileLoaded) / sizeof(FileLoaded[0]);

	static const RDFormatTemplateEntry SceneReady[] = {
		{ "Scene",    "${PlaybackSceneType}" },
		{ "Buffer",   "${RenderInstanceBufferSizeKB} KB (${PlaybackTotalNoteCount} notes)" },
	};

	static const size_t SceneReadyCount =
		sizeof(SceneReady) / sizeof(SceneReady[0]);
}
