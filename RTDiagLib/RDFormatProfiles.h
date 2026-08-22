#pragma once

#include "RDMetricDefs.h"

namespace RDFormatProfile
{
	static const RDFormatTemplateEntry MachineSignature[] = {
		{ "OS",      "${OsProductName} (${OsVersion})" },
		{ "CPU",     "${CpuName} (${CpuArchitecture})" },
		{ "CPU info","${CpuVendor}, ${CpuSockets}P/${CpuPhysicalCores}C/${CpuLogicalProcessors}T, ${CpuBaseMHz} MHz" },
		{ "GPU",     "${GpuName}" },
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
		{ "WS",     "${ProcessWorkingSetMB} MB (Handles: ${ProcessHandles})" },
		{ "VRAM",   "${GpuVramUsedMB} / ${GpuVramTotalMB} MB used (Free: ${GpuVramFreeMB} MB)" },
		{ "GPU",    "${GpuUsagePercent}%" },
		{ "Diag",   "poll ${DiagPollingTotalUs} us [CPU:${DiagPollingCpuInfoUs} GPU:${DiagPollingGpuInfoUs} Mem:${DiagPollingMemoryInfoUs}] (${DiagPollingCount} comp)" },
	};

	static const size_t RuntimeSystemCount =
		sizeof(RuntimeSystem) / sizeof(RuntimeSystem[0]);
}
